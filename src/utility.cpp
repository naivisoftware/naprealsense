#include <cstring>
#include "utility.h"

#include <rs.hpp>

namespace nap
{
namespace realsense
{
    void removeBackground(const rs2::depth_frame& depthFrame,
                           rs2::video_frame& videoFrame,
                           float depthScale,
                           glm::vec2 clippingDist)
    {
        const uint16_t *p_depth_frame = reinterpret_cast<const uint16_t *>(depthFrame.get_data());
        uint8_t *p_other_frame = reinterpret_cast<uint8_t *>(const_cast<void *>(videoFrame.get_data()));

        int width = videoFrame.get_width();
        int height = videoFrame.get_height();
        int other_bpp = videoFrame.get_bytes_per_pixel();

#pragma omp parallel for schedule(dynamic) //Using OpenMP to try to parallelise the loop
        for (int y = 0; y < height; y++)
        {
            auto depth_pixel_index = y * width;
            for (int x = 0; x < width; x++, ++depth_pixel_index)
            {
                // Get the depth value of the current pixel
                auto pixels_distance = depthScale * p_depth_frame[depth_pixel_index];

                // Check if the depth value is invalid (<=0) or greater than the threashold
                if (pixels_distance <= clippingDist.x || pixels_distance > clippingDist.y)
                {
                    // Calculate the offset in other frame's buffer to current pixel
                    auto offset = depth_pixel_index * other_bpp;

                    // Set pixel to "background" color
                    std::memset(&p_other_frame[offset], 0, other_bpp);
                }
            }
        }
    }


    void crop(rs2::video_frame& outputFrame,
              const glm::vec4& crop)
    {
        uint8_t *p_output_frame = reinterpret_cast<uint8_t *>(const_cast<void *>(outputFrame.get_data()));

        int width = outputFrame.get_width();
        int height = outputFrame.get_height();
        int other_bpp = outputFrame.get_bytes_per_pixel();

#pragma omp parallel for schedule(dynamic) //Using OpenMP to try to parallelise the loop
        for (int y = 0; y < height; y++)
        {
            auto depth_pixel_index = y * width;
            float y_norm = static_cast<float>(y) / static_cast<float>(height);

            if(y_norm > crop.y && y_norm < 1.0f - crop.w)
            {
                for (int x = 0; x < width; x++, ++depth_pixel_index)
                {
                    float x_norm = static_cast<float>(x) / static_cast<float>(width);
                    if(x_norm < crop.x || x_norm > 1.0f - crop.z)
                    {
                        // Calculate the offset in other frame's buffer to current pixel
                        auto offset = depth_pixel_index * other_bpp;

                        // Set pixel to "background" color
                        std::memset(&p_output_frame[offset], 0, other_bpp);
                    }
                }
            }else
            {
                for (int x = 0; x < width; x++, ++depth_pixel_index)
                {
                    // Calculate the offset in other frame's buffer to current pixel
                    auto offset = depth_pixel_index * other_bpp;

                    // Set pixel to "background" color
                    std::memset(&p_output_frame[offset], 0, other_bpp);
                }
            }
        }
    }
} // realsense
} // nap