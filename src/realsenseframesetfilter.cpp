#include "realsenseframesetfilter.h"
#include "realsensedevice.h"

#include <rs.hpp>
#include <concurrentqueue.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameSetFilter)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetAlignFilter)
    RTTI_PROPERTY("Align To", &nap::RealSenseFrameSetAlignFilter::mStreamType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetCutDistanceFilter)
        RTTI_PROPERTY("CuttingRange", &nap::RealSenseFrameSetCutDistanceFilter::mCuttingRange, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::RealSenseFrameHolesFillingFilter::EHoleFilling)
    RTTI_ENUM_VALUE(nap::RealSenseFrameHolesFillingFilter::EHoleFilling::FARTHEST_FROM_AROUND, "FarthestFromAround"),
    RTTI_ENUM_VALUE(nap::RealSenseFrameHolesFillingFilter::EHoleFilling::FILL_FROM_LEFT, "FillFromLeft"),
    RTTI_ENUM_VALUE(nap::RealSenseFrameHolesFillingFilter::EHoleFilling::NEAREST_FROM_AROUND, "NearestFromAround")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RealSenseFrameHolesFillingFilter)
    RTTI_PROPERTY("HoleFilling", &nap::RealSenseFrameHolesFillingFilter::mHoleFilling, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseTemporalFilter)
        RTTI_PROPERTY("SmoothAlpha", &nap::RealSenseTemporalFilter::mSmoothAlpha, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("SmoothDelta", &nap::RealSenseTemporalFilter::mSmoothDelta, nap::rtti::EPropertyMetaData::Default)
        RTTI_PROPERTY("PersintencyIndex", &nap::RealSenseTemporalFilter::mPersistencyIndex, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

void remove_background(const rs2::depth_frame& depth_frame, rs2::video_frame& other_frame, float depth_scale, glm::vec2 clipping_dist);

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameSetFilter::RealSenseFrameSetFilter(){}


    RealSenseFrameSetFilter::~RealSenseFrameSetFilter(){}


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetAlignFilter::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseFrameSetAlignFilter::Impl
    {
    public:
        Impl(rs2_stream streamType) :
            mAlign(streamType){}

        rs2::align mAlign;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetAlignFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameSetAlignFilter::RealSenseFrameSetAlignFilter(){}


    RealSenseFrameSetAlignFilter::~RealSenseFrameSetAlignFilter(){}


    bool RealSenseFrameSetAlignFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>(static_cast<rs2_stream>(mStreamType));

        return true;
    }


    rs2::frameset RealSenseFrameSetAlignFilter::process(RealSenseDevice* device, const rs2::frameset& frameset)
    {
        auto processed = mImpl->mAlign.process(frameset);
        return processed;
    }


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetAlignFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameSetCutDistanceFilter::RealSenseFrameSetCutDistanceFilter(){}


    RealSenseFrameSetCutDistanceFilter::~RealSenseFrameSetCutDistanceFilter(){}


    bool RealSenseFrameSetCutDistanceFilter::init(utility::ErrorState &errorState)
    {
        return true;
    }


    rs2::frameset RealSenseFrameSetCutDistanceFilter::process(RealSenseDevice* device, const rs2::frameset& frameset)
    {
        glm::vec2 range;
        {
            std::lock_guard<std::mutex> l(mMutex);
            range = mCuttingRange;
        }

        auto frame = frameset.first(RS2_STREAM_DEPTH).as<rs2::depth_frame>();
        remove_background(frame, frame, device->getDepthScale(), range);
        return frameset;
    }


    void RealSenseFrameSetCutDistanceFilter::changeCuttingRange(const glm::vec2& range)
    {
        std::lock_guard<std::mutex> l(mMutex);
        mCuttingRange = range;
    }


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameHolesFillingFilter::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseFrameHolesFillingFilter::Impl
    {
    public:
        Impl(){}

        rs2::hole_filling_filter mFilter;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameHolesFillingFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameHolesFillingFilter::RealSenseFrameHolesFillingFilter(){}


    RealSenseFrameHolesFillingFilter::~RealSenseFrameHolesFillingFilter(){}


    bool RealSenseFrameHolesFillingFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        mImpl->mFilter.set_option(RS2_OPTION_HOLES_FILL, static_cast<int>(mHoleFilling));

        return true;
    }


    rs2::frameset RealSenseFrameHolesFillingFilter::process(RealSenseDevice* device, const rs2::frameset& frameset)
    {
        auto frame = frameset.first(RS2_STREAM_DEPTH).as<rs2::depth_frame>();
        frame.apply_filter(mImpl->mFilter);
        return frameset;
    }


    //////////////////////////////////////////////////////////////////////////
    // RealSenseTemporalFilter::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseTemporalFilter::Impl
    {
    public:
        Impl(){}

        rs2::temporal_filter mFilter;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameHolesFillingFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseTemporalFilter::RealSenseTemporalFilter(){}


    RealSenseTemporalFilter::~RealSenseTemporalFilter(){}


    bool RealSenseTemporalFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        mImpl->mFilter.set_option(RS2_OPTION_FILTER_SMOOTH_ALPHA, mSmoothAlpha);
        mImpl->mFilter.set_option(RS2_OPTION_FILTER_SMOOTH_DELTA, mSmoothDelta);
        mImpl->mFilter.set_option(RS2_OPTION_HOLES_FILL, mPersistencyIndex);

        return true;
    }


    rs2::frameset RealSenseTemporalFilter::process(RealSenseDevice* device, const rs2::frameset& frameset)
    {
        auto frame = frameset.first(RS2_STREAM_DEPTH).as<rs2::depth_frame>();
        frame.apply_filter(mImpl->mFilter);
        return frameset;
    }
}

void remove_background(const rs2::depth_frame& depth_frame, rs2::video_frame& other_frame, float depth_scale, glm::vec2 clipping_dist)
{
    const uint16_t* p_depth_frame = reinterpret_cast<const uint16_t*>(depth_frame.get_data());
    uint8_t* p_other_frame = reinterpret_cast<uint8_t*>(const_cast<void*>(other_frame.get_data()));

    int width = other_frame.get_width();
    int height = other_frame.get_height();
    int other_bpp = other_frame.get_bytes_per_pixel();

#pragma omp parallel for schedule(dynamic) //Using OpenMP to try to parallelise the loop
    for (int y = 0; y < height; y++)
    {
        auto depth_pixel_index = y * width;
        for (int x = 0; x < width; x++, ++depth_pixel_index)
        {
            // Get the depth value of the current pixel
            auto pixels_distance = depth_scale * p_depth_frame[depth_pixel_index];

            // Check if the depth value is invalid (<=0) or greater than the threashold
            if (pixels_distance <= clipping_dist.x || pixels_distance > clipping_dist.y)
            {
                // Calculate the offset in other frame's buffer to current pixel
                auto offset = depth_pixel_index * other_bpp;

                // Set pixel to "background" color
                std::memset(&p_other_frame[offset], 0, other_bpp);
            }
        }
    }
}
