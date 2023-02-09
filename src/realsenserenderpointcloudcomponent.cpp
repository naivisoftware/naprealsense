#include "realsenserenderpointcloudcomponent.h"
#include "realsensedevice.h"
#include "renderglobals.h"

#include <renderservice.h>
#include <meshutils.h>

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseRenderPointCloudComponent)
    RTTI_PROPERTY("Device", &nap::RealSenseRenderPointCloudComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("PointSize", &nap::RealSenseRenderPointCloudComponent::mPointSize, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("DepthRenderer", &nap::RealSenseRenderPointCloudComponent::mDepthRenderer, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("ColorRenderer", &nap::RealSenseRenderPointCloudComponent::mColorRenderer, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderPointCloudComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderPointCloudComponent
    //////////////////////////////////////////////////////////////////////////

    RealSenseRenderPointCloudComponent::RealSenseRenderPointCloudComponent() = default;


    RealSenseRenderPointCloudComponent::~RealSenseRenderPointCloudComponent() = default;

    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderPointCloudComponentInstance
    //////////////////////////////////////////////////////////////////////////

    RealSenseRenderPointCloudComponentInstance::RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource)
        : RenderableMeshComponentInstance(entity, resource)
    {
    }


    RealSenseRenderPointCloudComponentInstance::~RealSenseRenderPointCloudComponentInstance() = default;


    bool RealSenseRenderPointCloudComponentInstance::init(utility::ErrorState& errorState)
    {
        if(!RenderableMeshComponentInstance::init(errorState))
            return false;

        auto* resource = getComponent<RealSenseRenderPointCloudComponent>();
        mDevice = resource->mDevice.get();
        mPointSize = resource->mPointSize;

        return true;
    }

    void RealSenseRenderPointCloudComponentInstance::onDraw(nap::IRenderTarget& renderTarget,
                                                            VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix,
                                                            const glm::mat4& projectionMatrix)
    {
        if(mReady)
            RenderableMeshComponentInstance::onDraw(renderTarget, commandBuffer, viewMatrix, projectionMatrix);
    }


    void RealSenseRenderPointCloudComponentInstance::update(double deltaTime)
    {
        // make sure there is a depth and color texture available
        mReady = mDepthRenderer->isRenderTextureInitialized() && mColorRenderer->isRenderTextureInitialized();
        if(!mReady)
            return;

        // get material instance
        auto& material_instance = getMaterialInstance();

        // assign depth
        auto* depth_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("depth_texture");
        depth_sampler->setTexture(mDepthRenderer->getRenderTexture());

        // assign rgb
        auto* color_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("color_texture");
        color_sampler->setTexture(mColorRenderer->getRenderTexture());

        // obtain camera intrinsics from depth camera
        const auto& camera_intrinsics = mDevice->getIntrincicsMap();
        const auto& intrinsics = camera_intrinsics.find(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH)->second;

        // obtain depth scale
        float depth_scale = mDevice->getDepthScale();

        // obtain camera intrinsics UBO from point cloud shader, assign properties
        auto* ubo = material_instance.getOrCreateUniform("cam_intrinsics");
        ubo->getOrCreateUniform<UniformFloatInstance>("width")->setValue(intrinsics.mWidth);
        ubo->getOrCreateUniform<UniformFloatInstance>("height")->setValue(intrinsics.mHeight);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppx")->setValue(intrinsics.mPPX);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppy")->setValue(intrinsics.mPPY);
        ubo->getOrCreateUniform<UniformFloatInstance>("fx")->setValue(intrinsics.mFX);
        ubo->getOrCreateUniform<UniformFloatInstance>("fy")->setValue(intrinsics.mFY);
        ubo->getOrCreateUniform<UniformIntInstance>("model")->setValue(static_cast<int>(intrinsics.mModel));
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[0], 0);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[1], 1);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[2], 2);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[3], 3);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrinsics.mCoeffs[4], 4);

        // assign depth scale and point size to shader UBO
        ubo = material_instance.getOrCreateUniform("UBO");
        ubo->getOrCreateUniform<UniformFloatInstance>("realsense_depth_scale")->setValue(depth_scale);
        ubo->getOrCreateUniform<UniformFloatInstance>("point_size")->setValue(mPointSize);
    }
}
