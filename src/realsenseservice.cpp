/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

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
	}


	bool RealSenseService::init(nap::utility::ErrorState& errorState)
	{
		return true;
	}


	void RealSenseService::update(double deltaTime)
	{
	}


	void RealSenseService::shutdown()
	{
	}
}
