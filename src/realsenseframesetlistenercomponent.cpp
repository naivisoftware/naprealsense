#include "realsenseframesetlistenercomponent.h"
#include "realsensedevice.h"

#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetListenerComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameSetListenerComponentInstance)
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
}
