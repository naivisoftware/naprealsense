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
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseDevice;
    class RealSenseStreamDescription;
    class RealSenseFrameSetListenerComponentInstance;

    /**
     * RealSenseFrameSetListenerComponent is the resource of RealSenseFrameSetListenerComponentInstance
     * RealSenseFrameSetListenerComponent needs a reference to a RealSenseDevice it subscribes to
     */
    class NAPAPI RealSenseFrameSetListenerComponent : public Component
    {
    RTTI_ENABLE(Component)
    DECLARE_COMPONENT(RealSenseFrameSetListenerComponent, RealSenseFrameSetListenerComponentInstance)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetListenerComponent();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetListenerComponent();

        // Properties
        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component receives frames from
    };

    /**
     * RealSenseFrameSetListenerComponentInstance subscribes to the device and receives (processed) framesets
     */
    class NAPAPI RealSenseFrameSetListenerComponentInstance : public ComponentInstance
    {
        friend class RealSenseDevice;
    RTTI_ENABLE(ComponentInstance)
    public:
        /**
         * Constructor
         * @param entity reference to entity instance
         * @param resource reference to component
         */
        RealSenseFrameSetListenerComponentInstance(EntityInstance& entity, Component& resource) :
            ComponentInstance(entity, resource)	{ }

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetListenerComponentInstance();

        /**
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        virtual bool init(utility::ErrorState& errorState) override final;

        /**
         * Called before destruction
         */
        virtual void onDestroy() override final;

        // Signal triggered on new frame from process thread of RealSenseDevice
        Signal<const rs2::frameset&> frameSetReceived;
    protected:
        /**
         * Extend this mehthod to implement custom initialization, called from init method
         * @param errorState contains any errors
         * @return true on success
         */
        virtual bool onInit(utility::ErrorState& errorState);

        /**
         * Called before destruction
         */
        virtual void destroy();

        /**
         * Called from RealSenseDevice, triggers frameSetReceived signal
         * @param frameset the frameset
         */
        void trigger(const rs2::frameset& frameset);

        // pointer to RealSenseDevice
        RealSenseDevice* mDevice;
    };
}
