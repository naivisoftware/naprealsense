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

// rs2 forward declares
namespace rs2
{
    class frame;
}

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * RealSenseFrameFilter
     * Base class of a frame filter that can be applied to a frame out of a rs2::frameset
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
         * Process function, returns processed frame and takes a rs2::frame as input
         * @param frame frame to process
         * @return processed frame
         */
        virtual rs2::frame process(const rs2::frame& frame) = 0;
    private:
    };

    /**
     * RealSenseSpatialFilter
     * Spatial-Edge Preserving filter
     * See : https://dev.intelrealsense.com/docs/post-processing-filters
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
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Process function, returns processed frame and takes a rs2::frame as input
         * @param frame frame to process
         * @return processed frame
         */
        rs2::frame process(const rs2::frame& frame) override;

        // Properties
        float mMagnitude = 2.0f; ///< Property: 'Magnitude' Magnitude value
        float mSmoothAlpha = 0.5f; ///< Property: 'SmoothAlpha' SmoothAlpha value
        float mSmoothDelta = 20.0f; ///< Property: 'SmoothDelta' SmoothDelta value
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    /**
     * RealSenseDecFilter
     * Decimation filter
     * See : https://dev.intelrealsense.com/docs/post-processing-filters
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
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Process function, returns processed frame and takes a rs2::frame as input
         * @param frame frame to process
         * @return processed frame
         */
        rs2::frame process(const rs2::frame& frame) override;

        // Properties
        float mMagnitude = 3.0f; ///< Property: 'Magnitude' Magnitude value
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    /**
     * RealSenseColorizeFilter
     * Process a depth frame and returns a colored depth frame using a rs2::colorizer
     * Also see : https://intelrealsense.github.io/librealsense/doxygen/classrs2_1_1colorizer.html
     */
    class NAPAPI RealSenseColorizeFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        /**
         * Constructor
         */
        RealSenseColorizeFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseColorizeFilter();

        /**
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Process function, returns processed frame and takes a rs2::frame as input
         * @param frame frame to process
         * @return processed frame
         */
        rs2::frame process(const rs2::frame& frame) override;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
