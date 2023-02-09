/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <rtti/factory.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
    // forward declares
    class RealSenseDevice;

	class NAPAPI RealSenseService : public Service
	{
        friend class RealSenseDevice;

		RTTI_ENABLE(Service)
	public:

		/**
		 * Constructor
		 */
		RealSenseService(ServiceConfiguration* configuration);

		/**
		 * Deconstructor
		 */
		virtual ~RealSenseService();

		/**
		 * registers all objects that need a specific way of construction
		 * @param factory the factory to register the object creators with
		 */
		virtual void registerObjectCreators(rtti::Factory& factory) override;

		/**
		 * initializes service
		 * @param errorState contains any errors
		 * @return returns true on successful initialization
		 */
		virtual bool init(nap::utility::ErrorState& errorState) override;

        /**
         * Returns true if a device with given serial number is registered
         * @param serialNumber the serial number to check
         * @return true if a device with given serial number is registered
         */
        bool hasSerialNumber(const std::string& serialNumber) const;

        /**
         * Returns const reference to vector of all serial number of connected realsense devices
         * @return const reference to vector of all serial number of connected realsense devices
         */
        const std::vector<std::string>& getConnectedSerialNumbers() const{ return mConnectedSerialNumbers; }
	private:
        std::vector<std::string> mConnectedSerialNumbers;
	};
}
