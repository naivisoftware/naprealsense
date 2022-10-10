
#include "realsenseframesetfilter.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameSetFilter)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetAlignFilter)
RTTI_END_CLASS

namespace nap
{
    struct RealSenseFrameSetAlignFilter::Impl
    {
    public:
        rs2::align mAlign = rs2::align(RS2_STREAM_DEPTH);
    };

    bool RealSenseFrameSetAlignFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();

        return true;
    }

    rs2::frameset RealSenseFrameSetAlignFilter::process(const rs2::frameset& frameset)
    {
        return mImpl->mAlign.filter::process(frameset);
    }
}