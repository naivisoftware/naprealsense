
#include "realsenseframefilter.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameFilter)

RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseSpatialFilter)
        RTTI_PROPERTY("Cutoff", &nap::RealSenseSpatialFilter::mCutoff, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseDecFilter)
        RTTI_PROPERTY("Magnitude", &nap::RealSenseDecFilter::mMagnitude, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseDisparityFilter)
        RTTI_PROPERTY("ToDisparity", &nap::RealSenseDisparityFilter::mToDisparity, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseColorizeFilter)
RTTI_END_CLASS

namespace nap
{
    struct RealSenseSpatialFilter::Impl
    {
    public:
        rs2::spatial_filter mSpatFilter;
    };

    bool RealSenseSpatialFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        mImpl->mSpatFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, 2.0f);
        mImpl->mSpatFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, 0.5f);
        mImpl->mSpatFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, 20.0f);

        return true;
    }

    rs2::frame RealSenseSpatialFilter::process(const rs2::frame& frame)
    {
        return mImpl->mSpatFilter.filter::process(frame);
    }

    struct RealSenseDecFilter::Impl
    {
    public:
        rs2::decimation_filter mDecFilter;
    };

    bool RealSenseDecFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        mImpl->mDecFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, mMagnitude);
        return true;
    }

    rs2::frame RealSenseDecFilter::process(const rs2::frame& frame)
    {
        return mImpl->mDecFilter.filter::process(frame);
    }

    struct RealSenseDisparityFilter::Impl
    {
    public:
        rs2::disparity_transform mDisparityTransform;
    };

    struct RealSenseColorizeFilter::Impl
    {
    public:
        rs2::colorizer mColorizer;
    };

    bool RealSenseDisparityFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        mImpl->mDisparityTransform = rs2::disparity_transform(mToDisparity);

        return true;
    }

    rs2::frame RealSenseDisparityFilter::process(const rs2::frame& frame)
    {
        return mImpl->mDisparityTransform.filter::process(frame);
    }

    bool RealSenseColorizeFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        return true;
    }

    rs2::frame RealSenseColorizeFilter::process(const rs2::frame& frame)
    {
        return mImpl->mColorizer.process(frame);
    }
}