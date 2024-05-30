// SPDX-License-Identifier: MIT

#include <map>
#include <optional>
#include <ranges>               // views::drop()
#include <regex>
#include <stdexcept>            // runtime_error

#include "http_client.hpp"

#include "net/addrinfo.hpp"
#include "net/socket.hpp"
#include "utils.hpp"


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
        req += "User-Agent: " PLUGIN_NAME "/" PLUGIN_VERSION " (Wii U; Aroma)" + CRLF;
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


    // Not very efficient, read one byte at a time.
    std::string
    recv_until(net::socket& sock,
               const std::string& end_token)
    {
        std::string result;

        char buffer[1];
        while (true) {
            if (sock.recv(buffer, 1) == 0)
                break;
            result.append(buffer, 1);

            // if we found the end token, remove it from the result and break out
            if (result.ends_with(end_token)) {
                result.resize(result.size() - end_token.size());
                break;
            }
        }

        return result;
    }


    std::string
    get(const std::string& url)
    {
        auto fields = parse_url(url);

        if (fields.protocol != "http")
            throw std::runtime_error{"Protocol '" + fields.protocol + "' not supported."};

        if (!fields.port)
            fields.port = 80;

        net::addrinfo::hints opts { .type = net::socket::type::tcp };
        auto addresses = net::addrinfo::lookup(fields.host,
                                               std::to_string(*fields.port),
                                               opts);
        if (addresses.empty())
            throw std::runtime_error{"Host '" + fields.host + "' has no IP addresses."};

        const auto& host = addresses.front();
        net::socket sock{host.type};

        sock.connect(host.addr);

        auto request = build_request(fields);
        sock.send_all(request.data(), request.size());

        auto header_str = recv_until(sock, CRLF + CRLF);
        auto header = parse_header(header_str);

        if (!header.type.starts_with("text/plain"))
            throw std::runtime_error{"HTTP response is not plain text: \""
                + header.type + "\""};

        std::string result(header.length, '\0');
        sock.recv_all(result.data(), result.size());
        return result;
    }

}
