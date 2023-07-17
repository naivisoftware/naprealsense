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

    class RealSenseDevice;
    class RealSenseRenderPointCloudComponentInstance;
    class RenderService;

    /**
     * RealSenseRenderPointCloudComponent
     */
    class NAPAPI RealSenseRenderPointCloudComponent : public RenderableMeshComponent
    {
        friend class RealSenseRenderPointCloudComponentInstance;
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

        RealSenseRenderPointCloudComponentInstance* getInstance();

        ERealSenseStreamType mCameraIntrinsicsStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH;
        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component renders the point cloud from
        ComponentPtr<TransformComponent> mCameraTransform; ///< Property: 'CameraTransform' the camera transform
        ComponentPtr<RealSenseRenderFramesComponent> mFramesRenderer; ///< Property: 'FramesRenderer'
        float mPointSize = 1.0f; ///< Property: 'PointSize'
        float mMaxDistance = 5.0f; ///< Property: 'MaxDistance'
    private:
        RealSenseRenderPointCloudComponentInstance* mInstance = nullptr;
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
         * Update this component
         * @param deltaTime the time in between cooks in seconds
         */
        virtual void update(double deltaTime);

        /**
         *
         * @param renderTarget
         * @param commandBuffer
         * @param viewMatrix
         * @param projectionMatrix
         */
        void onDraw(nap::IRenderTarget &renderTarget, VkCommandBuffer commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) override;
    protected:
    private:
        ComponentInstancePtr<TransformComponent> mCameraTransform = { this, &RealSenseRenderPointCloudComponent::mCameraTransform };
        ComponentInstancePtr<RealSenseRenderFramesComponent> mFramesRenderer = { this, &RealSenseRenderPointCloudComponent::mFramesRenderer };
        RealSenseDevice* mDevice;
        float mPointSize;
        float mMaxDistance;
        bool mReady = false;
        ERealSenseStreamType mCameraIntrinsicsStreamType;
    };
}
