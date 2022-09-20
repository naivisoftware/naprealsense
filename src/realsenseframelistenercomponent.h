/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <component.h>
#include <nap/resourceptr.h>
#include <entity.h>
#include <rendertexture2d.h>
#include <nap/signalslot.h>

// Local includes
#include "realsensetypes.h"

// rs2 frame forward declaration
namespace rs2
{
    class frame;
}

namespace nap
{
    class RealSenseDevice;
    class RealSenseStreamDescription;
    class RealSenseFrameListenerComponentInstance;

    class NAPAPI RealSenseFrameListenerComponent : public Component
    {
        RTTI_ENABLE(Component)
        DECLARE_COMPONENT(RealSenseFrameListenerComponent, RealSenseFrameListenerComponentInstance)
    public:
        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component receives frames from
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR;
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

        Signal<const rs2::frame&> frameReceived;
    protected:
        virtual bool onInit(utility::ErrorState& errorState);

        virtual void destroy();

        void trigger(const rs2::frame& frame);

        ERealSenseStreamType getStreamType() const{ return mStreamType; }

        RealSenseDevice* mDevice;
        ERealSenseStreamType mStreamType;
    };
}
