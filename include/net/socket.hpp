// SPDX-License-Identifier: MIT

#ifndef NET_SOCKET_HPP
#define NET_SOCKET_HPP

#include <chrono>
#include <cstdint>
#include <expected>
#include <utility>              // pair<>

#include <netinet/in.h>         // IP_*
#include <netinet/tcp.h>        // TCP_*
#include <poll.h>
#include <sys/socket.h>         // SO_*, MSG_*

#include "net/address.hpp"
#include "net/error.hpp"


// Note: very simplified socket class, only what the Wii U supports.

namespace net {

    class socket {

        int fd = -1;

    public:

        enum class ip_option : int {
            tos = IP_TOS,
            ttl = IP_TTL,
        };


        enum class msg_flags : int {
            dontroute = MSG_DONTROUTE,
            dontwait  = MSG_DONTWAIT,
            none      = 0,
            oob       = MSG_OOB,
            peek      = MSG_PEEK,
        };


        enum class poll_flags : int {
            err  = POLLERR,
            hup  = POLLHUP,
            in   = POLLIN,
            none = 0,
            nval = POLLNVAL,
            out  = POLLOUT,
            pri  = POLLPRI,
        };


        enum class socket_option : int {
            bio         = SO_BIO,
            broadcast   = SO_BROADCAST,
            dontroute   = SO_DONTROUTE,
            error       = SO_ERROR,
            hopcnt      = SO_HOPCNT,
            keepalive   = SO_KEEPALIVE,
            keepcnt     = 0x101B,
            keepidle    = 0x1019,
            keepintvl   = 0x101A,
            linger      = SO_LINGER,
            maxmsg      = SO_MAXMSG,
            myaddr      = SO_MYADDR,
            nbio        = SO_NBIO,
            nonblock    = SO_NONBLOCK,
            noslowstart = SO_NOSLOWSTART,
            oobinline   = SO_OOBINLINE,
            rcvbuf      = SO_RCVBUF,
            rcvlowat    = SO_RCVLOWAT,
            reuseaddr   = SO_REUSEADDR,
            rusrbuf     = SO_RUSRBUF,
            rxdata      = SO_RXDATA,
            sndbuf      = SO_SNDBUF,
            sndlowat    = SO_SNDLOWAT,
            tcpsack     = SO_TCPSACK,
            txdata      = SO_TXDATA,
            type        = SO_TYPE,
            winscale    = SO_WINSCALE,
        };


        enum class tcp_option : int {
            ackdelaytime = TCP_ACKDELAYTIME,
            ackfrequency = 0x2005,
            maxseg       = TCP_MAXSEG,
            noackdelay   = TCP_NOACKDELAY,
            nodelay      = TCP_NODELAY,
        };


        enum class type {
            tcp,
            udp
        };


        constexpr
        socket() noexcept = default;

        explicit
        socket(int fd);

        explicit
        socket(type t);

        // move constructor
        socket(socket&& other) noexcept;

        // move assignment
        socket& operator =(socket&& other) noexcept;

        ~socket();


        // convenience named constructors

        static
        socket make_tcp();

        static
        socket make_udp();


        // check if socket is valid
        explicit
        operator bool() const noexcept;

        bool is_socket() const noexcept;


        // The regular BSD sockets API, throw net::error

        std::pair<socket, address> accept();

        void bind(address addr);

        void close();

        void connect(ipv4_t ip, port_t port);
        void connect(address addr);


        std::expected<std::uint8_t, error>
        getsockopt(ip_option opt) const noexcept;

        template<typename T>
        std::expected<T, error>
        getsockopt(socket_option opt) const noexcept;

        std::expected<unsigned, error>
        getsockopt(tcp_option opt) const noexcept;

        // convenience getters

        // getters for ip_option
        std::expected<std::uint8_t, error> get_tos() const noexcept;
        std::expected<std::uint8_t, error> get_ttl() const noexcept;

        // getters for socket_option
        std::expected<bool,     error> get_broadcast() const noexcept;
        std::expected<bool,     error> get_dontroute() const noexcept;
        std::expected<error,    error> get_error()     const noexcept;
        std::expected<unsigned, error> get_hopcnt()    const noexcept;
        std::expected<bool,     error> get_keepalive() const noexcept;
        std::expected<unsigned, error> get_keepcnt()   const noexcept;
        std::expected<unsigned, error> get_keepidle()  const noexcept;
        std::expected<unsigned, error> get_keepintvl() const noexcept;
        std::expected<::linger, error> get_linger()    const noexcept;
        std::expected<unsigned, error> get_maxmsg()    const noexcept;
        std::expected<address,  error> get_myaddr()    const noexcept;
        std::expected<bool,     error> get_nonblock()  const noexcept;
        std::expected<bool,     error> get_oobinline() const noexcept;
        std::expected<unsigned, error> get_rcvbuf()    const noexcept;
        std::expected<unsigned, error> get_rcvlowat()  const noexcept;
        std::expected<bool,     error> get_reuseaddr() const noexcept;
        std::expected<bool,     error> get_rusrbuf()   const noexcept;
        std::expected<unsigned, error> get_rxdata()    const noexcept;
        std::expected<unsigned, error> get_sndbuf()    const noexcept;
        std::expected<unsigned, error> get_sndlowat()  const noexcept;
        std::expected<bool,     error> get_tcpsack()   const noexcept;
        std::expected<unsigned, error> get_txdata()    const noexcept;
        std::expected<type,     error> get_type()      const noexcept;
        std::expected<bool,     error> get_winscale()  const noexcept;

        // getters for tcp_option
        std::expected<std::chrono::milliseconds, error> get_ackdelaytime() const noexcept;
        std::expected<unsigned, error> get_ackfrequency() const noexcept;
        std::expected<unsigned, error> get_maxseg() const noexcept;
        std::expected<bool, error> get_noackdelay() const noexcept;
        std::expected<bool, error> get_nodelay() const noexcept;


        address getpeername() const;
        address get_remote_address() const;

        address getsockname() const;
        address get_local_address() const;

        void listen(int backlog);


        poll_flags poll(poll_flags flags, std::chrono::milliseconds timeout = {}) const;
        // convenience wrappers for poll()
        bool is_readable(std::chrono::milliseconds timeout = {}) const;
        bool is_writable(std::chrono::milliseconds timeout = {}) const;


        std::size_t
        recv(void* buf, std::size_t len,
             msg_flags flags = msg_flags::none);

        std::size_t
        recv_all(void* buf, std::size_t total,
                 msg_flags flags = msg_flags::none);

        std::pair<std::size_t, address>
        recvfrom(void* buf, std::size_t len,
                 msg_flags flags = msg_flags::none);


        // Disassociate the handle from this socket.
        int release() noexcept;


        std::size_t
        send(const void* buf, std::size_t len,
             msg_flags flags = msg_flags::none);

        std::size_t
        send_all(const void* buf, std::size_t total,
                 msg_flags flags = msg_flags::none);

        std::size_t
        sendto(const void* buf, std::size_t len,
               address dst,
               msg_flags flags = msg_flags::none);


        void setsockopt(ip_option opt, std::uint8_t arg);

        void setsockopt(socket_option opt);
        void setsockopt(socket_option opt, unsigned arg);
        void setsockopt(socket_option opt, const struct ::linger& arg);

        void setsockopt(tcp_option opt, unsigned arg);


        // convenience setters

        // setters for ip_option
        void set_tos(std::uint8_t t);
        void set_ttl(std::uint8_t t);

        // setters for socket_option
        void set_bio();
        void set_broadcast(bool enable);
        void set_dontroute(bool enable);
        void set_keepalive(bool enable);
        void set_keepcnt(unsigned count);
        void set_keepidle(unsigned period);
        void set_keepintvl(unsigned interval);
        void set_linger(bool enable, int period = 0);
        void set_maxmsg(unsigned size);
        void set_nbio();
        void set_nonblock(bool enable);
        void set_noslowstart(bool enable);
        void set_oobinline(bool enable);
        void set_rcvbuf(unsigned size);
        void set_reuseaddr(bool enable);
        void set_rusrbuf(bool enable);
        void set_sndbuf(unsigned size);
        void set_tcpsack(bool enable);
        void set_winscale(bool enable);

        // setters for tcp_option
        void set_ackdelaytime(unsigned ms);
        void set_ackfrequency(unsigned pending);
        void set_maxseg(unsigned size);
        void set_noackdelay();
        void set_nodelay(bool enable);


        std::expected<poll_flags, error>
        try_poll(poll_flags flags, std::chrono::milliseconds timeout = {})
            const noexcept;

        std::expected<bool, error>
        try_is_readable(std::chrono::milliseconds timeout = {})
            const noexcept;

        std::expected<bool, error>
        try_is_writable(std::chrono::milliseconds timeout = {})
            const noexcept;


        std::expected<std::size_t, error>
        try_recv(void* buf, std::size_t len,
                 msg_flags flags = msg_flags::none)
            noexcept;

        std::expected<std::pair<std::size_t, address>,
                      error>
        try_recvfrom(void* buf, std::size_t len,
                     msg_flags flags = msg_flags::none)
            noexcept;


        std::expected<std::size_t, error>
        try_send(const void* buf, std::size_t len,
                 msg_flags flags = msg_flags::none)
            noexcept;

        std::expected<std::size_t, error>
        try_sendto(const void* buf, std::size_t len,
                   address dst,
                   msg_flags flags = msg_flags::none)
            noexcept;
    };


    socket::msg_flags operator &(socket::msg_flags a, socket::msg_flags b) noexcept;
    socket::msg_flags operator ^(socket::msg_flags a, socket::msg_flags b) noexcept;
    socket::msg_flags operator |(socket::msg_flags a, socket::msg_flags b) noexcept;
    socket::msg_flags operator ~(socket::msg_flags a) noexcept;


    socket::poll_flags operator &(socket::poll_flags a, socket::poll_flags b) noexcept;
    socket::poll_flags operator ^(socket::poll_flags a, socket::poll_flags b) noexcept;
    socket::poll_flags operator |(socket::poll_flags a, socket::poll_flags b) noexcept;
    socket::poll_flags operator ~(socket::poll_flags a) noexcept;

} // namespace net


#endif
