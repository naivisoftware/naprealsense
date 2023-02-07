#include "realsenserenderpointcloudcomponent.h"
#include "realsensedevice.h"
#include "renderglobals.h"

#include <renderservice.h>
#include <meshutils.h>

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseRenderPointCloudComponent)
    RTTI_PROPERTY("Device", &nap::RealSenseRenderPointCloudComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("CameraTransform", &nap::RealSenseRenderPointCloudComponent::mCameraTransform, nap::rtti::EPropertyMetaData::Required)
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

    RealSenseRenderPointCloudComponent::RealSenseRenderPointCloudComponent(){}


    RealSenseRenderPointCloudComponent::~RealSenseRenderPointCloudComponent(){}

    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderPointCloudComponentInstance
    //////////////////////////////////////////////////////////////////////////

    RealSenseRenderPointCloudComponentInstance::RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource)
        : RenderableMeshComponentInstance(entity, resource)
    {
    }


    RealSenseRenderPointCloudComponentInstance::~RealSenseRenderPointCloudComponentInstance()
    {

    }


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
        mReady = mDepthRenderer->isRenderTextureInitialized() && mColorRenderer->isRenderTextureInitialized();
        if(!mReady)
            return;

        auto& material_instance = getMaterialInstance();
        auto* depth_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("depth_texture");
        depth_sampler->setTexture(mDepthRenderer->getRenderTexture());

        auto* color_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("color_texture");
        color_sampler->setTexture(mColorRenderer->getRenderTexture());

        const auto& camera_intrincics = mDevice->getIntrincicsMap();
        const auto& intrincics = camera_intrincics.find(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH)->second;

        auto* ubo = material_instance.getOrCreateUniform("cam_intrinsics");
        float depth_scale = mDevice->getDepthScale();
        ubo->getOrCreateUniform<UniformFloatInstance>("width")->setValue(intrincics.mWidth);
        ubo->getOrCreateUniform<UniformFloatInstance>("height")->setValue(intrincics.mHeight);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppx")->setValue(intrincics.mPPX);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppy")->setValue(intrincics.mPPY);
        ubo->getOrCreateUniform<UniformFloatInstance>("fx")->setValue(intrincics.mFX);
        ubo->getOrCreateUniform<UniformFloatInstance>("fy")->setValue(intrincics.mFY);
        ubo->getOrCreateUniform<UniformIntInstance>("model")->setValue(static_cast<int>(intrincics.mModel));
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[0], 0);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[1], 1);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[2], 2);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[3], 3);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[4], 4);

        ubo = material_instance.getOrCreateUniform("UBO");
        ubo->getOrCreateUniform<UniformVec3Instance>("camera_world_position")->setValue(math::extractPosition(mCameraTransform->getGlobalTransform()));
        ubo->getOrCreateUniform<UniformFloatInstance>("realsense_depth_scale")->setValue(depth_scale);
        ubo->getOrCreateUniform<UniformFloatInstance>("point_size_scale")->setValue(mPointSize);
        ubo->getOrCreateUniform<UniformFloatInstance>("max_distance")->setValue(mMaxDistance);
    }
}
