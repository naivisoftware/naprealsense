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

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseService;
    class RealSensePipeLine;
    class RealSenseFrameListenerComponentInstance;

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

        virtual bool start(utility::ErrorState& errorState) override final;

        virtual void stop() override final;

        virtual void onDestroy() override final;

        void update(double deltaTime);

        // properties
        std::string mSerial;
        int mMaxFrameSize = 5;
        std::vector<ResourcePtr<RealSenseStreamDescription>> mStreams;

        void addFrameListener(RealSenseFrameListenerComponentInstance* frameListener);

        void removeFrameListener(RealSenseFrameListenerComponentInstance* frameListener);
    private:
        void process();

        std::future<void>		mCaptureTask;
        std::atomic_bool        mRun = { false };

        RealSenseService&       mService;
        std::unique_ptr<RealSensePipeLine> mPipeLine;

        std::unordered_map<ERealSenseStreamType, std::vector<RealSenseFrameListenerComponentInstance*>> mFrameListeners;
    };

    using RealSenseDeviceObjectCreator = rtti::ObjectCreator<RealSenseDevice, RealSenseService>;
}
