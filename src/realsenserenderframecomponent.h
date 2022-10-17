#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class RealSenseRenderFrameComponentInstance;

    /**
     * RealSenseRenderFrameComponent
     */
    class NAPAPI RealSenseRenderFrameComponent : public RealSenseFrameSetListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderFrameComponent, RealSenseRenderFrameComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseRenderFrameComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseRenderFrameComponent();

        RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8; ///< Property: 'Format' render texture format
    };

    /**
     * RealSenseRenderFrameComponentInstance creates a RenderTexture2D and renders a frame into it
     */
    class NAPAPI RealSenseRenderFrameComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        /**
         * Constructor
         * @param entity
         * @param resource
         */
        RealSenseRenderFrameComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseRenderFrameComponentInstance();

        /**
         * Returns created render texture, if not initialized will return a render texture of 1x1
         * @return created render texture
         */
        RenderTexture2D& getRenderTexture() const{ return *mRenderTexture; }

        /**
         * Returns whether or not we have a render texture initialized
         * @return true when a render texture is initialized
         */
        bool isRenderTextureInitialized() const{ return mTextureInitialized; }

    protected:
        /**
         * internal initialization method called from init
         * Creates initial 1x1 render texture
         * @param errorState contains any errors
         * @return true on success
         */
        bool onInit(utility::ErrorState& errorState) override;

        /**
         * Called before deconstruction
         */
        void destroy() override;

        /**
         * Update method, uploads new render texture to GPU if necessary
         * @param deltaTime
         */
        void update(double deltaTime) override;

        /**
         * Called from RealSense processing thread
         * @param frame
         */
        void onTrigger(const rs2::frame& frame);
    private:
        std::unique_ptr<RenderTexture2D> mRenderTexture;
        RealSenseRenderFrameComponent* mResource;
        RenderTexture2D::EFormat mFormat;
        bool mTextureInitialized = false;

        struct Impl;
        std::unique_ptr<Impl> mImplementation;
    };
}
