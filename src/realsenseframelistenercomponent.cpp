#include "realsenseframelistenercomponent.h"
#include "realsensedevice.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameListenerComponent)
    RTTI_PROPERTY("RealSenseDevice", &nap::RealSenseFrameListenerComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameListenerComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseRenderVideoFrameComponent)
        RTTI_PROPERTY("RenderTexture2D", &nap::RealSenseRenderVideoFrameComponent::mRenderTexture, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderVideoFrameComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseRenderDepthFrameComponent)
        RTTI_PROPERTY("RenderTexture2D", &nap::RealSenseRenderDepthFrameComponent::mRenderTexture, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderDepthFrameComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    RealSenseFrameListenerComponentInstance::~RealSenseFrameListenerComponentInstance() {}


    bool RealSenseFrameListenerComponentInstance::init(utility::ErrorState &errorState)
    {
        auto* resource = getComponent<RealSenseFrameListenerComponent>();
        mDevice = resource->mDevice.get();

        mDevice->addFrameListener(this);

        return onInit(errorState);
    }


    void RealSenseFrameListenerComponentInstance::onDestroy()
    {
        mDevice->removeFrameListener(this);

        destroy();
    }


    bool RealSenseRenderVideoFrameComponentInstance::onInit(utility::ErrorState &errorState)
    {
        auto* resource = getComponent<RealSenseRenderVideoFrameComponent>();

        mRenderTexture = resource->mRenderTexture.get();

        return true;
    }


    void RealSenseRenderVideoFrameComponentInstance::destroy()
    {

    }


    void RealSenseRenderVideoFrameComponentInstance::trigger(const rs2::frame &frame)
    {
        assert(frame.is<rs2::video_frame>());

        const auto& video_frame = frame.as<rs2::video_frame>();

        //int x = video_frame.get_width();
        //int y = video_frame.get_height();
        //nap::Logger::info("%i %i", x, y);

        // Ensure dimensions are the same
        glm::vec2 tex_size = mRenderTexture->getSize();
        if (video_frame.get_width() != tex_size.x || video_frame.get_height() != tex_size.y)
        {
            nap::Logger::warn("%s: invalid size, got %d:%d, expect: %i:%i", mID.c_str(),
                              tex_size.x,
                              tex_size.y,
                              video_frame.get_width(),
                              video_frame.get_height());
            return;
        }

        auto format = frame.get_profile().format();
        //nap::Logger::info("jej %i %s", video_frame.get_bytes_per_pixel(), rs2_format_to_string(format));

        // Update texture on GPU
        mRenderTexture->update(video_frame.get_data(), mRenderTexture->getDescriptor());
    }


    ERealSenseFrameTypes RealSenseRenderVideoFrameComponentInstance::getFrameType() const
    {
        return ERealSenseFrameTypes::COLOR;
    }


    bool RealSenseRenderDepthFrameComponentInstance::onInit(utility::ErrorState &errorState)
    {
        auto* resource = getComponent<RealSenseRenderDepthFrameComponent>();

        mRenderTexture = resource->mRenderTexture.get();

        return true;
    }


    void RealSenseRenderDepthFrameComponentInstance::destroy()
    {

    }


    void RealSenseRenderDepthFrameComponentInstance::trigger(const rs2::frame &frame)
    {
        assert(frame.is<rs2::depth_frame>());

        const auto& depth_frame = frame.as<rs2::depth_frame>();

        //int x = depth_frame.get_width();
        //int y = depth_frame.get_height();
        //nap::Logger::info("%i %i", x, y);

        // Ensure dimensions are the same
        glm::vec2 tex_size = mRenderTexture->getSize();
        if (depth_frame.get_width() != tex_size.x || depth_frame.get_height() != tex_size.y)
        {
            nap::Logger::warn("%s: invalid size, got %d:%d, expect: %i:%i", mID.c_str(),
                              tex_size.x,
                              tex_size.y,
                              depth_frame.get_width(),
                              depth_frame.get_height());
            return;
        }

        auto format = frame.get_profile().format();
        //nap::Logger::info("jej %i %s", video_frame.get_bytes_per_pixel(), rs2_format_to_string(format));

        // Update texture on GPU
        mRenderTexture->update(depth_frame.get_data(), mRenderTexture->getDescriptor());
    }


    ERealSenseFrameTypes RealSenseRenderDepthFrameComponentInstance::getFrameType() const
    {
        return ERealSenseFrameTypes::DEPTH;
    }
}