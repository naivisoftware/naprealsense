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
    //////////////////////////////////////////////////////////////////////////
    // RealSenseDevice::Impl
    //////////////////////////////////////////////////////////////////////////

    struct RealSenseDevice::Impl
    {
    public:
        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline mPipe;

        // Frame queue
        rs2::frame_queue mFrameQueue;

        // Pipe configuration
        rs2::config mConfig;
    };

    //////////////////////////////////////////////////////////////////////////
    // RealSenseDevice
    //////////////////////////////////////////////////////////////////////////

    RealSenseDevice::RealSenseDevice(RealSenseService &service) : mService(service)
    {}


    RealSenseDevice::~RealSenseDevice() = default;


    bool RealSenseDevice::start(utility::ErrorState &errorState)
    {
        if(!mRun.load())
        {
            // create implementation and framequeue
            mImplementation = std::make_unique<Impl>();
            mImplementation->mFrameQueue = rs2::frame_queue(mMaxFrameSize);

            // Check if serial is available
            if(!mSerial.empty())
            {
                if(!errorState.check(mService.hasSerialNumber(mSerial),
                                     utility::stringFormat("Device with serial number %s is not connected", mSerial.c_str())))
                    return false;
            }

            // set all streams in config
            std::vector<ERealSenseStreamType> stream_types;
            for(const auto &stream: mStreams)
            {
                // check for duplicate stream descriptions
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

                // enable stream in config
                auto rs2_stream_type = static_cast<rs2_stream>(stream->mStream);
                auto rs2_stream_format = static_cast<rs2_format>(stream->mFormat);
                mImplementation->mConfig.enable_stream(rs2_stream_type, rs2_stream_format);
            }

            // set serial in config
            if(!mSerial.empty())
                mImplementation->mConfig.enable_device(mSerial);

            try
            {
                // open pipe
                mImplementation->mPipe.start(mImplementation->mConfig);

                // fetch camera intrinsics for each stream type
                for(auto &stream: mStreams)
                {
                    if(stream->mStream == ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR)
                    {
                        auto intrinsics_rs2 = mImplementation->mPipe
                                .get_active_profile()
                                .get_stream(static_cast<rs2_stream>(stream->mStream))
                                .as<rs2::video_stream_profile>()
                                .get_intrinsics();

                        RealSenseCameraIntrincics intrinsics{};
                        intrinsics.mHeight = intrinsics_rs2.height;
                        intrinsics.mWidth = intrinsics_rs2.width;
                        for(int i = 0;i < 5;i++)
                        {
                            intrinsics.mCoeffs[i] = intrinsics_rs2.coeffs[i];
                        }
                        intrinsics.mFX = intrinsics_rs2.fx;
                        intrinsics.mFY = intrinsics_rs2.fy;
                        intrinsics.mPPX = intrinsics_rs2.ppx;
                        intrinsics.mPPY = intrinsics_rs2.ppy;
                        intrinsics.mModel = static_cast<ERealSenseDistortionModels>(intrinsics_rs2.model);
                        mCameraIntrinsics.emplace(ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR, intrinsics);
                    }else if(stream->mStream == ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH)
                    {
                        auto intrinsics_rs2 = mImplementation->mPipe
                                .get_active_profile()
                                .get_stream(static_cast<rs2_stream>(stream->mStream))
                                .as<rs2::video_stream_profile>()
                                .get_intrinsics();

                        RealSenseCameraIntrincics intrinsics{};
                        intrinsics.mHeight = intrinsics_rs2.height;
                        intrinsics.mWidth = intrinsics_rs2.width;
                        for(int i = 0;i < 5;i++)
                        {
                            intrinsics.mCoeffs[i] = intrinsics_rs2.coeffs[i];
                        }
                        intrinsics.mFX = intrinsics_rs2.fx;
                        intrinsics.mFY = intrinsics_rs2.fy;
                        intrinsics.mPPX = intrinsics_rs2.ppx;
                        intrinsics.mPPY = intrinsics_rs2.ppy;
                        intrinsics.mModel = static_cast<ERealSenseDistortionModels>(intrinsics_rs2.model);
                        mCameraIntrinsics.emplace(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH, intrinsics);

                        mDepthScale = mImplementation->mPipe.get_active_profile()
                                .get_device().first<rs2::depth_sensor>()
                                .get_depth_scale();
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
            mCaptureTask = std::async(std::launch::async, [this] { process(); });

            return true;
        }

        errorState.fail("RealSenseDevice already started.");

        return false;
    }


    float RealSenseDevice::getDepthScale() const
    {
        return mDepthScale;
    }


    void RealSenseDevice::stop()
    {
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            mImplementation->mPipe.stop();
        }
    }

    void RealSenseDevice::onDestroy()
    {
        stop();
    }


    void RealSenseDevice::addFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        assert(std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener) == mFrameSetListeners.end()); // device already exists
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
