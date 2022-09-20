#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    class RealSenseRenderPointCloudComponentInstance;

    class NAPAPI RealSenseRenderPointCloudComponent : public RealSenseFrameSetListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderPointCloudComponent, RealSenseRenderPointCloudComponentInstance)
    public:
    };

    class NAPAPI RealSenseRenderPointCloudComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource);

        virtual ~RealSenseRenderPointCloudComponentInstance();
    protected:
        void onTrigger(const rs2::frameset& frameset);
    private:
        struct Impl;
        std::unique_ptr<Impl> mImplementation;
    };
}