/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/service.h>
#include <rtti/factory.h>
#include <nap/timer.h>
#include <concurrentqueue.h>
#include <future>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseDevice;

    /**
     * RealSenseService
     */
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
		 * updates any tweens
		 * @param deltaTime deltaTime
		 */
		virtual void update(double deltaTime) override;

		/**
		 * called when service is shutdown, deletes all tweens
		 */
		void shutdown() override;

        /**
         * Check whether device with serial number is present
         * @param serialNumber
         * @return true if device is present
         */
        bool hasSerialNumber(const std::string& serialNumber);

        /**
         * Returns vector of all serial numbers connected
         * @return vector of all serial numbers connected
         */
        const std::vector<std::string>& getConnectedSerialNumbers() const{ return mConnectedSerialNumbers; }
	private:
        /**
         * Registers a RealSenseDevice
         * @param device pointer to RealSenseDevice
         * @param errorState contains any errors
         * @return true on success
         */
        bool registerDevice(RealSenseDevice* device, utility::ErrorState& errorState);

        /**
         * Removes a RealSenseDevice
         * @param device pointer to RealSenseDevice
         */
        void removeDevice(RealSenseDevice* device);

        /**
         * This task takes a snapshot of connected realsense devices on a different thread
         * When a device has been added or removed, the correct stop/start calls to registered devices will occur on
         * the main thread
         */
        void queryDeviceTask();

        /**
         * Acquires current connected serial numbers, thread-safe
         * @param serials
         */
        void acquireSerialNumbers(std::vector<std::string>& serials);

        /**
         * Sets current connected serial numbers, thread-safe
         * @param serials
         */
        void setSerialNumbers(const std::vector<std::string>& serials);

        // The query task
        std::future<void>		mQueryDevicesTask;
        std::atomic_bool        mRun = { false };

        // interval of query task
        const float mQueryIntervalSeconds = 1.0f;

        // these concurrent queues hold serials of devices to (re)start or stop
        moodycamel::ConcurrentQueue<std::string> mDevicesToRestart;
        moodycamel::ConcurrentQueue<std::string> mDevicesToStop;

        // conencted serial numbers with mutex
        std::mutex mMutex;
        std::vector<std::string> mConnectedSerialNumbers;

        // current registered devices
        std::vector<RealSenseDevice*> mDevices;

        // Impl contains the rs2::context
        struct Impl;
        std::unique_ptr<Impl> mImpl;
	};
}
