/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "realsensedevice.h"
#include "realsensecamera.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "realsenseservice.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	RealSenseService::RealSenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{
	}


	RealSenseService::~RealSenseService()
	{ }


	void RealSenseService::registerObjectCreators(rtti::Factory& factory)
	{
        factory.addObjectCreator(std::make_unique<RealSenseCameraObjectCreator>(*this));
	}


	bool RealSenseService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void RealSenseService::update(double deltaTime)
	{
        for(auto* device : mDevices)
        {
            device->update(deltaTime);
        }
	}


	void RealSenseService::shutdown()
	{
	}


    void RealSenseService::registerDevice(nap::RealSenseDevice *device)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        assert(it == mDevices.end()); // device already exists
        mDevices.emplace_back(device);
    }


    void RealSenseService::removeDevice(nap::RealSenseDevice *device)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        assert(it != mDevices.end()); // device does not exist
        mDevices.erase(it);
    }
}
