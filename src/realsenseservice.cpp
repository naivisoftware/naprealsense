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
	{

	}


	RealSenseService::~RealSenseService()
	{ }


	void RealSenseService::registerObjectCreators(rtti::Factory& factory)
	{
        factory.addObjectCreator(std::make_unique<RealSenseDeviceObjectCreator>(*this));
	}


	bool RealSenseService::init(nap::utility::ErrorState& errorState)
	{
        try
        {
            std::vector<std::string> serials;
            rs2::context ctx;
            auto list = ctx.query_devices(); // Get a snapshot of currently connected devices

            nap::Logger::info("There are %d connected RealSense devices.", list.size());
            for(size_t i = 0 ; i < list.size(); i++)
            {
                rs2::device device = list[i];

                nap::Logger::info("RealSense device %i, an %s", i,  device.get_info(RS2_CAMERA_INFO_NAME));
                nap::Logger::info("    Serial number: %s", device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
                nap::Logger::info("    Firmware version: %s", device.get_info(RS2_CAMERA_INFO_FIRMWARE_VERSION));

                serials.emplace_back(std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER)));
            }
            setSerialNumbers(serials);
        }catch(const rs2::error& e)
        {
            nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s(%s)\n      %s",
                                           e.get_failed_function().c_str(),
                                           e.get_failed_args().c_str(),
                                           e.what()));
        }
        catch(const std::exception& e)
        {
            nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s",
                                                     e.what()));
        }

        mRun.store(true);
        mQueryDevicesTask = std::async(std::launch::async, std::bind(&RealSenseService::queryDeviceTask, this));

		return true;
	}


	void RealSenseService::update(double deltaTime)
	{
        // Check if a device needs to stop because it is no longer available
        std::string device_to_stop;
        while(mDevicesToStop.try_dequeue(device_to_stop))
        {
            for(const auto& device : mDevices)
            {
                if(device->getIsConnected() && device->getCameraInfo().mSerial == device_to_stop)
                {
                    device->stop();
                }
            }
        }

        // Check if a realsense camera is (re)added, if so, find a matching RealSenseDevice
        std::string device_to_restart;
        while(mDevicesToRestart.try_dequeue(device_to_restart))
        {
            // Iterate trough registered devices
            for(const auto& device : mDevices)
            {
                if(!device->getIsConnected() && // device is disconnected
                    (device->getCameraInfo().mSerial == device_to_restart || // and serial matches the device to restart
                    device->mSerial.empty())) // OR serial is empty, in which case it maybe can make a claim on this device
                {
                    // First check if any other devices make a claim on this device
                    bool skip = false;
                    for(auto& other_device : mDevices)
                    {
                        if(other_device!=device)
                        {
                            if(other_device->mSerial == device_to_restart || // another device has a claim
                                                                             // OR, other device is disconnected and was previously connected to this serial
                                (other_device->getCameraInfo().mSerial == device_to_restart && !other_device->getIsConnected()))
                            {
                                skip = true;
                                break;
                            }
                        }
                    }
                    if(skip)
                        continue;

                    // Restart this device
                    nap::Logger::info("Restarting device : %s", device_to_restart.c_str());
                    utility::ErrorState error_state;
                    if(!device->restart(error_state))
                    {
                        nap::Logger::error("Error restarting device %s : %s", device_to_restart.c_str(), error_state.toString().c_str());
                    }
                }
            }
        }
	}


    void RealSenseService::acquireSerialNumbers(std::vector<std::string> &serials)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        for(const auto& serial : mConnectedSerialNumbers)
        {
            serials.emplace_back(serial);
        }
    }


    void RealSenseService::setSerialNumbers(const std::vector<std::string> &serials)
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mConnectedSerialNumbers.clear();
        for(const auto& serial : serials)
        {
            mConnectedSerialNumbers.emplace_back(serial);
        }
    }


    void RealSenseService::queryDeviceTask()
    {
        while(mRun.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(mQueryIntervalSeconds * 1000)));

            try
            {
                std::vector<std::string> serials;
                acquireSerialNumbers(serials);

                rs2::context ctx;
                auto list = ctx.query_devices(); // Get a snapshot of currently connected devices

                std::vector<std::string> found_devices;
                std::vector<std::string> devices_to_restart;
                std::vector<std::string> devices_to_stop;
                for(size_t i = 0 ; i < list.size(); i++)
                {
                    rs2::device device = list[i];
                    auto serial = std::string(device.get_info(RS2_CAMERA_INFO_SERIAL_NUMBER));
                    found_devices.emplace_back(serial);
                    auto it = std::find_if(serials.begin(), serials.end(), [serial](const std::string& other)
                    {
                        return other == serial;
                    });
                    if(it == serials.end())
                    {
                        nap::Logger::info(utility::stringFormat("new device %s", serial.c_str()));
                        serials.emplace_back(serial);

                        devices_to_restart.emplace_back(serial);
                    }
                }

                auto c_it = serials.begin();
                while(c_it != serials.end())
                {
                    auto serial = *(c_it);
                    auto found = std::find_if(found_devices.begin(), found_devices.end(), [serial](const std::string& other)
                    {
                        return other == serial;
                    }) != found_devices.end();
                    if(!found)
                    {
                        nap::Logger::info(utility::stringFormat("RealSense Camera disconnected %s", serial.c_str()));
                        devices_to_stop.emplace_back(serial);
                        c_it = serials.erase(c_it);
                    }else
                    {
                        c_it++;
                    }
                }

                setSerialNumbers(serials);

                for(const auto& device_to_stop : devices_to_stop)
                    mDevicesToStop.enqueue(device_to_stop);

                for(const auto& device_to_restart : devices_to_restart)
                    mDevicesToRestart.enqueue(device_to_restart);
            }catch(const rs2::error& e)
            {
                nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s(%s)\n      %s",
                                                         e.get_failed_function().c_str(),
                                                         e.get_failed_args().c_str(),
                                                         e.what()));
            }
            catch(const std::exception& e)
            {
                nap::Logger::error(utility::stringFormat("Error query RealSense devices : %s",
                                                         e.what()));
            }
        }
    }


	void RealSenseService::shutdown()
	{
        mRun.store(false);
        if(mQueryDevicesTask.valid())
            mQueryDevicesTask.wait();
	}


    bool RealSenseService::registerDevice(nap::RealSenseDevice *device, utility::ErrorState& errorState)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        if(it != mDevices.end())
        {
            errorState.fail("Device already registered");
            return false;
        }
        for(auto* other : mDevices)
        {
            if(!other->mSerial.empty() && !device->mSerial.empty())
            {
                if(other->mSerial==device->mSerial)
                {
                    errorState.fail(utility::stringFormat("Device with serial %s already registered", device->mSerial.c_str()));
                    return false;
                }
            }
        }

        mDevices.emplace_back(device);

        return true;
    }


    bool RealSenseService::hasSerialNumber(const std::string& serialNumber)
    {
        std::vector<std::string> serials;
        acquireSerialNumbers(serials);

        auto it = std::find_if(serials.begin(), serials.end(), [this, serialNumber](const std::string& other)
        {
            return other == serialNumber;
        });
        return it != serials.end();
    }


    void RealSenseService::removeDevice(nap::RealSenseDevice *device)
    {
        auto it = std::find(mDevices.begin(), mDevices.end(), device);
        assert(it != mDevices.end()); // device does not exist
        mDevices.erase(it);
    }
}
