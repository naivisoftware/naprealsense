
#include "realsenseframefilter.h"
#include "realsensedevice.h"

#include <rs.hpp>

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

RTTI_BEGIN_CLASS(nap::RealSenseCutDistanceFilter)
    RTTI_PROPERTY("Distance", &nap::RealSenseCutDistanceFilter::mDistance, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("RealSenseDevice", &nap::RealSenseCutDistanceFilter::mDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseCopyFrameFilter)
RTTI_END_CLASS

static void remove_background(const rs2::depth_frame& depth_frame, rs2::video_frame& other_frame, float depth_scale, float clipping_dist);

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameFilter::RealSenseFrameFilter(){}


    RealSenseFrameFilter::~RealSenseFrameFilter(){}


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


    RealSenseSpatialFilter::RealSenseSpatialFilter(){}


    RealSenseSpatialFilter::~RealSenseSpatialFilter(){}


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


    RealSenseDecFilter::RealSenseDecFilter(){}


    RealSenseDecFilter::~RealSenseDecFilter(){}


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


    //////////////////////////////////////////////////////////////////////////
    // RealSenseColorizeFilter::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseColorizeFilter::Impl
    {
    public:
        rs2::colorizer mColorizer;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseColorizeFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseColorizeFilter::RealSenseColorizeFilter(){}


    RealSenseColorizeFilter::~RealSenseColorizeFilter(){}


    bool RealSenseColorizeFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        return true;
    }


    rs2::frame RealSenseColorizeFilter::process(const rs2::frame& frame)
    {
        return mImpl->mColorizer.process(frame);
    }


    //////////////////////////////////////////////////////////////////////////
    // RealSenseDisparityFilter::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseDisparityFilter::Impl
    {
    public:
        rs2::disparity_transform mDisparityTransform;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseDisparityFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseDisparityFilter::RealSenseDisparityFilter(){}


    RealSenseDisparityFilter::~RealSenseDisparityFilter(){}


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


    //////////////////////////////////////////////////////////////////////////
    // RealSenseCutDistanceFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseCutDistanceFilter::RealSenseCutDistanceFilter(){}


    RealSenseCutDistanceFilter::~RealSenseCutDistanceFilter(){}


    bool RealSenseCutDistanceFilter::init(utility::ErrorState &errorState)
    {
        return true;
    }


    rs2::frame RealSenseCutDistanceFilter::process(const rs2::frame& frame)
    {
        if(frame.is<rs2::depth_frame>())
        {
            rs2::depth_frame output_frame(frame);
            remove_background(frame.as<rs2::depth_frame>(), output_frame, mDevice->getDepthScale(), mDistance);
            return output_frame;
        }else
        {
            nap::Logger::error(*this, "Frame is not a depth frame");
        }
        return frame;
    }

    //////////////////////////////////////////////////////////////////////////
    // RealSenseCopyFrameFilter::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseCopyFrameFilter::Impl
    {
    public:
        std::unique_ptr<rs2::processing_block> mPb;
        rs2::frame_queue mFrameQueue;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseDisparityFilter
    //////////////////////////////////////////////////////////////////////////


    RealSenseCopyFrameFilter::RealSenseCopyFrameFilter(){}


    RealSenseCopyFrameFilter::~RealSenseCopyFrameFilter(){}


    bool RealSenseCopyFrameFilter::init(utility::ErrorState &errorState)
    {
        mImpl = std::make_unique<Impl>();
        mImpl->mPb = std::make_unique<rs2::processing_block>([](rs2::frame frame, const rs2::frame_source& source)
        {
            assert(frame.is<rs2::depth_frame>() || frame.is<rs2::video_frame>());
            rs2_extension extension = frame.is<rs2::depth_frame>() ? RS2_EXTENSION_DEPTH_FRAME : RS2_EXTENSION_VIDEO_FRAME;

            rs2::frame out = frame;

            // source can be used to allocate new frames and send them out
            // Allocate and copy the content of the original Depth frame to the target
            auto vf = frame.as<rs2::video_frame>();
            rs2::frame tgt = source.allocate_video_frame(vf.get_profile(), frame,
                                                      vf.get_bytes_per_pixel(),
                                                      vf.get_width(),
                                                      vf.get_height(),
                                                      vf.get_stride_in_bytes(),
                                                      extension);

            memmove(const_cast<void *>(tgt.get_data()), frame.get_data(), vf.get_data_size());

            out = tgt;

            source.frame_ready(out);
        });

        mImpl->mPb->start(mImpl->mFrameQueue);

        return true;
    }


    rs2::frame RealSenseCopyFrameFilter::process(const rs2::frame& frame)
    {
        if(frame.is<rs2::depth_frame>() || frame.is<rs2::video_frame>())
        {
            mImpl->mPb->invoke(frame);
            return mImpl->mFrameQueue.wait_for_frame().as<rs2::video_frame>(); // Fetch the result
        }else
        {
            nap::Logger::error(*this, "Frame is not a depth or video frame");
        }

        return frame;
    }
}


void remove_background(const rs2::depth_frame& depth_frame, rs2::video_frame& other_frame, float depth_scale, float clipping_dist)
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
            if (pixels_distance <= 0.f || pixels_distance > clipping_dist)
            {
                // Calculate the offset in other frame's buffer to current pixel
                auto offset = depth_pixel_index * other_bpp;

                // Set pixel to "background" color (0x999999)
                std::memset(&p_other_frame[offset], 0, other_bpp);
            }
        }
    }
}
