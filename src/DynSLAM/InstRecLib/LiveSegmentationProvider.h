//
// Created by catphive on 4/8/19.
//

#ifndef INSTRECLIB_LIVESEGMENTATIONPROVIDER_H
#define INSTRECLIB_LIVESEGMENTATIONPROVIDER_H

#include <memory>
#include <opencv2/core/mat.hpp>

#include "InstanceSegmentationResult.h"
#include "SegmentationProvider.h"


#include "HttpClient.h"

namespace instreclib {
namespace segmentation {

class LiveSegmentationProvider : public SegmentationProvider {

    HttpClient m_client;
    std::string m_host;
    std::string m_port;

    // legacy from precomputed provider
    const std::string seg_folder_;
    int frame_idx_;
    cv::Mat3b *last_seg_preview_;
    const float input_scale_;

public:

    LiveSegmentationProvider(const std::string &seg_folder, int frame_offset, float scale,
            const std::string& host, const std::string& port)
    : m_client(host, port),
    seg_folder_(seg_folder),
    frame_idx_(frame_offset),
    last_seg_preview_(nullptr),
    input_scale_(scale)
    {}

    std::shared_ptr<InstanceSegmentationResult> SegmentFrame(const cv::Mat3b &rgb) override;

    cv::Mat3b *GetSegResult() override;

    const cv::Mat3b *GetSegResult() const override;


private:
    std::shared_ptr<InstanceSegmentationResult> post(const cv::Mat3b &rgb);

};


}  // namespace segmentation
}  // namespace instreclib

#endif //INSTRECLIB_LIVESEGMENTATIONPROVIDER_H
