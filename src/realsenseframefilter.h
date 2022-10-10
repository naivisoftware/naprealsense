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
    class frame;
}

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    class NAPAPI RealSenseFrameFilter : public Resource
    {
        RTTI_ENABLE(Resource)
    public:
        virtual rs2::frame process(const rs2::frame& frame) = 0;
    private:
    };

    class NAPAPI RealSenseSpatialFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        bool init(utility::ErrorState& errorState) override;

        rs2::frame process(const rs2::frame& frame) override;

        float mCutoff = 0.5f;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    class NAPAPI RealSenseDecFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        bool init(utility::ErrorState& errorState) override;

        rs2::frame process(const rs2::frame& frame) override;

        float mMagnitude = 3.0f;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    class NAPAPI RealSenseDisparityFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        bool init(utility::ErrorState& errorState) override;

        rs2::frame process(const rs2::frame& frame) override;

        bool mToDisparity = false;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    class NAPAPI RealSenseColorizeFilter : public RealSenseFrameFilter
    {
    RTTI_ENABLE(RealSenseFrameFilter)
    public:
        bool init(utility::ErrorState& errorState) override;

        rs2::frame process(const rs2::frame& frame) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
