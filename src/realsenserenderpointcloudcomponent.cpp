#include "realsenserenderpointcloudcomponent.h"
#include "realsensedevice.h"

// RealSense includes
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS(nap::RealSenseRenderPointCloudComponent)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderPointCloudComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
    struct RealSenseRenderPointCloudComponentInstance::Impl
    {
    public:
        // Declare pointcloud object, for calculating pointclouds and texture mappings
        rs2::pointcloud mPointCloud;

        // We want the points object to be persistent so we can display the last cloud when a frame drops
        rs2::points mPoints;
    };


    RealSenseRenderPointCloudComponentInstance::RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource)
        : RealSenseFrameSetListenerComponentInstance(entity, resource)
    {
        mImplementation = std::make_unique<Impl>();
    }


    RealSenseRenderPointCloudComponentInstance::~RealSenseRenderPointCloudComponentInstance()
    {

    }


    void RealSenseRenderPointCloudComponentInstance::onTrigger(const rs2::frameset& frameset)
    {
        auto color = frameset.get_color_frame();

        // For cameras that don't have RGB sensor, we'll map the pointcloud to infrared instead of color
        if (!color)
            color = frameset.get_infrared_frame();

        // Tell pointcloud object to map to this color frame
        mImplementation->mPointCloud.map_to(color);

        auto depth = frameset.get_depth_frame();

        // Generate the pointcloud and texture mappings
        mImplementation->mPoints = mImplementation->mPointCloud.calculate(depth);

    }
}