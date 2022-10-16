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

// rs2 frameset forward declaration
namespace rs2
{
    class frameset;
}

namespace nap
{
    class RealSenseDevice;
    class RealSenseStreamDescription;
    class RealSenseFrameSetListenerComponentInstance;

    class NAPAPI RealSenseFrameSetListenerComponent : public Component
    {
    RTTI_ENABLE(Component)
    DECLARE_COMPONENT(RealSenseFrameSetListenerComponent, RealSenseFrameSetListenerComponentInstance)
    public:
        RealSenseFrameSetListenerComponent();

        virtual ~RealSenseFrameSetListenerComponent();

        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component receives frames from
    };

    class NAPAPI RealSenseFrameSetListenerComponentInstance : public ComponentInstance
    {
        friend class RealSenseDevice;
    RTTI_ENABLE(ComponentInstance)
    public:

        RealSenseFrameSetListenerComponentInstance(EntityInstance& entity, Component& resource) :
            ComponentInstance(entity, resource)									{ }

        virtual ~RealSenseFrameSetListenerComponentInstance();

        virtual bool init(utility::ErrorState& errorState) override final;

        virtual void onDestroy() override final;

        Signal<const rs2::frameset&> frameSetReceived;
    protected:
        virtual bool onInit(utility::ErrorState& errorState);

        virtual void destroy();

        void trigger(const rs2::frameset& frame);

        RealSenseDevice* mDevice;
    };
}
