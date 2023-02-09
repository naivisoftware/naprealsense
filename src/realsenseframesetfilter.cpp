#include "realsenseframesetfilter.h"
#include "realsensedevice.h"

#include <rs.hpp>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseFrameSetFilter)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::RealSenseFrameSetAlignFilter)
    RTTI_PROPERTY("Align To", &nap::RealSenseFrameSetAlignFilter::mStreamType, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetFilter
    //////////////////////////////////////////////////////////////////////////

    RealSenseFrameSetFilter::RealSenseFrameSetFilter() = default;


    RealSenseFrameSetFilter::~RealSenseFrameSetFilter() = default;

    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetAlignFilter::Impl
    //////////////////////////////////////////////////////////////////////////

    struct RealSenseFrameSetAlignFilter::Impl
    {
    public:
        Impl(rs2_stream streamType) :
                mAlign(streamType){}

        rs2::align mAlign;
    };

    //////////////////////////////////////////////////////////////////////////
    // RealSenseFrameSetAlignFilter
    //////////////////////////////////////////////////////////////////////////

    RealSenseFrameSetAlignFilter::RealSenseFrameSetAlignFilter() = default;


    RealSenseFrameSetAlignFilter::~RealSenseFrameSetAlignFilter() = default;


    bool RealSenseFrameSetAlignFilter::init(utility::ErrorState &errorState)
    {
        try
        {
            mImpl = std::make_unique<Impl>(static_cast<rs2_stream>(mStreamType));
        }catch(std::exception& e)
        {
            errorState.fail(e.what());
            return false;
        }

        return true;
    }


    rs2::frameset RealSenseFrameSetAlignFilter::process(const rs2::frameset& frameset)
    {
        return mImpl->mAlign.filter::process(frameset);
    }
}
