#pragma once

#include <mesh.h>
#include <rect.h>
#include <renderablemeshcomponent.h>

#include "realsenserenderframecomponent.h"
#include "realsenseframesetlistenercomponent.h"
#include "pointcloudmesh.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseDevice;
    class RealSenseRenderPointCloudComponentInstance;
    class RenderService;

    /**
     * RealSenseRenderPointCloudComponent
     * RenderableMesh component that renders a pointcloud
     */
    class NAPAPI RealSenseRenderPointCloudComponent : public RenderableMeshComponent
    {
    RTTI_ENABLE(RenderableMeshComponent)
    DECLARE_COMPONENT(RealSenseRenderPointCloudComponent, RealSenseRenderPointCloudComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseRenderPointCloudComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseRenderPointCloudComponent();

        // Properties
        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device of which to extract the pointcloud
        ComponentPtr<RealSenseRenderFrameComponent> mDepthRenderer; ///< Property: 'DepthRenderer' the render frame component that renders the depth frame into a texture
        ComponentPtr<RealSenseRenderFrameComponent> mColorRenderer; ///< Property: 'ColorRenderer' the render frame component that renders the color frame into a texture
        float mPointSize = 1.0f; ///< Property: 'PointSize' size of the point cloud points
    };

    /**
     * RealSenseRenderPointCloudComponentInstance
     * RenderableMeshComponentInstance that renders a pointcloud
     */
    class NAPAPI RealSenseRenderPointCloudComponentInstance : public RenderableMeshComponentInstance
    {
    RTTI_ENABLE(RenderableMeshComponentInstance)
    public:
        /**
         * Constructor
         * @param entity reference to entity instance
         * @param resource reference to component
         */
        RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseRenderPointCloudComponentInstance();

        /**
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState);

        /**
         * Update this component
         * @param deltaTime the time in between cooks in seconds
         */
        virtual void update(double deltaTime);

        /**
         * Renders the pointcloud
          */
        void onDraw(nap::IRenderTarget &renderTarget, VkCommandBuffer commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) override;
    protected:
    private:
        ComponentInstancePtr<RealSenseRenderFrameComponent> mDepthRenderer = { this, &RealSenseRenderPointCloudComponent::mDepthRenderer };
        ComponentInstancePtr<RealSenseRenderFrameComponent> mColorRenderer = { this, &RealSenseRenderPointCloudComponent::mColorRenderer };
        RealSenseDevice* mDevice;
        float mPointSize;
        bool mReady = false;
    };
}
