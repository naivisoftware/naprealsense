#include "realsenserenderframecomponent.h"
#include "realsensedevice.h"
#include "realsenseframefilter.h"

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS(nap::RealSenseRenderFrameComponent)
    RTTI_PROPERTY("RenderTexture2D", &nap::RealSenseRenderFrameComponent::mRenderTexture, nap::rtti::EPropertyMetaData::Required)
    RTTI_PROPERTY("StreamType", &nap::RealSenseRenderFrameComponent::mStreamType, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Filters", &nap::RealSenseRenderFrameComponent::mFilters, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderFrameComponentInstance)
        RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    struct RealSenseRenderFrameComponentInstance::Impl
    {
    public:
        // Frame queue
        rs2::frame_queue mFrameQueue;
    };

    bool RealSenseRenderFrameComponentInstance::onInit(utility::ErrorState &errorState)
    {
        mImplementation = std::make_unique<Impl>();

        auto *resource = getComponent<RealSenseRenderFrameComponent>();

        mRenderTexture = resource->mRenderTexture.get();
        mStreamType = resource->mStreamType;
        for(auto& filter : resource->mFilters)
        {
            mFilters.emplace_back(filter.get());
        }

        frameSetReceived.connect([this](const rs2::frameset& frameset){ onTrigger(frameset); });

        return true;
    }


    void RealSenseRenderFrameComponentInstance::destroy()
    {

    }


    void RealSenseRenderFrameComponentInstance::update(double deltaTime)
    {
        rs2::frame frame;
        if(mImplementation->mFrameQueue.poll_for_frame(&frame))
        {
            assert(frame.is<rs2::video_frame>());

            const auto &video_frame = frame.as<rs2::video_frame>();

            // Ensure dimensions are the same
            glm::vec2 tex_size = mRenderTexture->getSize();

            if(video_frame.get_width() != tex_size.x || video_frame.get_height() != tex_size.y)
            {
                nap::Logger::warn("%s: invalid size, got %d:%d, expect: %d:%d", mID.c_str(),
                                  tex_size.x,
                                  tex_size.y,
                                  video_frame.get_width(),
                                  video_frame.get_height());
                return;
            }

            // Update texture on GPU
            mRenderTexture->update(video_frame.get_data(), mRenderTexture->getDescriptor());
        }
    }


    void RealSenseRenderFrameComponentInstance::onTrigger(const rs2::frameset &frameset)
    {
        for(const auto& frame : frameset)
        {
            if(frame.get_profile().stream_type()==static_cast<rs2_stream>(mStreamType))
            {
                rs2::frame process_frame = frame;
                for(auto* filter : mFilters)
                {
                    process_frame = filter->process(process_frame);
                }
                mImplementation->mFrameQueue.enqueue(process_frame);
            }
        }
    }
}