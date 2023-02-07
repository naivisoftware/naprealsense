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
     * RealSenseFrameSetFilter
     * RealSenseFrameSetFilter base class, overidde process to apply filtering on rs2::framesets
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
         * Process function, returns processed rs2::frameset
         * @param frameset frameset to filter
         * @return processed frameset
         */
        virtual rs2::frameset process(const rs2::frameset& frameset) = 0;
    private:
    };

    /**
     * RealSenseFrameSetAlignFilter
     * RealSenseFrameSetAlignFilter aligns one frame to another frame within the frameset according to given streamtype
     */
    class NAPAPI RealSenseFrameSetAlignFilter : public RealSenseFrameSetFilter
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
         * Process function, returns processed rs2::frameset
         * @param frameset frameset to filter
         * @return processed frameset
         */
        rs2::frameset process(const rs2::frameset& frameset) override;

        /**
         * Initialization method
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        // Properties
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH; ///< Property: 'Align To' StreamType to align frames to
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
