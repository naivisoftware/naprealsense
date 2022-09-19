/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <component.h>
#include <nap/resourceptr.h>
#include <entity.h>
#include <rendertexture2d.h>

// Local includes
#include "realsensetypes.h"

// rs2 frame forward declare
namespace rs2
{
    class frame;
}

namespace nap
{
    class RealSenseDevice;

    class NAPAPI RealSenseFrameListenerComponent : public Component
    {
        RTTI_ENABLE(Component)
    public:
        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component receives frames from
    };

    class NAPAPI RealSenseFrameListenerComponentInstance : public ComponentInstance
    {
        friend class RealSenseDevice;
        RTTI_ENABLE(ComponentInstance)
    public:
        /**
         * @param entity the entity this component belongs to.
         * @param resource the resource this instance was created from.
         */
        RealSenseFrameListenerComponentInstance(EntityInstance& entity, Component& resource) :
            ComponentInstance(entity, resource)									{ }

        virtual ~RealSenseFrameListenerComponentInstance();

        /**
         * Initializes the capture component instance.
         */
        virtual bool init(utility::ErrorState& errorState) override final;

        virtual void onDestroy() override final;

    protected:
        virtual bool onInit(utility::ErrorState& errorState) = 0;

        virtual void destroy() = 0;

        virtual void trigger(const rs2::frame& frame) = 0;

        virtual ERealSenseFrameTypes getFrameType() const = 0;

        RealSenseDevice* mDevice;
    };

    class RealSenseRenderVideoFrameComponentInstance;

    class NAPAPI RealSenseRenderVideoFrameComponent : public RealSenseFrameListenerComponent
    {
        RTTI_ENABLE(RealSenseFrameListenerComponent)
        DECLARE_COMPONENT(RealSenseRenderVideoFrameComponent, RealSenseRenderVideoFrameComponentInstance)
    public:
        ResourcePtr<RenderTexture2D> mRenderTexture;
    };

    class NAPAPI RealSenseRenderVideoFrameComponentInstance : public RealSenseFrameListenerComponentInstance
    {
        RTTI_ENABLE(RealSenseFrameListenerComponentInstance)
    public:
        RealSenseRenderVideoFrameComponentInstance(EntityInstance& entity, Component& resource) :
            RealSenseFrameListenerComponentInstance(entity, resource)     {}

        RenderTexture2D* mRenderTexture;
    protected:
        bool onInit(utility::ErrorState& errorState) override;

        void destroy() override;

        virtual void trigger(const rs2::frame& frame) override;

        virtual ERealSenseFrameTypes getFrameType() const override;
    };

    class RealSenseRenderDepthFrameComponentInstance;

    class NAPAPI RealSenseRenderDepthFrameComponent : public RealSenseFrameListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderDepthFrameComponent, RealSenseRenderDepthFrameComponentInstance)
    public:
        ResourcePtr<RenderTexture2D> mRenderTexture;
    };

    class NAPAPI RealSenseRenderDepthFrameComponentInstance : public RealSenseFrameListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameListenerComponentInstance)
    public:
        RealSenseRenderDepthFrameComponentInstance(EntityInstance& entity, Component& resource) :
            RealSenseFrameListenerComponentInstance(entity, resource)     {}

        RenderTexture2D* mRenderTexture;
    protected:
        bool onInit(utility::ErrorState& errorState) override;

        void destroy() override;

        virtual void trigger(const rs2::frame& frame) override;

        virtual ERealSenseFrameTypes getFrameType() const override;
    };
}
