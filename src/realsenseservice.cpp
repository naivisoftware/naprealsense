/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "realsensedevice.h"

// External Includes
#include <nap/core.h>
#include <nap/logger.h>
#include <iostream>
#include <utility/stringutils.h>

// Local Includes
#include "realsenseservice.h"

// RealSense includes
#include <rs.hpp>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseService)
RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseService
    //////////////////////////////////////////////////////////////////////////

	RealSenseService::RealSenseService(ServiceConfiguration* configuration) :
		Service(configuration)
	{ }


	RealSenseService::~RealSenseService() = default;


	void RealSenseService::registerObjectCreators(rtti::Factory& factory)
	{
        factory.addObjectCreator(std::make_unique<RealSenseDeviceObjectCreator>(*this));
	}


	bool RealSenseService::init(nap::utility::ErrorState& errorState)
	{
        rs2::context ctx;
        auto list = ctx.query_devices(); // Get a snapshot of currently connected devices

        nap::Logger::info("There are %d connected RealSense devices.", list.size());
        for(size_t i = 0 ; i < list.size(); i++)
        {
            rs2::device device = list[i];

            nap::Logger::info("RealSense device %i, an %s", i,  device.get_info(RS2_CAMERA_INFO_NAME));
            nap::Logger::info("    Serial number: %s", device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
            nap::Logger::info("    Firmware version: %s", device.get_info(RS2_CAMERA_INFO_FIRMWARE_VERSION));

            mConnectedSerialNumbers.emplace_back(std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)));
        }

		return true;
	}


    bool RealSenseService::hasSerialNumber(const std::string& serialNumber) const
    {
        auto it = std::find_if(mConnectedSerialNumbers.begin(), mConnectedSerialNumbers.end(), [this, serialNumber](const std::string& other)
        {
            return other == serialNumber;
        });
        return it != mConnectedSerialNumbers.end();
    }
}
