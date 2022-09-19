#pragma once

#include <nap/core.h>

namespace nap
{
    enum NAPAPI ERealSenseFrameTypes : int
    {
        COLOR = 0,
        DEPTH = 1
    };

    enum NAPAPI ERealSenseStreamFormat : int
    {
        RGBA8 = 0,
        Z16 = 1
    };
}