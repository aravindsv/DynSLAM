

#include "HttpClient.h"
#include <iostream>
#include <stdexcept>
#include <opencv2/core/mat.hpp>
#include <cstring>
#include <vector>

#include <opencv2/highgui/highgui.hpp>
#include "SegmentationDataset.h"


//#include <zip.h>

#include "server_result.pb.h"


using namespace std;

namespace instreclib {
namespace segmentation {

static bool global_init_called = false;

HttpClient::HttpClient() {

    GOOGLE_PROTOBUF_VERIFY_VERSION;

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


void opencv_rgb_to_server(const cv::Mat3b &rgb, ServerMatRGB *server_mat) {

    cout << "rgb rows " << rgb.rows << " cols " << rgb.cols << endl;

    size_t buf_len = rgb.total() * rgb.elemSize();

    server_mat->set_array((void *) rgb.data, buf_len);

    server_mat->set_dim1(rgb.rows);
    server_mat->set_dim2(rgb.cols);
}

static cv::Mat1b* server_mat_to_opencv(const ServerMat &server_mat) {
    cv::Mat1b* mat = new cv::Mat1b(server_mat.dim1(), server_mat.dim2(), uchar(0));

    size_t buf_len = mat->total() * mat->elemSize();

    if (buf_len != server_mat.array().size()) {
        cerr << "ERRROR!!!!!!!! server_mat_to_opencv wrote wrong number of bytes" << endl;

        delete mat;
        throw logic_error("server_mat_to_opencv broken");
    }

    memcpy(mat->data, server_mat.array().data(), server_mat.array().size());

    return mat;
}

static utils::BoundingBox make_bbox(const ServerBoundingBox& bbox) {
    return utils::BoundingBox((int)bbox.x0(), (int)bbox.y0(), (int)bbox.x1(), (int)bbox.y1());
}

static std::shared_ptr<InstanceSegmentationResult> make_segmentation_result(const ServerResult &server_result) {

    vector<InstanceDetection> detections;

    for (const ServerDetection& detection : server_result.detections()) {

        // PrecomputedSegmentationProvider::ReadInstanceInfo shows how to build this stuff.
        auto bbox = make_bbox(detection.mask().bbox());
        cv::Mat1b* mat = server_mat_to_opencv(detection.mask().mat());
        auto mask = make_shared<utils::Mask>(bbox, mat);

        // Need to generate delete mask as least... just using the same for all 3 now.
        InstanceDetection instance_detection(detection.class_probability(), detection.class_id(),
                mask, mask, mask, &kPascalVoc2012);

        detections.push_back(instance_detection);
    }

    // TODO: pascal should be wrong. Set up POCO categories.
    return make_shared<InstanceSegmentationResult>(&kPascalVoc2012, detections, 1000);
}

std::shared_ptr<InstanceSegmentationResult> HttpClient::post(const cv::Mat3b &rgb) {

    ServerInput input;

    opencv_rgb_to_server(rgb, input.mutable_image());

    string data;

    if (!input.SerializeToString(&data)) {
        throw runtime_error("Could not serialize ServerInput");
    }

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

    ServerResult server_result;

    if (!server_result.ParseFromString(m_out_stream.str())) {
        throw runtime_error("Failed to parse ServerResult");
    }

    return make_segmentation_result(server_result);
}

void disp(cv::Mat mat) {
    namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
    imshow("Display window", mat);
    cv::waitKey(0);
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