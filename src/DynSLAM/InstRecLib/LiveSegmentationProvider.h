//
// Created by catphive on 4/8/19.
//

#ifndef INSTRECLIB_LIVESEGMENTATIONPROVIDER_H
#define INSTRECLIB_LIVESEGMENTATIONPROVIDER_H

#include <memory>

#include "InstanceSegmentationResult.h"
#include "SegmentationProvider.h"

#include "HttpClient.h"

namespace instreclib {
namespace segmentation {

class LiveSegmentationProvider : public SegmentationProvider {

    HttpClient m_client;

public:
    std::shared_ptr<InstanceSegmentationResult> SegmentFrame(const cv::Mat3b &rgb) override;

    cv::Mat3b *GetSegResult() override;

    const cv::Mat3b *GetSegResult() const override;

};


}  // namespace segmentation
}  // namespace instreclib

#endif //INSTRECLIB_LIVESEGMENTATIONPROVIDER_H
