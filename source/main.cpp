// SPDX-License-Identifier: MIT

// standard headers
#include <atomic>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>           // invoke()
#include <future>
#include <memory>               // unique_ptr<>
#include <numeric>              // accumulate()
#include <optional>
#include <ranges>               // ranges::zip()
#include <semaphore>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>              // pair<>
#include <vector>

// unix headers
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// WUT/WUPS headers
#include <coreinit/time.h>
#include <nn/pdm.h>
#include <notifications/notifications.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>
#include <wups/config/WUPSConfigItemStub.h>

#include "ntp.hpp"


using namespace std::literals;


#define PLUGIN_NAME "Wii U Time Sync"

#define CFG_HOURS        "hours"
#define CFG_MINUTES      "minutes"
#define CFG_MSG_DURATION "msg_duration"
#define CFG_NOTIFY       "notify"
#define CFG_SERVER       "server"
#define CFG_SYNC         "sync"
#define CFG_TOLERANCE    "tolerance"

// Important plugin information.
WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes a Wii U's clock to the Internet.");
WUPS_PLUGIN_VERSION("v2.0.0");
WUPS_PLUGIN_AUTHOR("Nightkingale, Daniel K. O.");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE(PLUGIN_NAME);


namespace cfg {
    int  hours        = 0;
    int  minutes      = 0;
    int  msg_duration = 5;
    bool notify       = false;
    char server[512]  = "pool.ntp.org";
    bool sync         = false;
    int  tolerance    = 250;

    OSTime offset = 0;          // combines hours and minutes offsets
}


std::atomic<bool> in_progress = false;


// RAII type that handles the in_progress flag.

struct progress_error : std::runtime_error {
    progress_error() :
        std::runtime_error{"progress_error"}
    {}
};

struct progress_guard {
    progress_guard()
    {
        bool expected_progress = false;
        if (!in_progress.compare_exchange_strong(expected_progress, true))
            throw progress_error{};
    }

    ~progress_guard()
    {
        in_progress = false;
    }
};


// The code below implements a wrapper for std::async() that respects a thread limit.

std::counting_semaphore async_limit{6};


template<typename Sem>
struct semaphore_releaser {
    Sem& s;

    semaphore_releaser(Sem& s) :
        s(s)
    {}

    ~semaphore_releaser()
    {
        s.release();
    }
};


template<typename Func,
         typename... Args>
[[nodiscard]]
std::future<typename std::invoke_result_t<std::decay_t<Func>, std::decay_t<Args>...>>
limited_async(Func&& func,
              Args&&... args)
{
    async_limit.acquire();

    try {
        return std::async(std::launch::async,
                          [](auto&& f, auto&&... a) -> auto
                          {
                              semaphore_releaser guard{async_limit};
                              return std::invoke(std::forward<decltype(f)>(f),
                                                 std::forward<decltype(a)>(a)...);
                          },
                          std::forward<Func>(func),
                          std::forward<Args>(args)...);
    }
    catch (...) {
        async_limit.release();
        throw;
    }
}


#ifdef __WUT__
// These can usually be found in <endian.h>, but WUT does not provide them.

constexpr
std::uint64_t
htobe64(std::uint64_t x)
{
    if constexpr (std::endian::native == std::endian::big)
        return x;
    else
        return std::byteswap(x);
}


constexpr
std::uint64_t
be64toh(std::uint64_t x)
{
    return htobe64(x);
}

#endif


void
report_error(const std::string& arg)
{
    std::string msg = "[" PLUGIN_NAME "] " + arg;
    NotificationModule_AddErrorNotificationEx(msg.c_str(),
                                              cfg::msg_duration,
                                              1,
                                              {255, 255, 255, 255},
                                              {160, 32, 32, 255},
                                              nullptr,
                                              nullptr);
}


void
report_info(const std::string& arg)
{
    if (!cfg::notify)
        return;

    std::string msg = "[" PLUGIN_NAME "] " + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {32, 32, 160, 255},
                                             nullptr,
                                             nullptr);
}


void
report_success(const std::string& arg)
{
    if (!cfg::notify)
        return;

    std::string msg = "[" PLUGIN_NAME "] " + arg;
    NotificationModule_AddInfoNotificationEx(msg.c_str(),
                                             cfg::msg_duration,
                                             {255, 255, 255, 255},
                                             {32, 160, 32, 255},
                                             nullptr,
                                             nullptr);
}


// Wrapper for strerror_r()
std::string
errno_to_string(int e)
{
    char buf[100];
    strerror_r(e, buf, sizeof buf);
    return buf;
}


OSTime
get_utc_time()
{
    return OSGetTime() - cfg::offset;
}


double
ntp_to_double(ntp::timestamp t)
{
    return std::ldexp(static_cast<double>(t), -32);
}


ntp::timestamp
double_to_ntp(double t)
{
    return std::ldexp(t, 32);
}


OSTime
ntp_to_wiiu(ntp::timestamp t)
{
    // Change t from NTP epoch (1900) to Wii U epoch (2000).
    // There are 24 leap years in this period.
    constexpr std::uint64_t seconds_per_day = 24 * 60 * 60;
    constexpr std::uint64_t seconds_offset = seconds_per_day * (100 * 365 + 24);
    t -= seconds_offset << 32;

    // Convert from u32.32 to Wii U ticks count.
    double dt = ntp_to_double(t);

    // Note: do the conversion in floating point to avoid overflows.
    OSTime r = dt * OSTimerClockSpeed;

    return r;
}


ntp::timestamp
wiiu_to_ntp(OSTime t)
{
    // Convert from Wii U ticks to seconds.
    // Note: do the conversion in floating point to avoid overflows.
    double dt = static_cast<double>(t) / OSTimerClockSpeed;
    ntp::timestamp r = double_to_ntp(dt);

    // Change r from Wii U epoch (2000) to NTP epoch (1900).
    constexpr std::uint64_t seconds_per_day = 24 * 60 * 60;
    constexpr std::uint64_t seconds_offset = seconds_per_day * (100 * 365 + 24);
    r += seconds_offset << 32;

    return r;
}


std::string
to_string(const struct sockaddr_in& addr)
{
    char buf[32];
    return inet_ntop(addr.sin_family, &addr.sin_addr,
                     buf, sizeof buf);
}


std::string
seconds_to_human(double s)
{
    char buf[64];

    if (std::fabs(s) < 2) // less than 2 seconds
        std::snprintf(buf, sizeof buf, "%.3f ms", 1000 * s);
    else if (std::fabs(s) < 2 * 60) // less than 2 minutes
        std::snprintf(buf, sizeof buf, "%.1f s", s);
    else if (std::fabs(s) < 2 * 60 * 60) // less than 2 hours
        std::snprintf(buf, sizeof buf, "%.1f min", s / 60);
    else if (std::fabs(s) < 2 * 24 * 60 * 60) // less than 2 days
        std::snprintf(buf, sizeof buf, "%.1f hrs", s / (60 * 60));
    else
        std::snprintf(buf, sizeof buf, "%.1f days", s / (24 * 60 * 60));

    return buf;
}


std::string
format_wiiu_time(OSTime wt)
{
    OSCalendarTime cal;
    OSTicksToCalendarTime(wt, &cal);
    char buffer[256];
    std::snprintf(buffer, sizeof buffer,
                  "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                  cal.tm_year, cal.tm_mon + 1, cal.tm_mday,
                  cal.tm_hour, cal.tm_min, cal.tm_sec, cal.tm_msec);
    return buffer;
}


std::string
format_ntp(ntp::timestamp t)
{
    OSTime wt = ntp_to_wiiu(t);
    return format_wiiu_time(wt);
}


std::vector<std::string>
split(const std::string& input,
      const std::string& separators)
{
    using std::string;

    std::vector<string> result;

    string::size_type start = 0;
    while (start != string::npos) {
        auto finish = input.find_first_of(separators, start);
        result.push_back(input.substr(start, finish - start));
        start = input.find_first_not_of(separators, finish);
    }

    return result;
}


extern "C" int32_t CCRSysSetSystemTime(OSTime time); // from nn_ccr
extern "C" BOOL __OSSetAbsoluteSystemTime(OSTime time); // from coreinit


bool
apply_clock_correction(double correction)
{
    OSTime correction_ticks = correction * OSTimerClockSpeed;

    OSTime now = OSGetTime();
    OSTime corrected = now + correction_ticks;

    nn::pdm::NotifySetTimeBeginEvent();

    if (CCRSysSetSystemTime(corrected)) {
        nn::pdm::NotifySetTimeEndEvent();
        return false;
    }

    bool res = __OSSetAbsoluteSystemTime(corrected);

    nn::pdm::NotifySetTimeEndEvent();

    return res;
}


// RAII class to close down a socket
struct socket_guard {
    int fd;

    socket_guard(int ns, int st, int pr) :
        fd{socket(ns, st, pr)}
    {}

    ~socket_guard()
    {
        if (fd != -1)
            close();
    }

    void
    close()
    {
        ::close(fd);
        fd = -1;
    }
};


// Note: hardcoded for IPv4, the Wii U doesn't have IPv6.
std::pair<double, double>
ntp_query(struct sockaddr_in address)
{
    socket_guard s{PF_INET, SOCK_DGRAM, IPPROTO_UDP};
    if (s.fd == -1)
        throw std::runtime_error{"Unable to create socket!"};

    connect(s.fd, reinterpret_cast<struct sockaddr*>(&address), sizeof address);

    ntp::packet packet;
    packet.version(4);
    packet.mode(ntp::packet::mode::client);


    unsigned num_send_tries = 0;
 try_again_send:

    ntp::timestamp t1 = wiiu_to_ntp(get_utc_time());
    packet.transmit_time = htobe64(t1);

    if (send(s.fd, &packet, sizeof packet, 0) == -1) {
        int e = errno;
        if (e != ENOMEM)
            throw std::runtime_error{"Unable to send NTP request: "s + errno_to_string(e)};
        if (++num_send_tries < 4) {
            std::this_thread::sleep_for(100ms);
            goto try_again_send;
        } else
            throw std::runtime_error{"No resources for send(), too many retries!"};
    }

    struct timeval timeout = { 4, 0 };
    fd_set read_set;


    unsigned num_select_tries = 0;
 try_again_select:

    FD_ZERO(&read_set);
    FD_SET(s.fd, &read_set);

    if (select(s.fd + 1, &read_set, nullptr, nullptr, &timeout) == -1) {
        // Wii U's OS can only handle 16 concurrent select() calls,
        // so we may need to try again later.
        int e = errno;
        if (e != ENOMEM)
            throw std::runtime_error{"select() failed: "s + errno_to_string(e)};
        if (++num_select_tries < 4) {
            std::this_thread::sleep_for(10ms);
            goto try_again_select;
        } else
            throw std::runtime_error{"No resources for select(), too many retries!"};
    }


    if (!FD_ISSET(s.fd, &read_set))
        throw std::runtime_error{"Timeout reached!"};

    if (recv(s.fd, &packet, sizeof packet, 0) < 48)
        throw std::runtime_error{"Invalid NTP response!"};

    ntp::timestamp t4 = wiiu_to_ntp(get_utc_time());

    ntp::timestamp t1_copy = be64toh(packet.origin_time);
    if (t1 != t1_copy)
        throw std::runtime_error{"NTP response does not match request: ["s
                                 + format_ntp(t1) + "] vs ["s
                                 + format_ntp(t1_copy) + "]"s};

    // when our request arrived at the server
    ntp::timestamp t2 = be64toh(packet.receive_time);
    // when the server sent out a response
    ntp::timestamp t3 = be64toh(packet.transmit_time);

    double roundtrip = ntp_to_double((t4 - t1) - (t3 - t2));
    double latency = roundtrip / 2;

    // t4 + correction = t3 + latency
    double correction = ntp_to_double(t3) + latency - ntp_to_double(t4);

    return { correction, latency };
}


// Wrapper for getaddrinfo(), hardcoded for IPv4

struct addrinfo_query {
    int flags    = 0;
    int family   = AF_UNSPEC;
    int socktype = 0;
    int protocol = 0;
};


struct addrinfo_result {
    int                        family;
    int                        socktype;
    int                        protocol;
    struct sockaddr_in         address;
    std::optional<std::string> canonname;
};


std::vector<addrinfo_result>
get_address_info(const std::optional<std::string>& name,
                 const std::optional<std::string>& port = {},
                 std::optional<addrinfo_query> query = {})
{
    // RAII: unique_ptr is used to invoke freeaddrinfo() on function exit
    std::unique_ptr<struct addrinfo,
                    decltype([](struct addrinfo* p) { freeaddrinfo(p); })>
        info;

    {
        struct addrinfo hints;
        const struct addrinfo *hints_ptr = nullptr;

        if (query) {
            hints_ptr = &hints;
            std::memset(&hints, 0, sizeof hints);
            hints.ai_flags = query->flags;
            hints.ai_family = query->family;
            hints.ai_socktype = query->socktype;
            hints.ai_protocol = query->protocol;
        }

        struct addrinfo* raw_info = nullptr;
        int err = getaddrinfo(name ? name->c_str() : nullptr,
                              port ? port->c_str() : nullptr,
                              hints_ptr,
                              &raw_info);
        if (err)
            throw std::runtime_error{gai_strerror(err)};

        info.reset(raw_info); // put it in the smart pointer
    }

    std::vector<addrinfo_result> result;

    // walk through the linked list
    for (auto a = info.get(); a; a = a->ai_next) {

        // sanity check: Wii U only supports IPv4
        if (a->ai_addrlen != sizeof(struct sockaddr_in))
            throw std::logic_error{"getaddrinfo() returned invalid result!"};

        addrinfo_result item;
        item.family = a->ai_family;
        item.socktype = a->ai_socktype;
        item.protocol = a->ai_protocol,
        std::memcpy(&item.address, a->ai_addr, sizeof item.address);
        if (a->ai_canonname)
            item.canonname = a->ai_canonname;

        result.push_back(std::move(item));
    }

    return result;
}


// ordering operator, so we can put sockaddr_in inside a std::set.
constexpr
bool
operator <(const struct sockaddr_in& a,
           const struct sockaddr_in& b)
    noexcept
{
    return a.sin_addr.s_addr < b.sin_addr.s_addr;
}


void
update_time()
try
{
    if (!cfg::sync)
        return;
        
    progress_guard guard;

    cfg::offset = OSSecondsToTicks(cfg::minutes * 60);
    if (cfg::hours < 0)
        cfg::offset -= OSSecondsToTicks(-cfg::hours * 60 * 60);
    else
        cfg::offset += OSSecondsToTicks(cfg::hours * 60 * 60);

    std::vector<std::string> servers = split(cfg::server, " \t,;");

    addrinfo_query query = {
        .family = AF_INET,
        .socktype = SOCK_DGRAM,
        .protocol = IPPROTO_UDP
    };

    // First, resolve all the names, in parallel.
    // Some IP addresses might be duplicated when we use *.pool.ntp.org.
    std::set<struct sockaddr_in> addresses;
    {
        std::vector<std::future<std::vector<addrinfo_result>>> infos(servers.size());
        for (auto [info_vec, server] : std::views::zip(infos, servers))
            info_vec = limited_async(get_address_info, server, "123", query);

        for (auto& info_vec : infos)
            try {
                for (auto info : info_vec.get())
                    addresses.insert(info.address);
            }
            catch (std::exception& e) {
                report_error(e.what());
            }
    }

    // Launch all NTP queries in parallel.
    std::vector<std::future<std::pair<double, double>>> results(addresses.size());
    for (auto [address, result] : std::views::zip(addresses, results))
        result = limited_async(ntp_query, address);

    // Now collect all results.
    std::vector<double> corrections;
    for (auto [address, result] : std::views::zip(addresses, results))
        try {
            auto [correction, latency] = result.get();
            corrections.push_back(correction);
            report_info(to_string(address)
                        + ": correction = "s + seconds_to_human(correction)
                        + ", latency = "s + seconds_to_human(latency) + "."s);
        }
        catch (std::exception& e) {
            report_error(to_string(address) + ": "s + e.what());
        }


    if (corrections.empty()) {
        report_error("No NTP server could be used!");
        return;
    }

    double avg_correction = std::accumulate(corrections.begin(),
                                            corrections.end(),
                                            0.0)
        / corrections.size();

    if (std::fabs(avg_correction) * 1000 <= cfg::tolerance) {
        report_success("Tolerating clock drift (correction is only "
                       + seconds_to_human(avg_correction) + ")."s);
        return;
    }

    if (cfg::sync) {
        if (!apply_clock_correction(avg_correction)) {
            report_error("Failed to set system clock!");
            return;
        }
    }

    if (cfg::notify)
        report_success("Clock corrected by " + seconds_to_human(avg_correction));
}
catch (progress_error&) {
    report_info("Skipping NTP task: already in progress.");
}


INITIALIZE_PLUGIN()
{
    WUPSStorageError storageRes = WUPS_OpenStorage();
    // Check if the plugin's settings have been saved before.
    if (storageRes == WUPS_STORAGE_ERROR_SUCCESS) {
        if (WUPS_GetBool(nullptr, CFG_SYNC, &cfg::sync) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreBool(nullptr, CFG_SYNC, cfg::sync);

        if (WUPS_GetBool(nullptr, CFG_NOTIFY, &cfg::notify) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreBool(nullptr, CFG_NOTIFY, cfg::notify);

        if (WUPS_GetInt(nullptr, CFG_MSG_DURATION, &cfg::msg_duration) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_MSG_DURATION, cfg::msg_duration);

        if (WUPS_GetInt(nullptr, CFG_HOURS, &cfg::hours) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_HOURS, cfg::hours);

        if (WUPS_GetInt(nullptr, CFG_MINUTES, &cfg::minutes) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_MINUTES, cfg::minutes);

        if (WUPS_GetInt(nullptr, CFG_TOLERANCE, &cfg::tolerance) == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreInt(nullptr, CFG_TOLERANCE, cfg::tolerance);

        if (WUPS_GetString(nullptr, CFG_SERVER, cfg::server, sizeof cfg::server)
            == WUPS_STORAGE_ERROR_NOT_FOUND)
            WUPS_StoreString(nullptr, CFG_SERVER, cfg::server);

        WUPS_CloseStorage();
    }

    NotificationModule_InitLibrary(); // Set up for notifications.

    if (cfg::sync)
        update_time(); // Update time when plugin is loaded.
}


WUPS_GET_CONFIG()
{
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS)
        return 0;

    WUPSConfigHandle settings;
    WUPSConfig_CreateHandled(&settings, PLUGIN_NAME);

    WUPSConfigCategoryHandle config;
    WUPSConfig_AddCategoryByNameHandled(settings, "Configuration", &config);
    WUPSConfigCategoryHandle preview;
    WUPSConfig_AddCategoryByNameHandled(settings, "Preview Time", &preview);

    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, CFG_SYNC,
                                               "Syncing Enabled",
                                               cfg::sync,
                                               [](ConfigItemBoolean*, bool value)
                                               {
                                                   WUPS_StoreBool(nullptr, CFG_SYNC, value);
                                                   cfg::sync = value;
                                               });
    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, CFG_NOTIFY,
                                               "Show Notifications",
                                               cfg::notify,
                                               [](ConfigItemBoolean*, bool value)
                                               {
                                                   WUPS_StoreBool(nullptr, CFG_NOTIFY, value);
                                                   cfg::notify = value;
                                               });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_HOURS,
                                                    "Time Offset (hours)",
                                                    cfg::hours, -12, 14,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_HOURS, value);
                                                        cfg::hours = value;
                                                    });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_MINUTES,
                                                    "Time Offset (minutes)",
                                                    cfg::minutes, 0, 59,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_MINUTES,
                                                                      value);
                                                        cfg::minutes = value;
                                                    });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_MSG_DURATION,
                                                    "Message Duration (seconds)",
                                                    cfg::msg_duration, 0, 30,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_MSG_DURATION,
                                                                      value);
                                                        cfg::msg_duration = value;
                                                    });
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, CFG_TOLERANCE,
                                                    "Tolerance (milliseconds)",
                                                    cfg::tolerance, 0, 5000,
                                                    [](ConfigItemIntegerRange*, int32_t value)
                                                    {
                                                        WUPS_StoreInt(nullptr, CFG_TOLERANCE,
                                                                      value);
                                                        cfg::tolerance = value;
                                                    });

    // show current NTP server address, no way to change it.
    std::string server = "NTP Servers: "s + cfg::server;
    WUPSConfigItemStub_AddToCategoryHandled(settings, config, CFG_SERVER, server.c_str());

    // Prepare the time to be shown for the user.
    std::string time = "Current Time: "s + format_wiiu_time(OSGetTime());
    WUPSConfigItemStub_AddToCategoryHandled(settings, preview, "time",
                                            time.c_str());

    return settings;
}


WUPS_CONFIG_CLOSED()
{
    std::jthread update_time_thread(update_time);
    update_time_thread.detach(); // Update time when settings are closed.

    WUPS_CloseStorage(); // Save all changes.
}
