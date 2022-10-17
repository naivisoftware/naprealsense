#include "realsenserenderframecomponent.h"
#include "realsensedevice.h"

#include <rs.hpp>

RTTI_BEGIN_CLASS(nap::RealSenseRenderFrameComponent)
    RTTI_PROPERTY("Format", &nap::RealSenseRenderFrameComponent::mFormat, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("StreamType", &nap::RealSenseRenderFrameComponent::mStreamType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderFrameComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderFrameComponent
    //////////////////////////////////////////////////////////////////////////


    RealSenseRenderFrameComponent::RealSenseRenderFrameComponent(){}


    RealSenseRenderFrameComponent::~RealSenseRenderFrameComponent(){}


    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderFrameComponentInstance::Impl
    //////////////////////////////////////////////////////////////////////////


    struct RealSenseRenderFrameComponentInstance::Impl
    {
    public:
        // Frame queue
        rs2::frame_queue mFrameQueue;
    };


    //////////////////////////////////////////////////////////////////////////
    // RealSenseRenderFrameComponentInstance
    //////////////////////////////////////////////////////////////////////////


    RealSenseRenderFrameComponentInstance::RealSenseRenderFrameComponentInstance(EntityInstance& entity, Component& resource) :
        RealSenseFrameSetListenerComponentInstance(entity, resource)
    {

    }


    RealSenseRenderFrameComponentInstance::~RealSenseRenderFrameComponentInstance(){}


    bool RealSenseRenderFrameComponentInstance::onInit(utility::ErrorState &errorState)
    {
        mImplementation = std::make_unique<Impl>();

        mResource = getComponent<RealSenseRenderFrameComponent>();
        mFormat = mResource->mFormat;
        mStreamType = mResource->mStreamType;

        mRenderTexture = std::make_unique<RenderTexture2D>(*getEntityInstance()->getCore());
        mRenderTexture->mWidth = 0;
        mRenderTexture->mHeight = 0;
        mRenderTexture->mClearColor = { 0, 0, 0 , 0 };
        mRenderTexture->mColorSpace = EColorSpace::Linear;
        mRenderTexture->mFormat = mFormat;

        frameReceived.connect([this](const rs2::frame& frame){ onTrigger(frame); });

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
                mRenderTexture->mWidth = video_frame.get_width();
                mRenderTexture->mHeight = video_frame.get_height();
                mRenderTexture->mClearColor = { 0, 0, 0 , 0 };
                mRenderTexture->mColorSpace = EColorSpace::Linear;
                mRenderTexture->mFormat = mFormat;
                mRenderTexture->mUsage = ETextureUsage::DynamicWrite;

                utility::ErrorState error_state;
                mTextureInitialized = mRenderTexture->init(error_state);
            }

            if(mTextureInitialized)
            {
                // Update texture on GPU
                mRenderTexture->update(video_frame.get_data(), mRenderTexture->getDescriptor());
            }
        }
    }


    void RealSenseRenderFrameComponentInstance::onTrigger(const rs2::frame &frame)
    {
        mImplementation->mFrameQueue.enqueue(frame);
    }
}