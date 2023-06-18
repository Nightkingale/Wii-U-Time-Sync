#include <arpa/inet.h>
#include <malloc.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <coreinit/debug.h>
#include <coreinit/filesystem.h>
#include <coreinit/ios.h>
#include <coreinit/mcp.h>
#include <coreinit/time.h>
#include <nn/pdm.h>
#include <whb/proc.h>
#include <wups.h>
#include <wups/config/WUPSConfigItemBoolean.h>
#include <wups/config/WUPSConfigItemStub.h>
#include <wups/config/WUPSConfigItemIntegerRange.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <thread>

#define SYNCING_ENABLED_CONFIG_ID "enabledSync"
#define DST_ENABLED_CONFIG_ID "enabledDST"
#define OFFSET_HOURS_CONFIG_ID "offsetHours"
#define OFFSET_MINUTES_CONFIG_ID "offsetMinutes"
// Seconds between 1900 (NTP epoch) and 2000 (Wii U epoch)
#define NTP_TIMESTAMP_DELTA 3155673600llu

// Important plugin information.
WUPS_PLUGIN_NAME("Wii U Time Sync");
WUPS_PLUGIN_DESCRIPTION("A plugin that synchronizes a Wii U's clock to the Internet.");
WUPS_PLUGIN_VERSION("v1.0.0");
WUPS_PLUGIN_AUTHOR("Nightkingale");
WUPS_PLUGIN_LICENSE("MIT");

WUPS_USE_WUT_DEVOPTAB();
WUPS_USE_STORAGE("Wii U Time Sync");

bool enabledSync = false;
bool enabledDST = false;
int offsetHours = 0;
int offsetMinutes = 0;

// From https://github.com/lettier/ntpclient/blob/master/source/c/main.c
typedef struct
{
    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                                // li.   Two bits.   Leap indicator.
                                // vn.   Three bits. Version number of the protocol.
                                // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

} ntp_packet;              // Total: 384 bits or 48 bytes.

extern "C" int32_t CCRSysSetSystemTime(OSTime time);
extern "C" BOOL __OSSetAbsoluteSystemTime(OSTime time);

bool SetSystemTime(OSTime time)
{
    nn::pdm::NotifySetTimeBeginEvent();

    if (CCRSysSetSystemTime(time) != 0) {
        nn::pdm::NotifySetTimeEndEvent();
        return false;
    }

    BOOL res = __OSSetAbsoluteSystemTime(time);

    nn::pdm::NotifySetTimeEndEvent();

    return res != FALSE;
}

OSTime NTPGetTime(const char* hostname)
{
    ntp_packet packet;
    memset(&packet, 0, sizeof(packet));

    // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
    packet.li_vn_mode = 0x1b;

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        return 0;
    }

    // Get host address by name
    struct hostent* server = gethostbyname(hostname);
    if (!server) {
        return 0;
    }

    // Prepare socket address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    // Copy the server's IP address to the server address structure.
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Convert the port number integer to network big-endian style and save it to the server address structure.
    serv_addr.sin_port = htons(123); // UDP port

    // Call up the server using its IP address and port number.
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd);
        return 0;
    }

    // Send it the NTP packet it wants. If n == -1, it failed.
    if (write(sockfd, &packet, sizeof(packet)) < 0) {
        close(sockfd);
        return 0;
    }

    // Wait and receive the packet back from the server. If n == -1, it failed.
    if (read(sockfd, &packet, sizeof(packet)) < 0) {
        close(sockfd);
        return 0;
    }

    // Close the socket
    close(sockfd);

    // These two fields contain the time-stamp seconds as the packet left the NTP server.
    // The number of seconds correspond to the seconds passed since 1900.
    // ntohl() converts the bit/byte order from the network's to host's "endianness".
    packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
    packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

    OSTime tick = 0;
    // Convert seconds to ticks and adjust timestamp
    tick += OSSecondsToTicks(packet.txTm_s - NTP_TIMESTAMP_DELTA);
    // Convert fraction to ticks
    tick += OSNanosecondsToTicks((packet.txTm_f * 1000000000llu) >> 32);
    return tick;
}

void updateTime() {
    OSTime time = NTPGetTime("time.windows.com"); // Connect to the time server.

    if (time == 0) {
        return; // Probably didn't connect correctly.
    }

    if (offsetHours < 0) {
        time -= OSSecondsToTicks(abs(offsetHours) * 60 * 60);
    } else {
        time += OSSecondsToTicks(offsetHours * 60 * 60);
    }

    if (enabledDST) {
        time += OSSecondsToTicks(60 * 60); // DST adds an hour.
    }

    time += OSSecondsToTicks(offsetMinutes * 60);

    OSTime currentTime = OSGetTime();
    int timeDifference = abs(time - currentTime);

    if (static_cast<uint64_t>(timeDifference) <= OSMillisecondsToTicks(250)) {
        return; // Time difference is within 250 milliseconds, no need to update.
    }

    SetSystemTime(time); // This finally sets the console time.
}

INITIALIZE_PLUGIN() {
    WUPSStorageError storageRes = WUPS_OpenStorage();
    // Check if the plugin's settings have been saved before.
    if (storageRes == WUPS_STORAGE_ERROR_SUCCESS) {
        if ((storageRes = WUPS_GetBool(nullptr, SYNCING_ENABLED_CONFIG_ID, &enabledSync)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreBool(nullptr, SYNCING_ENABLED_CONFIG_ID, enabledSync);
        }

        if ((storageRes = WUPS_GetBool(nullptr, DST_ENABLED_CONFIG_ID, &enabledDST)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreBool(nullptr, DST_ENABLED_CONFIG_ID, enabledDST);
        }

        if ((storageRes = WUPS_GetInt(nullptr, OFFSET_HOURS_CONFIG_ID, &offsetHours)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreInt(nullptr, OFFSET_HOURS_CONFIG_ID, offsetHours);
        }

        if ((storageRes = WUPS_GetInt(nullptr, OFFSET_MINUTES_CONFIG_ID, &offsetMinutes)) == WUPS_STORAGE_ERROR_NOT_FOUND) {
            WUPS_StoreInt(nullptr, OFFSET_MINUTES_CONFIG_ID, offsetMinutes);
        }

        WUPS_CloseStorage(); // Close the storage.
    }

    if (enabledSync) {
        updateTime(); // Update time when plugin is loaded.
    }
}

void syncingEnabled(ConfigItemBoolean *item, bool value)
{
    // If false, bro is literally a time traveler!
    WUPS_StoreBool(nullptr, SYNCING_ENABLED_CONFIG_ID, value);
    enabledSync = value;
}

void savingsEnabled(ConfigItemBoolean *item, bool value)
{
    WUPS_StoreBool(nullptr, DST_ENABLED_CONFIG_ID, value);
    enabledDST = value;
}


void onHourOffsetChanged(ConfigItemIntegerRange *item, int32_t offset)
{
    WUPS_StoreInt(nullptr, OFFSET_HOURS_CONFIG_ID, offset);
    offsetHours = offset;
}

void onMinuteOffsetChanged(ConfigItemIntegerRange *item, int32_t offset)
{
    WUPS_StoreInt(nullptr, OFFSET_MINUTES_CONFIG_ID, offset);
    offsetMinutes = offset;
}

WUPS_GET_CONFIG() {
    if (WUPS_OpenStorage() != WUPS_STORAGE_ERROR_SUCCESS) {
        return 0;
    }

    WUPSConfigHandle settings;
    WUPSConfig_CreateHandled(&settings, "Wii U Time Sync");

    WUPSConfigCategoryHandle config;
    WUPSConfig_AddCategoryByNameHandled(settings, "Configuration", &config);
    WUPSConfigCategoryHandle preview;
    WUPSConfig_AddCategoryByNameHandled(settings, "Preview Time", &preview);

    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, "enabledSync", "Syncing Enabled", enabledSync, &syncingEnabled);
    WUPSConfigItemBoolean_AddToCategoryHandled(settings, config, "enabledDST", "Daylight Savings", enabledDST, &savingsEnabled);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, "offsetHours", "Time Offset (hours)", offsetHours, -12, 14, &onHourOffsetChanged);
    WUPSConfigItemIntegerRange_AddToCategoryHandled(settings, config, "offsetMinutes", "Time Offset (minutes)", offsetMinutes, 0, 59, &onMinuteOffsetChanged);

    OSCalendarTime ct;
    OSTicksToCalendarTime(OSGetTime(), &ct);
    char timeString[256];
    snprintf(timeString, 255, "Current Time: %04d-%02d-%02d %02d:%02d:%02d:%04d:%04d\n", ct.tm_year, ct.tm_mon + 1, ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec, ct.tm_msec, ct.tm_usec);
    WUPSConfigItemStub_AddToCategoryHandled(settings, preview, "time", timeString);

    return settings;
}

WUPS_CONFIG_CLOSED() {
    if (enabledSync) {
        std::thread updateTimeThread(updateTime);
        updateTimeThread.detach(); // Update time when settings are closed.
    }
    
    WUPS_CloseStorage(); // Save all changes.
}