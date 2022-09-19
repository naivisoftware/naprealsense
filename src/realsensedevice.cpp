#include "realsensedevice.h"
#include "realsenseservice.h"
#include "realsenseframelistenercomponent.h"

// RealSense includes
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseStreamDescription)
    RTTI_PROPERTY("StreamFormat", &nap::RealSenseStreamDescription::mFormat, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseDepthStream)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseColorStream)
RTTI_END_CLASS

RTTI_BEGIN_ENUM(nap::ERealSenseStreamFormat)
    RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::RGBA8, "RGBA8"),
    RTTI_ENUM_VALUE(nap::ERealSenseStreamFormat::Z16, "Z16")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::ERealSenseFrameTypes)
        RTTI_ENUM_VALUE(nap::ERealSenseFrameTypes::COLOR, "Color"),
        RTTI_ENUM_VALUE(nap::ERealSenseFrameTypes::DEPTH, "Depth")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseDevice)
    RTTI_CONSTRUCTOR(nap::RealSenseService&)
    RTTI_PROPERTY("Serial", &nap::RealSenseDevice::mSerial, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("MaxFrameSize", &nap::RealSenseDevice::mMaxFrameSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Streams", &nap::RealSenseDevice::mStreams, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

namespace nap
{

    static std::unordered_map<ERealSenseFrameTypes, rs2_stream> stream_type_to_rs2stream =
        {
            { ERealSenseFrameTypes::COLOR, RS2_STREAM_COLOR },
            { ERealSenseFrameTypes::DEPTH, RS2_STREAM_DEPTH }
        };

    static std::unordered_map<rs2_stream, ERealSenseFrameTypes> rs2stream_to_stream_type =
        {
            { RS2_STREAM_COLOR, ERealSenseFrameTypes::COLOR },
            { RS2_STREAM_DEPTH, ERealSenseFrameTypes::DEPTH }
        };


    static std::unordered_map<ERealSenseStreamFormat, rs2_format> format_type_conversion_map =
        {
            { ERealSenseStreamFormat::RGBA8, RS2_FORMAT_RGBA8 },
            { ERealSenseStreamFormat::Z16, RS2_FORMAT_Z16 }
        };

    class RealSensePipeLine
    {
    public:
        // Declare RealSense pipeline, encapsulating the actual device and sensors
        rs2::pipeline mPipe;

        // Frame queue
        rs2::frame_queue mFrameQueue;
    };

    RealSenseDevice::RealSenseDevice(RealSenseService &service) : mService(service)
    {
        mService.registerDevice(this);
    }


    bool RealSenseDevice::start(utility::ErrorState &errorState)
    {
        if(!mRun.load())
        {
            mPipeLine = std::make_unique<RealSensePipeLine>();
            mPipeLine->mFrameQueue = rs2::frame_queue(mMaxFrameSize);

            rs2::config cfg;
            for(const auto& stream_type : mStreams)
            {
                assert(stream_type_to_rs2stream.find(stream_type->getStreamType()) != stream_type_to_rs2stream.end());
                assert(format_type_conversion_map.find(stream_type->mFormat)!=format_type_conversion_map.end());
                cfg.enable_stream(stream_type_to_rs2stream[stream_type->getStreamType()], format_type_conversion_map[stream_type->mFormat]);
            }
            cfg.enable_device(mSerial);

            try
            {
                mPipeLine->mPipe.start(cfg);
            }catch (const rs2::error & e)
            {
                errorState.fail(utility::stringFormat("RealSense error calling %s(%s)\n     %s,",
                                                      e.get_failed_function().c_str(),
                                                      e.get_failed_args().c_str(),
                                                      e.what()));
                return false;
            }
            catch (const std::exception& e)
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


    void RealSenseDevice::stop()
    {
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            mPipeLine->mPipe.stop();
        }
    }

    void RealSenseDevice::onDestroy()
    {
        stop();
        mService.removeDevice(this);
    }


    void RealSenseDevice::update(double deltaTime)
    {
        rs2::frameset data;
        if(mPipeLine->mFrameQueue.poll_for_frame(&data))
        {
            if(data.size() > 0)
            {
                for(const auto& frame : data)
                {
                    auto stream_type = rs2stream_to_stream_type[frame.get_profile().stream_type()];
                    if(mFrameListeners.find(stream_type)!=mFrameListeners.end())
                    {
                        auto &listeners = mFrameListeners[stream_type];
                        for(auto *listener: listeners)
                        {
                            listener->trigger(frame);
                        }
                    }
                }
            }
        }
    }

    void RealSenseDevice::addFrameListener(RealSenseFrameListenerComponentInstance* frameListener)
    {
        if(mFrameListeners.find(frameListener->getFrameType()) == mFrameListeners.end())
        {
            mFrameListeners.emplace(frameListener->getFrameType(), std::vector<RealSenseFrameListenerComponentInstance*>());
        }

        auto& listeners = mFrameListeners[frameListener->getFrameType()];
        auto it_2 = std::find(listeners.begin(), listeners.end(), frameListener);
        assert(it_2 == listeners.end()); // device already exists
        listeners.emplace_back(frameListener);
    }


    void RealSenseDevice::removeFrameListener(RealSenseFrameListenerComponentInstance* frameListener)
    {
        assert(mFrameListeners.find(frameListener->getFrameType()) != mFrameListeners.end());
        auto& listeners = mFrameListeners[frameListener->getFrameType()];
        auto it = std::find(listeners.begin(), listeners.end(), frameListener);
        assert(it != listeners.end()); // device does not exist
        listeners.erase(it);
    }


    void RealSenseDevice::process()
    {
        while(mRun.load())
        {
            rs2::frameset data;
            if(mPipeLine->mPipe.poll_for_frames(&data))
            {
                mPipeLine->mFrameQueue.enqueue(data);
            }
        }
    }


    ERealSenseFrameTypes RealSenseColorStream::getStreamType() const
    {
        return ERealSenseFrameTypes::COLOR;
    }


    ERealSenseFrameTypes RealSenseDepthStream::getStreamType() const
    {
        return ERealSenseFrameTypes::DEPTH;
    }
}