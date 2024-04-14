// SPDX-License-Identifier: MIT

#include <map>
#include <optional>
#include <ranges>
#include <regex>
#include <stdexcept>

#include <sys/socket.h>         // connect()

#include "../include/http_client.hpp"

#include "../include/utils.hpp"


#define LOG(FMT, ...)  WHBLogPrintf(FMT __VA_OPT__(,) __VA_ARGS__)


namespace http {

    const std::string CRLF = "\r\n";


    struct url_fields {
        std::string protocol;
        std::string host;
        std::optional<int> port;
        std::optional<std::string> path;
    };


    url_fields
    parse_url(const std::string& url)
    {
        url_fields fields;

        std::regex re{R"((https?)://([^:/\s]+)(:(\d+))?(/.*)?)",
                      std::regex_constants::ECMAScript};

        std::smatch m;
        if (!regex_match(url, m, re))
            throw std::runtime_error{"failed to parse URL: \"" + url + "\""};

        fields.protocol = m[1];
        fields.host = m[2];
        if (m[4].matched)
            fields.port = std::stoi(m[4]);
        if (m[5].matched)
            fields.path = m[5];
        return fields;
    }


    std::string
    build_request(const url_fields& fields)
    {
        std::string req = "GET ";
        if (fields.path)
            req += *fields.path;
        else
            req += "/";
        req += " HTTP/1.1" + CRLF;
        req += "Host: " + fields.host + CRLF;
        req += "User-Agent: Wii U Time Sync Plugin" + CRLF;
        req += "Accept: text/plain" + CRLF;
        req += "Connection: close" + CRLF;
        req += CRLF;
        return req;
    }


    struct response_header {
        unsigned long length; // We only care about Content-Length.
        std::string type;
    };


    response_header
    parse_header(const std::string input)
    {
        auto lines = utils::split(input, CRLF);
        if (lines.empty())
            throw std::runtime_error{"Empty HTTP response."};

        {
            std::regex re{R"(HTTP/1\.1 (\d+)( (.*))?)",
                          std::regex_constants::ECMAScript};
            std::smatch m;
            if (!regex_match(lines[0], m, re))
                throw std::runtime_error{"Could not parse HTTP response: \""
                                         + lines[0] + "\""};
            int status = std::stoi(m[1]);
            if (status < 200 || status > 299)
                throw std::runtime_error{"HTTP status was " + m[1].str()};
        }

        std::map<std::string, std::string> fields;
        for (const auto& line : lines | std::views::drop(1)) {
            auto key_val = utils::split(line, ": ", 2);
            if (key_val.size() != 2)
                throw std::runtime_error{"invalid HTTP header field: " + line};
            auto key = key_val[0];
            auto val = key_val[1];
            fields[key] = val;
        }

        if (!fields.contains("Content-Length"))
            throw std::runtime_error{"HTTP header is missing mandatory Content-Length field."};

        response_header header;
        header.length = std::stoul(fields.at("Content-Length"));
        header.type = fields.at("Content-Type");

        return header;
    }


    std::string
    get(const std::string& url)
    {
        auto fields = parse_url(url);

        if (fields.protocol != "http")
            throw std::runtime_error{"Protocol '" + fields.protocol + "' not supported."};

        if (!fields.port)
            fields.port = 80;

        utils::addrinfo_query query = {
            .family = AF_INET,
            .socktype = SOCK_STREAM,
            .protocol = IPPROTO_TCP
        };
        auto addresses = utils::get_address_info(fields.host,
                                                 std::to_string(*fields.port),
                                                 query);
        if (addresses.empty())
            throw std::runtime_error{"Host '" + fields.host + "' has no IP addresses."};

        const auto& addr = addresses.front();
        utils::socket_guard sock{addr.family, addr.socktype, addr.protocol};

        if (connect(sock.fd,
                    reinterpret_cast<const struct sockaddr*>(&addr.address),
                    sizeof addr.address) == -1) {
            int e = errno;
            throw std::runtime_error{"connect() failed: " + utils::errno_to_string(e)};
        }

        auto request = build_request(fields);
        utils::send_all(sock.fd, request);

        auto header_str = utils::recv_until(sock.fd, CRLF + CRLF);
        auto header = parse_header(header_str);

        if (!header.type.starts_with("text/plain"))
            throw std::runtime_error{"HTTP response is not plain text: \""
                + header.type + "\""};

        return utils::recv_all(sock.fd, header.length);

    }

}
