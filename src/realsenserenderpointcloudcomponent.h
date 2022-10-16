#pragma once

#include <mesh.h>
#include <rect.h>
#include <renderablemeshcomponent.h>

#include "realsenserenderframecomponent.h"
#include "realsenseframesetlistenercomponent.h"
#include "pointcloudmesh.h"

namespace nap
{
    class RealSenseDevice;
    class RealSenseRenderPointCloudComponentInstance;
    class RenderService;

    class NAPAPI RealSenseRenderPointCloudComponent : public RenderableMeshComponent
    {
    RTTI_ENABLE(RenderableMeshComponent)
    DECLARE_COMPONENT(RealSenseRenderPointCloudComponent, RealSenseRenderPointCloudComponentInstance)
    public:
        RealSenseRenderPointCloudComponent();

        virtual ~RealSenseRenderPointCloudComponent();

        ResourcePtr<RealSenseDevice> mDevice;
        ComponentPtr<TransformComponent> mCameraTransform;
        ComponentPtr<RealSenseRenderFrameComponent> mDepthRenderer;
        ComponentPtr<RealSenseRenderFrameComponent> mColorRenderer;
        float mPointSize = 1.0f;
        float mMaxDistance = 5.0f;
    };

    class NAPAPI RealSenseRenderPointCloudComponentInstance : public RenderableMeshComponentInstance
    {
    RTTI_ENABLE(RenderableMeshComponentInstance)
    public:
        RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource);

        virtual ~RealSenseRenderPointCloudComponentInstance();

        bool init(utility::ErrorState& errorState);

        /**
         * Update this component
         * @param deltaTime the time in between cooks in seconds
         */
        virtual void update(double deltaTime);

        void onDraw(nap::IRenderTarget &renderTarget, VkCommandBuffer commandBuffer, const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix) override;
    protected:
    private:
        ComponentInstancePtr<TransformComponent> mCameraTransform = { this, &RealSenseRenderPointCloudComponent::mCameraTransform };
        ComponentInstancePtr<RealSenseRenderFrameComponent> mDepthRenderer = { this, &RealSenseRenderPointCloudComponent::mDepthRenderer };
        ComponentInstancePtr<RealSenseRenderFrameComponent> mColorRenderer = { this, &RealSenseRenderPointCloudComponent::mColorRenderer };
        RealSenseDevice* mDevice;
        float mPointSize;
        float mMaxDistance;
        bool mReady = false;
    };
}
