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

    class NAPAPI RealSenseStreamDescription : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        ERealSenseStreamFormat mFormat = ERealSenseStreamFormat::RGBA8;

        virtual ERealSenseFrameTypes getStreamType() const = 0;
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
        std::condition_variable	mCaptureCondition;
        std::atomic_bool        mRun = { false };

        RealSenseService&       mService;
        std::unique_ptr<RealSensePipeLine> mPipeLine;

        std::unordered_map<ERealSenseFrameTypes, std::vector<RealSenseFrameListenerComponentInstance*>> mFrameListeners;
    };

    class NAPAPI RealSenseColorStream : public RealSenseStreamDescription
    {
    RTTI_ENABLE(RealSenseStreamDescription)
    public:
        virtual ERealSenseFrameTypes getStreamType() const override;
    };

    class NAPAPI RealSenseDepthStream : public RealSenseStreamDescription
    {
    RTTI_ENABLE(RealSenseStreamDescription)
    public:
        virtual ERealSenseFrameTypes getStreamType() const override;
    };

    using RealSenseDeviceObjectCreator = rtti::ObjectCreator<RealSenseDevice, RealSenseService>;
}
