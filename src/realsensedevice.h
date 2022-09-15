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

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseService;

    class NAPAPI RealSenseDevice : public Device
    {
    RTTI_ENABLE(Device)
    public:
        RealSenseDevice(RealSenseService& service);

        virtual bool start(utility::ErrorState& errorState) override final;

        virtual void stop() override final;

        virtual void onDestroy() override final;

        virtual void update(double deltaTime) = 0;
    protected:
        virtual bool onStart(utility::ErrorState& errorState) = 0;

        virtual void onStop() = 0;

        virtual void onProcess() = 0;

    private:
        void process();

        std::future<void>		mCaptureTask;
        std::mutex				mCaptureMutex;
        std::condition_variable	mCaptureCondition;
        std::atomic_bool        mRun = { false };

        RealSenseService&       mService;
    };
}
