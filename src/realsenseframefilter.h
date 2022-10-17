/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <rtti/rtti.h>
#include <nap/resourceptr.h>

// Local includes
#include "realsensetypes.h"
#include "realsenseframesetlistenercomponent.h"

// RealSense frame forward declaration
namespace rs2
{
    class frame;
}

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * Base class of every FrameFilter
     */
    class NAPAPI RealSenseFrameFilter : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        /**
         * Constructor
         */
        RealSenseFrameFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameFilter();

        /**
         * Must be called from the RealSense process thread by a RealSenseFrameSetListenerComponent
         * @param frame const reference to frame to filter
         * @return the filtered frame
         */
        virtual rs2::frame process(const rs2::frame& frame) = 0;
    private:
    };

    /**
     * RealSense Spatial Filter
     */
    class NAPAPI RealSenseSpatialFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        /**
         * Constructor
         */
        RealSenseSpatialFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseSpatialFilter();

        /**
         * Init method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Must be called from the RealSense process thread by a RealSenseFrameSetListenerComponent
         * @param frame const reference to frame to filter
         * @return the filtered frame
         */
        rs2::frame process(const rs2::frame& frame) override;

        float mCutoff = 0.5f; ///< Property: 'Cutoff' spatial cutoff value
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    /**
     * RealSense decimation filter
     */
    class NAPAPI RealSenseDecFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        /**
         * Constructor
         */
        RealSenseDecFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseDecFilter();

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Must be called from the RealSense process thread by a RealSenseFrameSetListenerComponent
         * @param frame const reference to frame to filter
         * @return the filtered frame
         */
        rs2::frame process(const rs2::frame& frame) override;

        float mMagnitude = 3.0f; ///< Property: 'Magnitude' decimation magnitude
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    /**
     * RealSense disparity filter
     */
    class NAPAPI RealSenseDisparityFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        /**
         * Constructor
         */
        RealSenseDisparityFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseDisparityFilter();

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Must be called from the RealSense process thread by a RealSenseFrameSetListenerComponent
         * @param frame const reference to frame to filter
         * @return the filtered frame
         */
        rs2::frame process(const rs2::frame& frame) override;

        bool mToDisparity = false; ///< Property: 'ToDisparity'
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    class NAPAPI RealSenseColorizeFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        /*
         * Constructor
         */
        RealSenseColorizeFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseColorizeFilter();

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Must be called from the RealSense process thread by a RealSenseFrameSetListenerComponent
         * @param frame const reference to frame to filter
         * @return the filtered frame
         */
        rs2::frame process(const rs2::frame& frame) override;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
