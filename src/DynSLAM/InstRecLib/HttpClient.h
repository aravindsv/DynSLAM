
#ifndef INSTRECLIB_HTTPCLIENT_H
#define INSTRECLIB_HTTPCLIENT_H


#include <string>
#include <sstream>


#include <curl/curl.h>

namespace instreclib {
namespace segmentation {

class HttpClient {

    CURL *m_handle;

    curl_slist *m_headers;

    char m_err_buf[CURL_ERROR_SIZE];

    std::ostringstream m_out_stream;

public:
    HttpClient(const std::string& host, const std::string& port);
    ~HttpClient();

    std::string post(const std::string &data);

    static void global_cleanup();

    static void global_init();


private:

    std::string get_err() {
        return std::string(m_err_buf);
    }

    static size_t write_data_cb(void *buffer, size_t size, size_t nmemb, void *userp);
};

}
}

#endif //INSTRECLIB_HTTPCLIENT_H
