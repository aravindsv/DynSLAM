//
// Created by catphive on 4/8/19.
//

#include "LiveSegmentationProvider.h"

#include <string>
#include <iostream>
#include <chrono>
#include <cstring>
#include <vector>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SegmentationDataset.h"
#include "Timer.h"


//#include <boost/process.hpp>

#include "opencv2/opencv.hpp"

#include "server_result.pb.h"


#include "InstanceSegmentationResult.h"

#include "../Utils.h"

using namespace std;


namespace instreclib {
namespace segmentation {


static void opencv_rgb_to_server(const cv::Mat3b &rgb, ServerMatRGB *server_mat) {

    cout << "rgb rows " << rgb.rows << " cols " << rgb.cols << endl;

//    size_t buf_len = rgb.total() * rgb.elemSize();

    vector<uchar> buf;

    vector<int> params;
    params.push_back(cv::IMWRITE_JPEG_OPTIMIZE);
    params.push_back(1);
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(30);
    params.push_back(cv::IMWRITE_JPEG_LUMA_QUALITY);
    params.push_back(30);
    cv::imencode(".jpg", rgb, buf, params);

    server_mat->set_array((void *) buf.data(), buf.size());

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


std::shared_ptr<InstanceSegmentationResult> make_segmentation_result(const ServerResult &server_result, float input_scale) {

    int min_area = static_cast<int>(round(45 * 45 * input_scale));

    cout << "TESTING!!!! input_scale = " << input_scale << endl;

    // copied rescaling constant from precomputed.
/// Rescale factors used for RGB and depth masking of object instances.
    float kCopyMaskRescaleFactor = 1.00f;
    float kDeleteMaskRescaleFactor = 1.2f;

/// \brief Rescale factor used for scene flow masking of instances.
/// Smaller => fewer sparse scene flow vectors from, e.g., the background behind the object.
    float kConservativeMaskRescaleFactor = 0.97f;

    vector<InstanceDetection> detections;

    for (const ServerDetection& detection : server_result.detections()) {

        // PrecomputedSegmentationProvider::ReadInstanceInfo shows how to build this stuff.
        auto bounding_box = make_bbox(detection.mask().bbox());

        if (bounding_box.GetArea() <= min_area) {
            continue;
        }

        cv::Mat1b* mat = server_mat_to_opencv(detection.mask().mat());

        auto copy_mask = make_shared<utils::Mask>(bounding_box, mat);
        auto delete_mask = make_shared<utils::Mask>(*copy_mask);
        auto conservative_mask = make_shared<utils::Mask>(*copy_mask);

        copy_mask->Rescale(kCopyMaskRescaleFactor);
        float del_scale = kDeleteMaskRescaleFactor;
        // Adapt rescaling for distant objects. Constant chosen empirically.
        if (bounding_box.GetArea() < min_area * 1.375) {
            del_scale *= 1.2f;
        }
        delete_mask->Rescale(del_scale);
        conservative_mask->Rescale(kConservativeMaskRescaleFactor);


        // Need to generate delete mask as least... just using the same for all 3 now.
        InstanceDetection instance_detection(detection.class_probability(), detection.class_id(),
                                             copy_mask, delete_mask, conservative_mask, &kPascalVoc2012);

        detections.push_back(instance_detection);
    }

    // TODO: pascal should be wrong. Set up POCO categories.
    return make_shared<InstanceSegmentationResult>(&kPascalVoc2012, detections, 1000);
}

static void disp(cv::Mat mat) {
    namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
    imshow("Display window", mat);
    cv::waitKey(0);
}



std::shared_ptr<InstanceSegmentationResult> LiveSegmentationProvider::post(const cv::Mat3b &rgb) {

    Timer t1("live seg post: serialization");

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    ServerInput input;

    opencv_rgb_to_server(rgb, input.mutable_image());

    string data;

    if (!input.SerializeToString(&data)) {
        throw runtime_error("Could not serialize ServerInput");
    }

    t1.print();

    Timer t2("live seg post: calling http client post");

    string output = m_client.post(data);

    t2.print();

    Timer t3("live seg post: parsing");

    ServerResult server_result;

    if (!server_result.ParseFromString(output)) {
        throw runtime_error("Failed to parse ServerResult");
    }

    auto tmp = make_segmentation_result(server_result, input_scale_);

    t3.print();
    return tmp;
}

std::shared_ptr<InstanceSegmentationResult> LiveSegmentationProvider::SegmentFrame(const cv::Mat3b &rgb) {

    // Reusing precomputed code for preview.
    // TODO: not sure if necessary? Where is preview used?
    if (last_seg_preview_ == nullptr) {
        last_seg_preview_ = new cv::Mat3b(rgb.rows, rgb.cols);
    }
    stringstream img_fpath_ss;
    img_fpath_ss << this->seg_folder_ << "/"
                 << "cls_" << setfill('0') << setw(6) << this->frame_idx_ << ".png";
    const string img_fpath = img_fpath_ss.str();
    if (! dynslam::utils::FileExists(img_fpath)) {
        throw runtime_error(dynslam::utils::Format("Unable to find segmentation preview at [%s].",
                                                   img_fpath.c_str()));
    }
    cv::Mat seg_preview = cv::imread(img_fpath);
    cv::resize(seg_preview, *last_seg_preview_, cv::Size(), 1.0 / input_scale_, 1.0 / input_scale_, cv::INTER_LINEAR);

    if (! last_seg_preview_->data || last_seg_preview_->cols == 0 || last_seg_preview_->rows == 0) {
        throw runtime_error(dynslam::utils::Format(
                "Could not read segmentation preview image from file [%s].",
                img_fpath.c_str()));
    }

    // end preview code.

    auto t3 = chrono::steady_clock::now();

//    cv::Mat3b rgb_shrunk;
//    cv::resize(rgb, rgb_shrunk, cv::Size(), 0.25, 0.25, cv::INTER_LINEAR);
    auto result_ptr = post(rgb);

    auto t4 = chrono::steady_clock::now();

    cout << "TESTING: ellapsed duration: " << chrono::duration<double>(t4 - t3).count() << endl;

    return result_ptr;

//        cout << "TESTING: SegmentFrame called" << endl;
//        cout << "rgb ros " << rgb.rows << " cols " << rgb.cols << endl;
//
//        size_t buf_len = rgb.total() * rgb.elemSize();
//
//        cout << "buf_len = " << buf_len << endl;
//
//        string result = m_client.post(string((char*)rgb.data, buf_len));

}

cv::Mat3b *LiveSegmentationProvider::GetSegResult() {
    return this->last_seg_preview_;
}

const cv::Mat3b *LiveSegmentationProvider::GetSegResult() const {
    return this->last_seg_preview_;
}
}  // namespace segmentation
}  // namespace instreclib
