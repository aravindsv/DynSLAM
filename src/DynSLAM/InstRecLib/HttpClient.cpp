

#include "HttpClient.h"
#include <iostream>
#include <stdexcept>


using namespace std;

namespace instreclib {
namespace segmentation {

static bool global_init_called = false;

HttpClient::HttpClient() {



    m_err_buf[0] = 0;

    global_init();

    m_handle = curl_easy_init();

    if (!m_handle) {
        throw runtime_error("curl_easy_init failed");
    }

    curl_easy_setopt(m_handle, CURLOPT_URL, "http://bb10.ri.cs.cmu.edu:5000/post");
//    curl_easy_setopt(m_handle, CURLOPT_URL, "https://postman-echo.com/post");
//    curl_easy_setopt(m_handle, CURLOPT_URL, "https://postman-echo.com/get?foo1=bar1&foo2=bar2");
//    curl_easy_setopt(m_handle, CURLOPT_URL, "http://google.com");

    curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, write_data_cb);
    curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(m_handle, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(m_handle, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(m_handle, CURLOPT_ERRORBUFFER, m_err_buf);



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

    // clear the output buffer.
    m_out_stream.str("");

//    curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, data.data());

    /* set the size of the postfields data */
//    curl_easy_setopt(m_handle, CURLOPT_POSTFIELDSIZE, data.size());


    cout << "data: ";
    for (int ii = 0; ii < 10; ii++) {
        cout << (int) data[ii];
    }

    cout << endl;

    // TODO: mime should have its own class with destructor.
    curl_mime *multipart = curl_mime_init(m_handle);
    curl_mimepart *part = curl_mime_addpart(multipart);
    curl_mime_name(part, "server_input");
    curl_mime_filename(part, "server_input");
    curl_mime_data(part, data.data(), data.size());
    curl_mime_type(part, "application/octet-stream");

    /* Set the form info */
    curl_easy_setopt(m_handle, CURLOPT_MIMEPOST, multipart);

    CURLcode code = curl_easy_perform(m_handle);

    curl_mime_free(multipart);

    if (code != CURLE_OK) {
        throw runtime_error("curl_easy_perform failed: " + get_err());
    }

    return m_out_stream.str();

}


size_t HttpClient::write_data_cb(void *buffer, size_t size, size_t nmemb, void *user_ptr) {

//    cerr << "write data cb called" << endl;

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