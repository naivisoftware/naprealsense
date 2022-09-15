#include "realsensedevice.h"
#include "realsenseservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseDevice)
RTTI_END_CLASS

namespace nap
{
    RealSenseDevice::RealSenseDevice(RealSenseService &service) : mService(service)
    {
        mService.registerDevice(this);
    }


    bool RealSenseDevice::start(utility::ErrorState &errorState)
    {
        if(!mRun.load())
        {
            mRun.store(true);
            mCaptureTask = std::async(std::launch::async, std::bind(&RealSenseDevice::process, this));

            return onStart(errorState);
        }

        errorState.fail("RealSenseDevice already started.");

        return false;
    }


    void RealSenseDevice::stop()
    {
        if(mRun.load())
        {
            mRun.store(false);
            if(mCaptureTask.valid())
                mCaptureTask.wait();

            onStop();
        }
    }

    void RealSenseDevice::onDestroy()
    {
        stop();
        mService.removeDevice(this);
    }


    void RealSenseDevice::process()
    {
        while(mRun.load())
        {
            onProcess();
        }
    }
}