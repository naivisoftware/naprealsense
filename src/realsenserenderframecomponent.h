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
        RealSenseRenderFrameComponent();

        virtual ~RealSenseRenderFrameComponent();

        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR;

        RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8;
        std::vector<ResourcePtr<RealSenseFrameFilter>> mFilters;

    };

    class NAPAPI RealSenseRenderFrameComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        RealSenseRenderFrameComponentInstance(EntityInstance& entity, Component& resource);

        virtual ~RealSenseRenderFrameComponentInstance();

        RenderTexture2D& getRenderTexture() const{ return *mRenderTexture; }

        bool isRenderTextureInitialized() const{ return mTextureInitialized; }

    protected:
        bool onInit(utility::ErrorState& errorState) override;

        void destroy() override;

        void update(double deltaTime);

        void onTrigger(const rs2::frameset& frame);
    private:
        std::unique_ptr<RenderTexture2D> mRenderTexture;
        RealSenseRenderFrameComponent* mResource;
        ERealSenseStreamType mStreamType;
        RenderTexture2D::EFormat mFormat;
        bool mTextureInitialized = false;

        struct Impl;
        std::unique_ptr<Impl> mImplementation;

        std::vector<RealSenseFrameFilter*> mFilters;
    };
}
