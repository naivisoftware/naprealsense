// Local includes
#include "realsensecamera.h"
#include "realsenseservice.h"

// NAP includes
#include <nap/logger.h>
#include <image.h>

// RealSense includes
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseCamera)
    RTTI_CONSTRUCTOR(nap::RealSenseService&)
    RTTI_PROPERTY("Image", &nap::RealSenseCamera::mImage, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
    class RealSenseCameraCapture
    {
    public:
        // Declare depth colorizer for pretty visualization of depth data
        rs2::colorizer mColorMap;

        // Declare rates printer for showing streaming rates of the enabled streams.
        rs2::rates_printer mPrinter;

        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline mPipe;

        //
        rs2::frame_queue mFrameQueue;
    };


    RealSenseCamera::RealSenseCamera(RealSenseService &service) : RealSenseDevice(service)
    {
    }


    bool RealSenseCamera::init(utility::ErrorState &errorState)
    {
        mCameraCapture = std::make_unique<RealSenseCameraCapture>();

        // Create texture description
        SurfaceDescriptor tex_description;
        tex_description.mWidth = 640;
        tex_description.mHeight = 480;
        tex_description.mColorSpace = EColorSpace::Linear;
        tex_description.mDataType = ESurfaceDataType::BYTE;
        tex_description.mChannels = ESurfaceChannels::RGBA;

        //bool Texture2D::init(const SurfaceDescriptor& descriptor, bool generateMipMaps, void* initialData, VkImageUsageFlags requiredFlags, utility::ErrorState& errorState)
        if(!mImage->init(tex_description, false, 0, errorState))
            return false;

        return true;
    }


    bool RealSenseCamera::onStart(utility::ErrorState &errorState)
    {
        mCameraCapture->mPipe.start();

        return true;
    }


    void RealSenseCamera::update(double deltaTime)
    {
        rs2::frameset data;
        if(mCameraCapture->mFrameQueue.poll_for_frame(&data))
        {
            if(data.size() > 0)
            {
                nap::Logger::info("yey");


                for(const auto& frame : data)
                {
                    const auto& video_frame = frame.as<rs2::video_frame>();
                    mImage->update(video_frame.get_data(), video_frame.get_width(), video_frame.get_height(), video_frame.get_bytes_per_pixel(), ESurfaceChannels::RGBA);
                }
            }
        }
    }


    void RealSenseCamera::onStop()
    {
        mCameraCapture->mPipe.stop();
    }


    void RealSenseCamera::onProcess()
    {
        rs2::frameset data;
        if(mCameraCapture->mPipe.poll_for_frames(&data))
        {
            data.apply_filter(mCameraCapture->mPrinter);
            data.apply_filter(mCameraCapture->mColorMap);

            mCameraCapture->mFrameQueue.enqueue(data);
        }
    }
}