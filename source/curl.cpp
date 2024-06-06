// SPDX-License-Identifier: MIT

#include "curl.hpp"

#include "logging.hpp"


namespace curl {

    error::error(CURLcode code)  :
        std::runtime_error{curl_easy_strerror(code)},
        code{code}
    {}


    void
    check(CURLcode code)
    {
        if (code == CURLE_OK)
            return;
        throw error{code};
    }


    global::global()
    {
        check(curl_global_init(CURL_GLOBAL_DEFAULT ));
    }


    global::~global()
    {
        curl_global_cleanup();
    }



    handle::handle() :
        h{curl_easy_init()}
    {
        init();
    }


    handle::handle(const handle& other) :
        h{curl_easy_duphandle(other.h)}
    {
        init();
    }


    handle::~handle()
    {
        curl_easy_cleanup(h);
    }


    void
    handle::init()
    {
        if (!h)
            throw std::logic_error{"curl easy handle is null"};

        error_buffer = std::make_unique<char[]>(CURL_ERROR_SIZE);

        check(curl_easy_setopt(h, CURLOPT_ERRORBUFFER, error_buffer.get()));

        check(curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, &handle::write_callback));
        check(curl_easy_setopt(h, CURLOPT_WRITEDATA, this));
    }


    std::size_t
    handle::write_callback(char* buffer,
                           std::size_t /*size*/,
                           std::size_t nmemb,
                           void* ctx)
    {
        handle* h = static_cast<handle*>(ctx);
        try {
            if (!h)
                throw std::logic_error{"null handle"};
            return h->on_recv(buffer, nmemb);
        }
        catch (std::exception& e) {
            logging::printf("curl::handle::write_callback(): %s", e.what());
            return CURL_WRITEFUNC_ERROR;
        }
    }


    std::size_t
    handle::on_recv(const char* buffer, std::size_t size)
    {
        result.append(buffer, size);
        return size;
    }


    void
    handle::setopt(CURLoption option, bool arg)
    {
        long enable = arg;
        check(curl_easy_setopt(h, option, enable));
    }


    void
    handle::setopt(CURLoption option, const std::string& arg)
    {
        check(curl_easy_setopt(h, option, arg.c_str()));
    }


    // convenience setters

    void
    handle::set_followlocation(bool enable)
    {
        setopt(CURLOPT_FOLLOWLOCATION, enable);
    }


    void
    handle::set_url(const std::string& url)
    {
        setopt(CURLOPT_URL, url);
    }


    void
    handle::set_useragent(const std::string& agent)
    {
        setopt(CURLOPT_USERAGENT, agent);
    }


    void
    handle::perform()
    {
        check(curl_easy_perform(h));
    }


} // namespace curl
