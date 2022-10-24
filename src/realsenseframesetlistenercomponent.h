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
    class frame;
}

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class RealSenseDevice;
    class RealSenseStreamDescription;
    class RealSenseFrameSetListenerComponentInstance;
    class RealSenseFrameFilter;

    /**
     * RealSenseFrameSetListenerComponent component
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

        ResourcePtr<RealSenseDevice> mDevice; ///< Property: 'Device' the device this component receives framesets from
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR; ///< Property: 'Stream' the stream type we're interested in
    };

    /**
     * RealSenseFrameSetListenerComponentInstance listens to frames and applies any necessary processing/filtering
     * Calls frameReceived signal with filtered frame
     */
    class NAPAPI RealSenseFrameSetListenerComponentInstance : public ComponentInstance
    {
        friend class RealSenseDevice;
    RTTI_ENABLE(ComponentInstance)
    public:
        /**
         * Constructor
         * @param entity
         * @param resource
         */
        RealSenseFrameSetListenerComponentInstance(EntityInstance& entity, Component& resource) ;

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetListenerComponentInstance();

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        virtual bool init(utility::ErrorState& errorState) override final;

        /**
         * Called before deconstruction
         */
        virtual void onDestroy() override final;
    protected:
        /**
         * internal initialization method called from init
         * @param errorState contains any errors
         * @return true on success
         */
        virtual bool onInit(utility::ErrorState& errorState);

        /**
         * Called before deconstruction
         */
        virtual void destroy();

        /**
         * Called from RealSense device upon receiving a new frameset, called from RealSense processing thread
         */
        virtual void trigger(const rs2::frameset& frameset) = 0;

        RealSenseDevice* mDevice;
        ERealSenseStreamType mStreamType;
    };
}
