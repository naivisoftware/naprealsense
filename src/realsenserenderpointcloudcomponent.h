#pragma once

#include <mesh.h>
#include <rect.h>
#include <renderablemeshcomponent.h>

#include "realsenserenderframescomponent.h"
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
     * The RealSenseRenderPointCloudComponent takes the video frame and depth frame from a RealSenseRenderFramesComponent
     * and displaces the points in the mesh using a displacement shader
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

        /**
         * Populates a list of components this component depends on.
         * Depends on RealSenseRenderFramesComponent
         * @param components list of component types this resource depends on.
         */
        void getDependentComponents(std::vector<rtti::TypeInfo> &components) const override;

        ERealSenseStreamType mCameraIntrinsicsStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH; ///< Property: 'IntrinsicsType' the camera intrinsics to use in vertex displacement shader
        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component renders the point cloud from
        ComponentPtr<RealSenseRenderFramesComponent> mFramesRenderer; ///< Property: 'FramesRenderer' the frames renderer
        float mPointSize = 1.0f; ///< Property: 'PointSize' point cloud point size
    };

    class NAPAPI RealSenseRenderPointCloudComponentInstance : public RenderableMeshComponentInstance
    {
    RTTI_ENABLE(RenderableMeshComponentInstance)
    public:
        /**
         * Constructor
         * @param entity
         * @param resource
         */
        RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource);

        /**
         * Destructor
         */
        virtual ~RealSenseRenderPointCloudComponentInstance();

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState);

        /**
         * Update this component, sets shader uniforms
         * @param deltaTime the time in between cooks in seconds
         */
        virtual void update(double deltaTime);

        /**
        * Called by the render service.
        * Renders pointcloud
        * @param renderTarget currently bound render target
        * @param commandBuffer active command buffer
        * @param viewMatrix the camera world space location
        * @param projectionMatrix the camera projection matrix
        */
        void onDraw(nap::IRenderTarget &renderTarget, VkCommandBuffer commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) override;
    private:
        ComponentInstancePtr<RealSenseRenderFramesComponent> mFramesRenderer = { this, &RealSenseRenderPointCloudComponent::mFramesRenderer };
        RealSenseDevice* mDevice;
        float mPointSize;
        bool mReady = false;
        ERealSenseStreamType mCameraIntrinsicsStreamType;
    };
}
