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
    class RealSensePipeLine;
    class RealSenseFrameSetListenerComponentInstance;
    class RealSenseFrameSetAlignFilter;

    class NAPAPI RealSenseStreamDescription final : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        ERealSenseStreamFormat  mFormat     = ERealSenseStreamFormat::REALSENSE_FORMAT_RGBA8;
        ERealSenseStreamType    mStream     = ERealSenseStreamType::REALSENSE_STREAMTYPE_COLOR;
    };

    class NAPAPI RealSenseDevice final : public Device
    {
    RTTI_ENABLE(Device)
    public:
        RealSenseDevice(RealSenseService& service);

        virtual ~RealSenseDevice();

        virtual bool start(utility::ErrorState& errorState) override final;

        virtual void stop() override final;

        virtual void onDestroy() override final;

        void update(double deltaTime);

        // properties
        std::string mSerial;
        int mMaxFrameSize = 5;
        std::vector<ResourcePtr<RealSenseStreamDescription>> mStreams;
        std::vector<ResourcePtr<RealSenseFrameSetAlignFilter>> mFilters;

        void addFrameSetListener(RealSenseFrameSetListenerComponentInstance *frameSetListener);

        void removeFrameSetListener(RealSenseFrameSetListenerComponentInstance* frameSetListener);

        const std::unordered_map<ERealSenseStreamType, RealSenseCameraIntrincics>& getIntrincicsMap() const
        { return mCameraIntrinsics; }

        float getDepthScale() const;
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
