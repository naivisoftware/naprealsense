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
    RTTI_PROPERTY("FrameSetFilters", &nap::RealSenseDevice::mFrameSetFilter, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("AllowFailure", &nap::RealSenseDevice::mAllowFailure, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseStreamDescription
    //////////////////////////////////////////////////////////////////////////

    RealSenseStreamDescription::RealSenseStreamDescription(){}


    RealSenseStreamDescription::~RealSenseStreamDescription(){}


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

        rs2::config mConfig;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseDevice
    //////////////////////////////////////////////////////////////////////////


    RealSenseDevice::RealSenseDevice(RealSenseService &service) : mService(service)
    {
    }


    RealSenseDevice::~RealSenseDevice(){}


    bool RealSenseDevice::start(utility::ErrorState &errorState)
    {
        /**
         * Handle error lambda
         * Fills error state and returns false upon failure
         * If failure is allowed, still return false but only log error message
         */
        auto handle_error = [this, &errorState](bool successCondition, const std::string& errorMessage) mutable -> bool
        {
            if(!successCondition)
            {
                if(mAllowFailure)
                {
                    nap::Logger::error(*this, errorMessage);
                }else
                {
                    errorState.fail(errorMessage);
                }

                return false;
            }

            return true;
        };

        if(!mRun.load())
        {
            mImplementation = std::make_unique<Impl>();
            mImplementation->mFrameQueue = rs2::frame_queue(mMaxFrameSize);

            if(!handle_error(!mService.getConnectedSerialNumbers().empty(), "No RealSense devices connected!"))
                return mAllowFailure;

            if(!mSerial.empty())
            {
                if(!handle_error(mService.hasSerialNumber(mSerial),
                                 utility::stringFormat("Device with serial number %s is not connected", mSerial.c_str())))
                    return mAllowFailure;
            }

           if(!handle_error(mService.registerDevice(this, errorState), "Cannot register device"))
               return mAllowFailure;

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
                    return mAllowFailure;
                }

                stream_types.emplace_back(stream_type);

                rs2_stream rs2_stream_type      = static_cast<rs2_stream>(stream->mStream);
                rs2_format rs2_stream_format    = static_cast<rs2_format>(stream->mFormat);
                mImplementation->mConfig.enable_stream(rs2_stream_type, rs2_stream_format);
            }

            //
            if(!mSerial.empty())
                mImplementation->mConfig.enable_device(mSerial);

            try
            {
                mImplementation->mPipe.start(mImplementation->mConfig);

                /**
                 * Gather camera intrinsics for all streams
                 */
                for(auto& stream : mStreams)
                {
                    if(mImplementation->mPipe.get_active_profile()
                        .get_stream(static_cast<rs2_stream>(stream->mStream))
                        .is<rs2::video_stream_profile>())
                    {
                        auto intrinsics_rs2 = mImplementation->mPipe
                                .get_active_profile()
                                .get_stream(static_cast<rs2_stream>(stream->mStream))
                                .as<rs2::video_stream_profile>()
                                .get_intrinsics();
                        mCameraIntrinsics.emplace(stream->mStream, RealSenseCameraIntrinsics::fromRS2Intrinsics(intrinsics_rs2));
                    }
                }

                /**
                 * Gather device info
                 */
                mCameraInfo.mName = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_NAME));
                mCameraInfo.mSerial = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_SERIAL_NUMBER));
                mCameraInfo.mFirmware = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_FIRMWARE_VERSION));
                mCameraInfo.mProductID = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_ID));
                mCameraInfo.mProductLine = std::string(mImplementation->mPipe.get_active_profile().get_device().get_info(rs2_camera_info::RS2_CAMERA_INFO_PRODUCT_LINE));
            }catch(const rs2::error& e)
            {
                handle_error(false, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                          e.get_failed_function().c_str(),
                                                          e.get_failed_args().c_str(),
                                                          e.what()));
                return mAllowFailure;

            }
            catch(const std::exception& e)
            {
                handle_error(false, e.what());
                return mAllowFailure;
            }

            for(auto& frameset_filter : mFrameSetFilter)
            {
                frameset_filter->setDevice(this);
            }

            mRun.store(true);
            mCaptureTask = std::async(std::launch::async, std::bind(&RealSenseDevice::process, this));
            mIsConnected = true;

            return true;
        }

        handle_error(false, "RealSenseDevice already started.");

        return mAllowFailure;
    }


    float RealSenseDevice::getDepthScale() const
    {
        return mLatestDepthScale.load();
    }


    void RealSenseDevice::stop()
    {
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            try
            {
                mImplementation->mPipe.stop();
            }catch(const rs2::error& e)
            {
                nap::Logger::error(*this, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                          e.get_failed_function().c_str(),
                                                          e.get_failed_args().c_str(),
                                                          e.what()));

            }
            catch(const std::exception& e)
            {
                nap::Logger::error(*this, e.what());
            }

            mService.removeDevice(this);
        }
    }

    void RealSenseDevice::onDestroy()
    {
        stop();
    }


    void RealSenseDevice::addFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it == mFrameSetListeners.end()); // device already exists

        std::lock_guard l(mFrameSetListenerMutex);
        mFrameSetListeners.emplace_back(frameSetListener);
    }


    void RealSenseDevice::removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener)
    {
        auto it = std::find(mFrameSetListeners.begin(), mFrameSetListeners.end(), frameSetListener);
        assert(it != mFrameSetListeners.end()); // device does not exist

        std::lock_guard l(mFrameSetListenerMutex);
        mFrameSetListeners.erase(it);
    }


    void RealSenseDevice::process()
    {
        try
        {
            while(mRun.load())
            {
                // store depth scale
                mLatestDepthScale.store(mImplementation->mPipe.get_active_profile().get_device().first<rs2::depth_sensor>().get_depth_scale());

                // poll for new frameset
                rs2::frameset data;
                if(mImplementation->mPipe.poll_for_frames(&data))
                {
                    for(auto& filter : mFrameSetFilter)
                    {
                        data = filter->process(data);
                    }

                    std::lock_guard l(mFrameSetListenerMutex);
                    for(auto* frameset_listener : mFrameSetListeners)
                    {
                        frameset_listener->trigger(data);
                    }
                }
            }
        }catch(const rs2::error& e)
        {
            nap::Logger::error(*this, utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                            e.get_failed_function().c_str(),
                                                            e.get_failed_args().c_str(),
                                                            e.what()));

        }
        catch(const std::exception& e)
        {
            nap::Logger::error(*this, e.what());
        }

        mRun.store(false);
        mIsConnected = false;
    }
}
