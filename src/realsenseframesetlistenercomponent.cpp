#include "realsenseframesetlistenercomponent.h"
#include "realsensedevice.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetListenerComponent)
        RTTI_PROPERTY("RealSenseDevice", &nap::RealSenseFrameSetListenerComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameSetListenerComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    RealSenseFrameSetListenerComponentInstance::~RealSenseFrameSetListenerComponentInstance()
    {}


    bool RealSenseFrameSetListenerComponentInstance::init(utility::ErrorState &errorState)
    {
        auto *resource = getComponent<RealSenseFrameSetListenerComponent>();
        mDevice = resource->mDevice.get();

        mDevice->addFrameSetListener(this);

        return onInit(errorState);
    }

    bool RealSenseFrameSetListenerComponentInstance::onInit(utility::ErrorState& errorState)
    {
        return true;
    }


    void RealSenseFrameSetListenerComponentInstance::destroy()
    {}


    void RealSenseFrameSetListenerComponentInstance::onDestroy()
    {
        mDevice->removeFrameSetListener(this);

        destroy();
    }


    void RealSenseFrameSetListenerComponentInstance::trigger(const rs2::frameset &frameset)
    {
        frameSetReceived.trigger(frameset);
    }
}