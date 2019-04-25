//
// Created by catphive on 4/8/19.
//

#include "LiveSegmentationProvider.h"

#include <string>
#include <iostream>
#include <chrono>

//#include <boost/process.hpp>

using namespace std;


namespace instreclib {
namespace segmentation {


    std::shared_ptr<InstanceSegmentationResult> LiveSegmentationProvider::SegmentFrame(const cv::Mat3b &rgb) {

        auto t1 = chrono::steady_clock::now();
        auto result_ptr = m_client.post(rgb);

        auto t2 = chrono::steady_clock::now();

        cout << "TESTING: ellapsed duration: " << chrono::duration<double>(t2 - t1).count() << endl;

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

        return nullptr;
    }

    const cv::Mat3b *LiveSegmentationProvider::GetSegResult() const {
        return nullptr;
    }
}  // namespace segmentation
}  // namespace instreclib
