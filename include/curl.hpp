// SPDX-License-Identifier: MIT

#ifndef CURL_HPP
#define CURL_HPP

#include <memory>
#include <stdexcept>            // runtime_error
#include <string>

#include <curl/curl.h>


namespace curl {

    struct error : std::runtime_error {

        CURLcode code;

        error(CURLcode code);

    };


    struct global {

        global();

        ~global();

    };


    class handle {

        CURL* h;
        std::unique_ptr<char[]> error_buffer;

        void init();

        // static int sockopt_callback(handle* h, curl_socket_t fd, curlsocktype purpose);

        static
        std::size_t
        write_callback(char* buffer, std::size_t size, std::size_t nmemb, void* ctx);

    protected:

        virtual std::size_t on_recv(const char* buffer, std::size_t size);


    public:

        std::string result;


        handle();

        handle(const handle& other);

        ~handle();


        void setopt(CURLoption option, bool arg);
        void setopt(CURLoption option, const std::string& arg);


        // convenience setters

        void set_followlocation(bool enable);
        void set_url(const std::string& url);
        void set_useragent(const std::string& agent);

        void perform();

    };

} // namespace curl


#endif
