#include "realsenseframelistenercomponent.h"
#include "realsensedevice.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS(nap::RealSenseFrameListenerComponent)
    RTTI_PROPERTY("RealSenseDevice", &nap::RealSenseFrameListenerComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("StreamType", &nap::RealSenseFrameListenerComponent::mStreamType, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameListenerComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    RealSenseFrameListenerComponentInstance::~RealSenseFrameListenerComponentInstance()
    {}


    bool RealSenseFrameListenerComponentInstance::init(utility::ErrorState &errorState)
    {
        auto *resource = getComponent<RealSenseFrameListenerComponent>();
        mDevice = resource->mDevice.get();
        mStreamType = resource->mStreamType;

        mDevice->addFrameListener(this);

        return onInit(errorState);
    }

    bool RealSenseFrameListenerComponentInstance::onInit(utility::ErrorState& errorState)
    {
        return true;
    }


    void RealSenseFrameListenerComponentInstance::destroy()
    {}


    void RealSenseFrameListenerComponentInstance::onDestroy()
    {
        mDevice->removeFrameListener(this);

        destroy();
    }


    void RealSenseFrameListenerComponentInstance::trigger(const rs2::frame &frame)
    {
        frameReceived.trigger(frame);
    }
}