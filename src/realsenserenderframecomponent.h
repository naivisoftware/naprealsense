#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    class RealSenseRenderFrameComponentInstance;
    class RealSenseFrameFilter;

    class NAPAPI RealSenseRenderFrameComponent : public RealSenseFrameSetListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderFrameComponent, RealSenseRenderFrameComponentInstance)
    public:
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR;
        ResourcePtr<RenderTexture2D> mRenderTexture;
        std::vector<ResourcePtr<RealSenseFrameFilter>> mFilters;
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

        void update(double deltaTime);

        void onTrigger(const rs2::frameset& frame);
    private:
        RenderTexture2D* mRenderTexture;
        ERealSenseStreamType mStreamType;

        struct Impl;
        std::unique_ptr<Impl> mImplementation;

        std::vector<RealSenseFrameFilter*> mFilters;
    };
}