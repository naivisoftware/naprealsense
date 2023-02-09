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
    class RealSenseFrameSetAlignFilter;

    /**
     * RealSenseStreamDescription
     * Describes a stream that can be fetched by a RealSenseDevice
     */
    class NAPAPI RealSenseStreamDescription final : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        ERealSenseStreamFormat  mFormat     = ERealSenseStreamFormat::REALSENSE_FORMAT_RGBA8; ///< Property: 'Format' The stream format
        ERealSenseStreamType    mStream     = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR; ///< Property: 'Stream' The stream type
    };

    /**
     * RealSenseDevice
     * Interface to a RealSense camera or device. Tries to fetch a stream from the device.
     */
    class NAPAPI RealSenseDevice final : public Device
    {
    RTTI_ENABLE(Device)
    public:
        /**
         * Constructor
         * @param service reference to the RealSenseService
         */
        RealSenseDevice(RealSenseService& service);

        /**
         * Destructor
         */
        virtual ~RealSenseDevice();

        /**
         * Opens a pipe with the realsense camera and tries to open streams as set in the stream descriptions property.
         * Returns false on failure
         * @param errorState contains any errors on start
         * @return true on success
         */
        virtual bool start(utility::ErrorState& errorState) override final;

        /**
         * Closes the pipe
         */
        virtual void stop() override final;

        /**
         * Called before destruction, stops device
         */
        virtual void onDestroy() override final;

        /**
         * Adds a RealSenseFrameSetListenerComponentInstance that is interested in process framesets
         * @param frameSetListener
         */
        void addFrameSetListener(RealSenseFrameSetListenerComponentInstance *frameSetListener);

        /**
         * Removes a RealSenseFrameSetListenerComponentInstance that is interested in process framesets
         * @param frameSetListener
         */
        void removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener);

        /**
         * Returns depth scale of realsense device
         * @return depth scale of realsense device
         */
        float getDepthScale() const;

        /**
         * Returns camera intrinsics map of all available camera intrinsics
         * @return camera intrinsics map of all available camera intrinsics
         */
        const std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrincics>& getIntrincicsMap() const
        { return mCameraIntrinsics; }

        // properties
        std::string mSerial;    ///< Property: 'Serial' Serial of the device, keep empty to assign first available device
        int mMaxFrameSize = 5;  ///< Property: 'MaxFrameSize' maximum frame size of frame queue
        std::vector<ResourcePtr<RealSenseStreamDescription>> mStreams; ///< Property: 'Streams' stream descriptions of streams to fetch from device
        std::vector<ResourcePtr<RealSenseFrameSetAlignFilter>> mFilters; ///< Property: 'Filters' filters applied to frameset before frameset is signalled to any listeners
        bool mAllowFailure = false; ///< Property: 'AllowFailure' allow failure of this device on initialization
    private:
        /**
         * Threaded process function
         */
        void process();

        std::future<void>		mCaptureTask;
        std::atomic_bool        mRun = { false };
        float                   mDepthScale = 0.0f ;

        RealSenseService&       mService;

        struct Impl;
        std::unique_ptr<Impl>   mImplementation;

        std::vector<RealSenseFrameSetListenerComponentInstance*> mFrameSetListeners;
        std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrincics> mCameraIntrinsics;
    };

    using RealSenseDeviceObjectCreator = rtti::ObjectCreator<RealSenseDevice, RealSenseService>;
}
