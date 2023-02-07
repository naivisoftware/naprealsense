
#include "realsenseframefilter.h"
#include "realsensedevice.h"

#include <rs.hpp>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameFilter)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseSpatialFilter)
    RTTI_PROPERTY("Magnitude", &nap::RealSenseSpatialFilter::mMagnitude, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("SmoothAlpha", &nap::RealSenseSpatialFilter::mSmoothAlpha, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("SmoothDelta", &nap::RealSenseSpatialFilter::mSmoothDelta, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseDecFilter)
        RTTI_PROPERTY("Magnitude", &nap::RealSenseDecFilter::mMagnitude, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseColorizeFilter)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameFilter
    //////////////////////////////////////////////////////////////////////////

    RealSenseFrameFilter::RealSenseFrameFilter() = default;


    RealSenseFrameFilter::~RealSenseFrameFilter() = default;

    //////////////////////////////////////////////////////////////////////////
    // RealSenseSpatialFilter::Impl
    //////////////////////////////////////////////////////////////////////////

    struct RealSenseSpatialFilter::Impl
    {
    public:
        rs2::spatial_filter mSpatFilter;
    };

    //////////////////////////////////////////////////////////////////////////
    // RealSenseSpatialFilter
    //////////////////////////////////////////////////////////////////////////

    RealSenseSpatialFilter::RealSenseSpatialFilter() = default;


    RealSenseSpatialFilter::~RealSenseSpatialFilter() = default;


    bool RealSenseSpatialFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        try
        {
            mImpl->mSpatFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, mMagnitude);
            mImpl->mSpatFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, mSmoothAlpha);
            mImpl->mSpatFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, mSmoothDelta);
        }catch(std::exception& e)
        {
            errorState.fail(e.what());
            return false;
        }

        return true;
    }

    rs2::frame RealSenseSpatialFilter::process(const rs2::frame& frame)
    {
        return mImpl->mSpatFilter.filter::process(frame);
    }

    //////////////////////////////////////////////////////////////////////////
    // RealSenseDecFilter::Impl
    //////////////////////////////////////////////////////////////////////////

    struct RealSenseDecFilter::Impl
    {
    public:
        rs2::decimation_filter mDecFilter;
    };

    //////////////////////////////////////////////////////////////////////////
    // RealSenseDecFilter
    //////////////////////////////////////////////////////////////////////////

    RealSenseDecFilter::RealSenseDecFilter() = default;


    RealSenseDecFilter::~RealSenseDecFilter() = default;


    bool RealSenseDecFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        try
        {
            mImpl->mDecFilter.set_option(RS2_OPTION_FILTER_MAGNITUDE, mMagnitude);
        }catch(std::exception& e)
        {
            errorState.fail(e.what());
            return false;
        }

        return true;
    }

    rs2::frame RealSenseDecFilter::process(const rs2::frame& frame)
    {
        return mImpl->mDecFilter.filter::process(frame);
    }

    //////////////////////////////////////////////////////////////////////////
    // RealSenseColorizeFilter::Impl
    //////////////////////////////////////////////////////////////////////////

    struct RealSenseColorizeFilter::Impl
    {
    public:
        rs2::colorizer mColorizer;
    };

    //////////////////////////////////////////////////////////////////////////
    // RealSenseDecFilter
    //////////////////////////////////////////////////////////////////////////

    RealSenseColorizeFilter::RealSenseColorizeFilter() = default;


    RealSenseColorizeFilter::~RealSenseColorizeFilter() = default;


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
