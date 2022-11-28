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
        RTTI_PROPERTY("FramesRenderer", &nap::RealSenseRenderPointCloudComponent::mFramesRenderer, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("IntrinsicsType", &nap::RealSenseRenderPointCloudComponent::mCameraIntrinsicsStreamType, nap::rtti::EPropertyMetaData::Default)
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


    void RealSenseRenderPointCloudComponent::getDependentComponents(std::vector<rtti::TypeInfo> &components) const
    {
        components.emplace_back(RTTI_OF(RealSenseRenderFramesComponent));
    }


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
        mCameraIntrinsicsStreamType = resource->mCameraIntrinsicsStreamType;

        /**
         * Obtain the RealSenseRenderFramesComponent to validate if it renders the necessary depth and video texture
         * in the correct formats
         */
        auto* frames_renderer_component = mFramesRenderer->getComponent<RealSenseRenderFramesComponent>();
        auto& render_descriptions = frames_renderer_component->mRenderDescriptions;
        bool renders_depth_correctly = std::find_if(render_descriptions.begin(),
                                                    render_descriptions.end(),
                                                    [](const RealSenseRenderFrameDescription& description)
                                                    {
                                                        return description.mStreamType ==
                                                               ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH &&
                                                               description.mFormat == RenderTexture2D::EFormat::R16;
                                                    }) != render_descriptions.end();
        bool renders_video_correctly = std::find_if(render_descriptions.begin(),
                                                    render_descriptions.end(),
                                                    [](const RealSenseRenderFrameDescription& description)
                                                    {
                                                        return  description.mStreamType == ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR &&
                                                                description.mFormat == RenderTexture2D::EFormat::RGBA8;
                                                    }) != render_descriptions.end();
        if(!errorState.check(renders_depth_correctly && renders_video_correctly,
                            "%s doesn't render video or depth correctly. A depth stream must be rendered using format R16"
                            " and video stream must be rendered using format RGBA8", mID.c_str()))
            return false;

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
        // we need both a depth render texture and color render texture
        mReady = mFramesRenderer->isRenderTextureInitialized(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH) &&
                mFramesRenderer->isRenderTextureInitialized(ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR);
        if(!mReady)
            return;

        // obtain material instance
        auto& material_instance = getMaterialInstance();

        // set samplers
        auto* depth_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("depth_texture");
        depth_sampler->setTexture(mFramesRenderer->getRenderTexture(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH));
        auto* color_sampler = material_instance.getOrCreateSampler<Sampler2DInstance>("color_texture");
        color_sampler->setTexture(mFramesRenderer->getRenderTexture(ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR));

        // get camera intrinsics
        const auto& camera_intrinsics = mDevice->getIntrincicsMap();
        assert(camera_intrinsics.find(mCameraIntrinsicsStreamType)!=camera_intrinsics.end());
        const auto& intrinsics = camera_intrinsics.find(mCameraIntrinsicsStreamType)->second;

        // get depth scale
        float depth_scale = mDevice->getDepthScale();

        // set camera intrinsics uniforms
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

        // set ubo uniforms
        ubo = material_instance.getOrCreateUniform("UBO");
        ubo->getOrCreateUniform<UniformFloatInstance>("realsense_depth_scale")->setValue(depth_scale);
        ubo->getOrCreateUniform<UniformFloatInstance>("point_size_scale")->setValue(mPointSize);
    }
}
