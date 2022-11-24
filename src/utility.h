#pragma once

#include <nap/core.h>
#include <glm/glm.hpp>

// rs2 forward declares
namespace rs2
{
    class depth_frame;
    class video_frame;
}

namespace nap
{
namespace realsense
{
    /**
     * Set pixel values of videoFrame to black given a specific depth range together with rs2::depth_frame.
     * depthFrame and videoFrame can be the same Frame
     * depthFrame and videoFrame must have the same dimensions (width & height)
     * @param depthFrame the depth frame
     * @param videoFrame the aligned video frame
     * @param depthScale the depth scale. (see RealSenseDevice::getDepthScale())
     * @param clippingDist the clipping distance (minimum, maximum) in meters
     */
    void NAPAPI removeBackground(const rs2::depth_frame& depthFrame,
                                 rs2::video_frame& videoFrame,
                                 float depthScale,
                                 glm::vec2 clippingDist);

    /**
     * Set pixel values of videoFrame to black given a specific crop.
     * Crop is in normalized values starting from left in clockwise order (left, top, right, bottom)
     * A value of 1 means everything is clipped starting from that side
     * @param videoFrame The video frame
     * @param crop normalized crop values
     */
    void NAPAPI crop(rs2::video_frame& videoFrame,
                     const glm::vec4& crop);
} // realsense
} // nap