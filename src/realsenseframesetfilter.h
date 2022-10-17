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

namespace rs2
{
    class frameset;
}

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * RealSenseFrameSetFilter is a filter that can be applied to a set of frames
     */
    class NAPAPI RealSenseFrameSetFilter : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        virtual rs2::frameset process(const rs2::frameset& frameset) = 0;
    private:
    };

    /**
     * Aligns a frameset to a specified stream type (f.e. color to depth or depth to color)
     */
    class NAPAPI RealSenseFrameSetAlignFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetAlignFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetAlignFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH; ///< Property: 'Stream' stream type to align to
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
