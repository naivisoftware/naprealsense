#pragma once

#include <rtti/rtti.h>
#include <nap/core.h>
#include <mesh.h>
#include <rect.h>
#include <color.h>

namespace nap
{
    class NAPAPI PointCloudMesh : public IMesh
    {
    RTTI_ENABLE(IMesh)
    public:
        PointCloudMesh(Core& core);

        virtual bool init(utility::ErrorState& errorState) override;

        bool setup(utility::ErrorState& error);

        virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }

        virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

        // property: the size of the plane
        ECullMode		mCullMode		= ECullMode::None;					///< Property: 'CullMode' Plane cull mode, defaults to no culling
        int             mRows           = 100;
        int             mColums         = 100;

        void constructPointCloud(nap::MeshInstance& mesh);
    private:
        EPolygonMode	mPolygonMode	= EPolygonMode::Point;		///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)
        EMemoryUsage	mUsage			= EMemoryUsage::Static;		///< Property: 'Usage' If the plane is uploaded once or frequently updated.

        RenderService* mRenderService;
        std::unique_ptr<MeshInstance> mMeshInstance;
    };
}

