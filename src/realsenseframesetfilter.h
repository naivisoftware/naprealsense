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
     * RealSenseFrameSetFilter is a filter that can be applied to a set of frames.
     * It can be part of a vector in the RealSenseFilterStackComponent effectively acting as a filter stack that filters
     * a frameset in a specific order
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
        virtual rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) = 0;
    protected:
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
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        // properties
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH; ///< Property: 'Stream' stream type to align to
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    /**
     * RealSenseFrameSetCutDistanceFilter cuts the range of a depth stream
     */
    class NAPAPI RealSenseFrameSetCutDistanceFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseFrameSetCutDistanceFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameSetCutDistanceFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Changes cutting range
         * @param range
         */
        void changeCuttingRange(const glm::vec2& range);

        // properties
        glm::vec2 mCuttingRange = { 0.2f, 2.0f };    ///< Property: 'CuttingRange' cut range according to depth, minimum and maximum values in meters
    private:
        std::mutex mMutex;
    };

    /**
     * RealSenseFrameSetCutDistanceFilter crops the borders of a video stream
     */
    class NAPAPI RealSenseFrameCropBordersFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseFrameCropBordersFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameCropBordersFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        /**
         * Changes crop
         * @param range
         */
        void changeCrop(const glm::vec4& crop);

        // properties
        ERealSenseStreamType mStreamType = ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH; ///< Property: 'StreamType' video streamtype to crop
        glm::vec4 mBorderCrop = { 0.0f, 0.0f, 0.0f, 0.0f }; ///< Property: 'BorderCrop' crops image left, top, right, bottom side using normalized (0-1) values
    private:
        std::mutex mMutex;
    };

    /**
     * RealSenseFrameHolesFillingFilter implements the RealSense hole filling algorithm
     * The filter implements several methods to rectify missing data in the resulting image.
     * The filter obtains the four immediate pixel "neighbors" (up, down ,left, right), and selects one of them according to a user-defined rule.
     * See https://dev.intelrealsense.com/docs/post-processing-filters
     */
    class NAPAPI RealSenseFrameHolesFillingFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Enum describing hole filling method
         */
        enum NAPAPI EHoleFilling : int
        {
            FILL_FROM_LEFT = 0,
            FARTHEST_FROM_AROUND = 1,
            NEAREST_FROM_AROUND = 2
        };

        /**
         * Constructor
         */
        RealSenseFrameHolesFillingFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseFrameHolesFillingFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        // properties
        EHoleFilling mHoleFilling = EHoleFilling::FARTHEST_FROM_AROUND; ///< Property: 'HoleFilling' Control the data that will be used to fill the invalid pixels
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };

    /**
     * RealSenseTemporalFilter implements the RealSense Temporal filter algorithm
     * The temporal filter is intended to improve the depth data persistency by manipulating per-pixel values based on previous frames.
     * The filter performs a single pass on the data, adjusting the depth values while also updating the tracking history. In cases where the pixel data is missing or invalid the filter uses a user-defined persistency mode to decide whether the missing value should be rectified with stored data.
     * Note that due to its reliance on historic data the filter may introduce visible blurring/smearing artefacts, and therefore is best-suited for static scenes.
     * See https://dev.intelrealsense.com/docs/post-processing-filters
     */
    class NAPAPI RealSenseTemporalFilter final : public RealSenseFrameSetFilter
    {
    RTTI_ENABLE(RealSenseFrameSetFilter)
    public:
        /**
         * Constructor
         */
        RealSenseTemporalFilter();

        /**
         * Destructor
         */
        virtual ~RealSenseTemporalFilter();

        /**
         * Called from RealSenseDevice on RealSense process thread before frameset is passed onto FrameSetListener components
         * @param frameset the frameset
         * @return the filtered frameset
         */
        rs2::frameset process(RealSenseDevice* device, const rs2::frameset& frameset) override;

        /**
         * Initialization
         * @param errorState contains any errors
         * @return true on success
         */
        bool init(utility::ErrorState& errorState) override;

        // properties
        float mSmoothAlpha = 0.4f; ///< Property: 'SmoothAlpha' The Alpha factor in an exponential moving average with Alpha=1 - no filter . Alpha = 0 - infinite filter
        int mSmoothDelta = 20; ///< Property: 'SmoothDelta' Step-size boundary. Establishes the threshold used to preserve surfaces (edges) [0-100]
        int mPersistenceIndex = 3; ///< Property: 'PersistenceIndex' A set of predefined rules (masks) that govern when missing pixels will be replace with the last valid value so that the data will remain persistent over time [0-8]
    private:
        struct Impl;
        std::unique_ptr<Impl> mImpl;
    };
}
