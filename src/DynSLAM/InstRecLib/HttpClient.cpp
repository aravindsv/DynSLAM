

#include "HttpClient.h"
#include <iostream>
#include <stdexcept>
#include "../Utils.h"

#include "SimpleTimer.h"


using namespace std;

namespace instreclib {
namespace segmentation {

using namespace dynslam::utils;

static bool global_init_called = false;

HttpClient::HttpClient(const std::string& host, const std::string& port) {

    string url = Format("http://%s:%s/post", host.c_str(), port.c_str());

    m_err_buf[0] = 0;

    global_init();

    m_handle = curl_easy_init();

    if (!m_handle) {
        throw runtime_error("curl_easy_init failed");
    }

    curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());

    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_data_cb);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(m_handle, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_err_buf);
//    curl_easy_setopt(m_handle, CURLOPT_ACCEPT_ENCODING, "");

//    m_headers = NULL;
//    m_headers = curl_slist_append(m_headers, "Content-Type: application/binary");
//    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, m_headers);
}

HttpClient::~HttpClient() {

    if (m_handle != nullptr) {
        curl_easy_cleanup(m_handle);
    }

    if (m_headers != nullptr) {
        curl_slist_free_all(m_headers);
    }
}

string HttpClient::post(const string &data) {

    SimpleTimer t1("HttpClient::post: setup");

    // clear the output buffer.
    m_out_stream.str("");

    // TODO: mime should have its own class with destructor.
    curl_mime *multipart = curl_mime_init(m_handle);
    curl_mimepart *part = curl_mime_addpart(multipart);
    curl_mime_name(part, "server_input");
    curl_mime_filename(part, "server_input");
    curl_mime_data(part, data.data(), data.size());
    curl_mime_type(part, "application/octet-stream");

    /* Set the form info */
    curl_easy_setopt(m_handle, CURLOPT_MIMEPOST, multipart);

    t1.print();

    SimpleTimer t2("HttpClient::post: perform");

    CURLcode code = curl_easy_perform(m_handle);

    t2.print();

    curl_mime_free(multipart);

    if (code != CURLE_OK) {
        throw runtime_error("curl_easy_perform failed: " + get_err());
    }

    return m_out_stream.str();

}


size_t HttpClient::write_data_cb(void *buffer, size_t size, size_t nmemb, void *user_ptr) {

    SimpleTimer t1("HttpClient::write_data_cb");

    string output((char *) buffer, size * nmemb);

    HttpClient *client = (HttpClient *) user_ptr;

    // accumulate buffer into output stream.
    client->m_out_stream << output;

    return size * nmemb;
}


void HttpClient::global_cleanup() {
    curl_global_cleanup();
}

void HttpClient::global_init() {
    CURLcode code{};
    if (!global_init_called) {
        if ((code = curl_global_init(CURL_GLOBAL_SSL)) != CURLE_OK) {
            cerr << curl_easy_strerror(code) << endl;
        }
    }
}

}
}