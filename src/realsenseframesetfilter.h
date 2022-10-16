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

    class NAPAPI RealSenseFrameSetFilter : public Resource
    {
    RTTI_ENABLE(Resource)
    public:
        RealSenseFrameSetFilter();

        virtual ~RealSenseFrameSetFilter();

        virtual rs2::frameset process(const rs2::frameset& frame) = 0;
    private:
    };

    class NAPAPI RealSenseFrameSetAlignFilter : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        RealSenseFrameSetAlignFilter();

        virtual ~RealSenseFrameSetAlignFilter();

        rs2::frameset process(const rs2::frameset& frameset) override;

        bool init(utility::ErrorState& errorState) override;

        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
