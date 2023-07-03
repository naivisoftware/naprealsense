#pragma once

#include "realsenseframesetlistenercomponent.h"
#include "realsensefilterstackcomponent.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class RealSenseRenderFramesComponentInstance;

    struct NAPAPI RealSenseRenderFrameDescription
    {
    RTTI_ENABLE()
    public:
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR;
        RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8;
    };

    /**
     * RealSenseRenderFrameComponent
     */
    class NAPAPI RealSenseRenderFramesComponent : public RealSenseFrameSetListenerComponent
    {
        friend class RealSenseRenderFramesComponentInstance;

    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderFramesComponent, RealSenseRenderFramesComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseRenderFramesComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseRenderFramesComponent();

        /**
         * Returns component instance, nullptr if not inited
         * @return component instance, nullptr if not inited
         */
        RealSenseRenderFramesComponentInstance* getInstance();

        ComponentPtr<RealSenseFilterStackComponent> mFilterStack;
        std::vector<RealSenseRenderFrameDescription> mRenderDescriptions;
    private:
        RealSenseRenderFramesComponentInstance* mInstance;
    };

    /**
     * RealSenseRenderFrameComponentInstance creates a RenderTexture2D and renders a frame into it
     */
    class NAPAPI RealSenseRenderFramesComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        /**
         * Constructor
         * @param entity
         * @param resource
         */
        RealSenseRenderFramesComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseRenderFramesComponentInstance();

        /**
         * Returns created render texture, if not initialized will return a render texture of 1x1
         * @return created render texture
         */
        RenderTexture2D& getRenderTexture(ERealSenseStreamType streamType) const;

        /**
         * Returns whether or not we have a render texture initialized
         * @return true when a render texture is initialized
         */
        bool isRenderTextureInitialized(ERealSenseStreamType streamType) const;

        /**
         * Called from RealSense processing thread
         * @param frame
         */
        virtual void trigger(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Called from RealSense device whenever a device is stopped
         */
        virtual void clear() override;
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
    private:
        ComponentInstancePtr<RealSenseFilterStackComponent> mFilterStack = { this, &RealSenseRenderFramesComponent::mFilterStack };

        std::vector<RealSenseRenderFrameDescription> mRenderDescriptions;
        std::map<ERealSenseStreamType, std::unique_ptr<RenderTexture2D>> mRenderTextures;
        std::map<ERealSenseStreamType, bool> mInitializationMap;

        RealSenseRenderFramesComponent* mResource;

        struct Impl;
        std::unique_ptr<Impl> mImplementation;
    };
}
