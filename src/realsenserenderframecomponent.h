#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    class RealSenseRenderFrameComponentInstance;

    class NAPAPI RealSenseRenderFrameComponent : public RealSenseFrameSetListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderFrameComponent, RealSenseRenderFrameComponentInstance)
    public:
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR;
        ResourcePtr<RenderTexture2D> mRenderTexture;
    };

    class NAPAPI RealSenseRenderFrameComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        RealSenseRenderFrameComponentInstance(EntityInstance& entity, Component& resource) :
            RealSenseFrameSetListenerComponentInstance(entity, resource)     {}
    protected:
        bool onInit(utility::ErrorState& errorState) override;

        void destroy() override;

        void onTrigger(const rs2::frameset& frame);
    private:
        RenderTexture2D* mRenderTexture;
        ERealSenseStreamType mStreamType;
    };
}