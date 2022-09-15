/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "realsensedevice.h"

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    // forward declares
    class RealSenseCameraCapture;
    class Image;

    class NAPAPI RealSenseCamera : public RealSenseDevice
    {
    RTTI_ENABLE(RealSenseDevice)
    public:
        RealSenseCamera(RealSenseService& service);

        virtual bool init(utility::ErrorState& errorState) override;

        virtual void update(double deltaTime) override;

        // properties
        ResourcePtr<Image> mImage;
    protected:
        virtual bool onStart(utility::ErrorState& errorState) override final;

        virtual void onStop() override final;

        virtual void onProcess() override final;
    private:
        std::unique_ptr<RealSenseCameraCapture> mCameraCapture;

    };

    using RealSenseCameraObjectCreator = rtti::ObjectCreator<RealSenseCamera, RealSenseService>;
}
