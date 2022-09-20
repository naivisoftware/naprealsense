#pragma once

#include "realsenseframelistenercomponent.h"

namespace nap
{
    class RealSenseRenderFrameComponentInstance;

    class NAPAPI RealSenseRenderFrameComponent : public RealSenseFrameListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderFrameComponent, RealSenseRenderFrameComponentInstance)
    public:
        ResourcePtr<RenderTexture2D> mRenderTexture;
    };

    class NAPAPI RealSenseRenderFrameComponentInstance : public RealSenseFrameListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameListenerComponentInstance)
    public:
        RealSenseRenderFrameComponentInstance(EntityInstance& entity, Component& resource) :
            RealSenseFrameListenerComponentInstance(entity, resource)     {}

        RenderTexture2D* mRenderTexture;
    protected:
        bool onInit(utility::ErrorState& errorState) override;

        void destroy() override;

        void onTrigger(const rs2::frame& frame);
    };
}