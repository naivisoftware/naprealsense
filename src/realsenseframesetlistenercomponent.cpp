#include "realsenseframesetlistenercomponent.h"
#include "realsensedevice.h"
#include "realsenseframefilter.h"

#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetListenerComponent)
    RTTI_PROPERTY("RealSenseDevice", &nap::RealSenseFrameSetListenerComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("Filters", &nap::RealSenseFrameSetListenerComponent::mFilters, nap::rtti::EPropertyMetaData::Embedded)
    RTTI_PROPERTY("StreamType", &nap::RealSenseFrameSetListenerComponent::mStreamType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameSetListenerComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetListenerComponent
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameSetListenerComponent::RealSenseFrameSetListenerComponent(){}


    RealSenseFrameSetListenerComponent::~RealSenseFrameSetListenerComponent(){}


    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetListenerComponentInstance
    //////////////////////////////////////////////////////////////////////////


    RealSenseFrameSetListenerComponentInstance::~RealSenseFrameSetListenerComponentInstance()
    {}

    RealSenseFrameSetListenerComponentInstance::RealSenseFrameSetListenerComponentInstance(EntityInstance& entity, Component& resource) :
        ComponentInstance(entity, resource)			{ }


    bool RealSenseFrameSetListenerComponentInstance::init(utility::ErrorState &errorState)
    {
        auto *resource = getComponent<RealSenseFrameSetListenerComponent>();
        mDevice = resource->mDevice.get();
        mDevice->addFrameSetListener(this);
        mStreamType = resource->mStreamType;
        for(auto& filter : resource->mFilters)
        {
            mFilters.emplace_back(filter.get());
        }

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
        for(const auto& frame : frameset)
        {
            if(frame.get_profile().stream_type()==static_cast<rs2_stream>(mStreamType))
            {
                rs2::frame process_frame = frame;
                for(auto* filter : mFilters)
                {
                    process_frame = filter->process(process_frame);
                }
                frameReceived.trigger(process_frame);
            }
        }
    }
}
