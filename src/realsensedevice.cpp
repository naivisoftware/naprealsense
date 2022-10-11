#include "realsensedevice.h"
#include "realsenseservice.h"
#include "realsenseframesetlistenercomponent.h"
#include "realsenseframesetfilter.h"

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseStreamDescription)
    RTTI_PROPERTY("Format", &nap::RealSenseStreamDescription::mFormat, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Stream", &nap::RealSenseStreamDescription::mStream, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseDevice)
    RTTI_CONSTRUCTOR(nap::RealSenseService&)
    RTTI_PROPERTY("Serial", &nap::RealSenseDevice::mSerial, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("MaxFrameSize", &nap::RealSenseDevice::mMaxFrameSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Streams", &nap::RealSenseDevice::mStreams, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("Filters", &nap::RealSenseDevice::mFilters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{
    struct RealSenseDevice::Impl
    {
    public:
        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline mPipe;

        // Frame queue
        rs2::frame_queue mFrameQueue;
    };

    RealSenseDevice::RealSenseDevice(RealSenseService &service) : mService(service)
    {
    }


    bool RealSenseDevice::start(utility::ErrorState &errorState)
    {
        if(!mRun.load())
        {
            mImplementation = std::make_unique<Impl>();
            mImplementation->mFrameQueue = rs2::frame_queue(mMaxFrameSize);

            if(!mSerial.empty())
            {
                if(!errorState.check(mService.hasSerialNumber(mSerial),
                                     utility::stringFormat("Device with serial number %s is not connected", mSerial.c_str())))
                    return false;
            }

           if(!mService.registerDevice(this, errorState))
               return false;

            rs2::config cfg;
            std::vector<ERealSenseStreamType> stream_types;
            for(const auto& stream : mStreams)
            {
                auto stream_type = stream->mStream;
                if(std::find_if(stream_types.begin(),
                                stream_types.end(),
                                [stream_type](ERealSenseStreamType other)
                                    { return stream_type == other; }) != stream_types.end())
                {
                    errorState.fail("Cannot open multiple streams of the same stream type!");
                    return false;
                }

                stream_types.emplace_back(stream_type);

                rs2_stream rs2_stream_type      = static_cast<rs2_stream>(stream->mStream);
                rs2_format rs2_stream_format    = static_cast<rs2_format>(stream->mFormat);
                cfg.enable_stream(rs2_stream_type, rs2_stream_format);
            }

            //

            if(!mSerial.empty())
                cfg.enable_device(mSerial);

            try
            {
                mImplementation->mPipe.start(cfg);

                for(auto& stream : mStreams)
                {
                    if(stream->mStream == ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR)
                    {
                        auto intrincics_rs2 = mImplementation->mPipe
                                                    .get_active_profile()
                                                    .get_stream(static_cast<rs2_stream>(stream->mStream))
                                                    .as<rs2::video_stream_profile>()
                                                    .get_intrinsics();

                        RealSenseCameraIntrincics intrincics;
                        intrincics.mHeight = intrincics_rs2.height;
                        intrincics.mWidth = intrincics_rs2.width;
                        for(int i = 0 ; i < 5; i++)
                        {
                            intrincics.mCoeffs[i] = intrincics_rs2.coeffs[i];
                        }
                        intrincics.mFX = intrincics_rs2.fx;
                        intrincics.mFY = intrincics_rs2.fy;
                        intrincics.mPPX = intrincics_rs2.ppx;
                        intrincics.mPPY = intrincics_rs2.ppy;
                        intrincics.mModel = static_cast<ERealSenseDistortionModels>(intrincics_rs2.model);
                        mCameraIntrinsics.emplace(ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR, intrincics);
                    }else if(stream->mStream == ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH)
                    {
                        auto intrincics_rs2 = mImplementation->mPipe
                            .get_active_profile()
                            .get_stream(static_cast<rs2_stream>(stream->mStream))
                            .as<rs2::video_stream_profile>()
                            .get_intrinsics();

                        RealSenseCameraIntrincics intrincics;
                        intrincics.mHeight = intrincics_rs2.height;
                        intrincics.mWidth = intrincics_rs2.width;
                        for(int i = 0 ; i < 5; i++)
                        {
                            intrincics.mCoeffs[i] = intrincics_rs2.coeffs[i];
                        }
                        intrincics.mFX = intrincics_rs2.fx;
                        intrincics.mFY = intrincics_rs2.fy;
                        intrincics.mPPX = intrincics_rs2.ppx;
                        intrincics.mPPY = intrincics_rs2.ppy;
                        intrincics.mModel = static_cast<ERealSenseDistortionModels>(intrincics_rs2.model);
                        mCameraIntrinsics.emplace(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH, intrincics);
                    }
                }

            }catch(const rs2::error& e)
            {
                errorState.fail(utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                      e.get_failed_function().c_str(),
                                                      e.get_failed_args().c_str(),
                                                      e.what()));
                return false;
            }
            catch(const std::exception& e)
            {
                errorState.fail(e.what());
                return false;
            }

            mRun.store(true);
            mCaptureTask = std::async(std::launch::async, std::bind(&RealSenseDevice::process, this));

            return true;
        }

        errorState.fail("RealSenseDevice already started.");

        return false;
    }


    float RealSenseDevice::getDepthScale() const
    {
        auto dpth = mImplementation->mPipe.get_active_profile().get_device().first<rs2::depth_sensor>();
        return dpth.get_depth_scale();
    }


    void RealSenseDevice::stop()
    {
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            mImplementation->mPipe.stop();

            mService.removeDevice(this);
        }
    }

    void RealSenseDevice::onDestroy()
    {
        stop();
    }


    void RealSenseDevice::update(double deltaTime)
    {
    }


    void RealSenseDevice::addFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it == mFrameSetListeners.end()); // device already exists
        mFrameSetListeners.emplace_back(frameSetListener);
    }


    void RealSenseDevice::removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it != mFrameSetListeners.end()); // device does not exist
        mFrameSetListeners.erase(it);
    }


    void RealSenseDevice::process()
    {
        while(mRun.load())
        {
            rs2::frameset data;
            if(mImplementation->mPipe.poll_for_frames(&data))
            {
                for(auto& filter : mFilters)
                {
                    data = filter->process(data);
                }

                for(auto* frameset_listener : mFrameSetListeners)
                {
                    frameset_listener->trigger(data);
                }
            }
        }
    }
}