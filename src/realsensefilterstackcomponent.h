#pragma once

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    class RealSenseFrameSetFilter;
    class RealSenseFilterStackComponentInstance;
    class RealSenseDevice;

    /**
     * RealSenseRenderFrameComponent
     */
    class NAPAPI RealSenseFilterStackComponent : public RealSenseFrameSetListenerComponent
    {
        friend class RealSenseFilterStackComponentInstance;

    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseFilterStackComponent, RealSenseFilterStackComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseFilterStackComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseFilterStackComponent();

        /**
         * Returns component instance, nullptr if not inited
         * @return component instance, nullptr if not inited
         */
        RealSenseFilterStackComponentInstance* getInstance();

        std::vector<ResourcePtr<RealSenseFrameSetFilter>> mFilters;
        ResourcePtr<RealSenseDevice> mDevice;
    private:
        RealSenseFilterStackComponentInstance* mInstance;
    };

    /**
     * RealSenseRenderFrameComponentInstance creates a RenderTexture2D and renders a frame into it
     */
    class NAPAPI RealSenseFilterStackComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        /**
         * Constructor
         * @param entity
         * @param resource
         */
        RealSenseFilterStackComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseFilterStackComponentInstance();

        void addFrameSetListener(RealSenseFrameSetListenerComponentInstance* framesetListener);

        void removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* framesetListener);

        /**
         * Called from RealSense processing thread
         * @param frame
         */
        virtual void trigger(RealSenseDevice* device, const rs2::frameset& frameset) override;
    protected:
        /**
         * initialization method
         * Creates initial 1x1 render texture
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) final;

        /**
         * Called before deconstruction
         */
        void onDestroy() final;
    private:
        std::vector<RealSenseFrameSetListenerComponentInstance*> mFrameSetListeners;
        RealSenseFilterStackComponent* mResource;
        std::vector<RealSenseFrameSetFilter*> mFilters;
        std::mutex mMutex;
        RealSenseDevice* mDevice;
    };
}
