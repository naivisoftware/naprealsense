/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/device.h>
#include <nap/resourceptr.h>
#include <nap/signalslot.h>
#include <rtti/factory.h>
#include <thread>
#include <future>
#include <atomic>

// Local includes
#include "realsensetypes.h"
#include "realsenseframesetlistenercomponent.h"


namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseService;
    class RealSenseFrameSetListenerComponentInstance;
    class RealSenseFrameSetFilter;

    /**
     * Describes a RealSenseStream provided by the device/camera
     */
    class NAPAPI RealSenseStreamDescription final : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        /**
         * Constructor
         */
        RealSenseStreamDescription();

        /**
         * Destructor
         */
        virtual ~RealSenseStreamDescription();

        ERealSenseStreamFormat  mFormat     = ERealSenseStreamFormat::REALSENSE_FORMAT_RGBA8; ///< Property: 'Format' stream format
        ERealSenseStreamType    mStream     = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR; ///< Property: 'Stream' stream type
    };

    /**
     * Interface for a RealSense device/camera
     */
    class NAPAPI RealSenseDevice final : public Device
    {
    RTTI_ENABLE(Device)
    public:
        /**
         * Constructor
         * @param service
         */
        RealSenseDevice(RealSenseService& service);

        /**
         * Destructor
         */
        virtual ~RealSenseDevice();

        /**
         * Called when device needs to start
         * @param errorState contains any errors
         * @return true on success
         */
        virtual bool start(utility::ErrorState& errorState) override final;

        /**
         * Called when device needs to stop
         */
        virtual void stop() override final;

        /**
         * Called before deconstruction
         */
        virtual void onDestroy() override final;

        /**
         * Adds a frameset listener
         * @param frameSetListener
         */
        void addFrameSetListener(RealSenseFrameSetListenerComponentInstance *frameSetListener);

        /**
         * Removes a frameset listener
         * @param frameSetListener
         */
        void removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener);

        /**
         * Returns map of camera intrinsics of every available stream
         * @return map of camera intrinsics of every available stream
         */
        const std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrincics>& getIntrincicsMap() const{ return mCameraIntrinsics; }

        /**
         * Returns current depth scale in meters per unit
         * @return current depth scale in meters per unit
         */
        float getDepthScale() const;

        // properties
        std::string mSerial; ///< Property: 'Serial' serial of device to use, keep empty to get first device available
        int mMaxFrameSize = 5; ///< Property: 'MaxFrameSize' maximum size of frame queue
        std::vector<ResourcePtr<RealSenseStreamDescription>> mStreams; ///< Property: 'Streams' settings for desired streams
        std::vector<ResourcePtr<RealSenseFrameSetFilter>> mFrameSetFilter; ///< Property: 'FrameSetFilters' filter that need to be applied to new framesets acquired by the device
        bool mAllowFailure = false; ///< Property: 'AllowFailure' return init success upon init failure
    private:
        void process();

        std::future<void>		mCaptureTask;
        std::atomic_bool        mRun = { false };

        RealSenseService&       mService;

        struct Impl;
        std::unique_ptr<Impl>   mImplementation;

        std::vector<RealSenseFrameSetListenerComponentInstance*> mFrameSetListeners;
        std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrincics> mCameraIntrinsics;
    };

    using RealSenseDeviceObjectCreator = rtti::ObjectCreator<RealSenseDevice, RealSenseService>;
}
