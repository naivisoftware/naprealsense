#pragma once

#include <rtti/rtti.h>
#include <nap/core.h>
#include <mesh.h>
#include <rect.h>
#include <color.h>

namespace nap
{
    //////////////////////////////////////////////////////////////////////////

    /**
     * PointCloudMesh
     * A mesh that consists of points. Amount of points is determined by rows and columns
     */
    class NAPAPI PointCloudMesh : public IMesh
    {
    RTTI_ENABLE(IMesh)
    public:
        /**
         * Constructor
         * @param core reference to NAP core
         */
        PointCloudMesh(Core& core);

        /**
         * Destructor
         */
        virtual ~PointCloudMesh();

        /**
         * Initialization method, creates mesh
         * @param errorState contains any error
         * @return true on success
         */
        virtual bool init(utility::ErrorState& errorState) override;

        /**
         * Returns MeshInstance reference
         * @return MeshInstance reference
         */
        virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }

        /**
         * Returns const MeshInstance reference
         * @return const MeshInstance reference
         */
        virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

        // property: the size of the plane
        ECullMode		mCullMode		= ECullMode::None;	///< Property: 'CullMode' Plane cull mode, defaults to no culling
        int             mRows           = 100;              ///< Property: 'Rows' Amount of rows
        int             mColums         = 100;              ///< Property: 'Columns' Amount of columns
        float           mSize           = 1.0f;             ///< Property: 'Size' size

        /**
         * Constructs pointcloud and updates mesh
         * @param mesh reference to MeshInstance
         */
        void constructPointCloud(nap::MeshInstance& mesh);
    private:
        EPolygonMode	mPolygonMode	= EPolygonMode::Point;
        EMemoryUsage	mUsage			= EMemoryUsage::Static;

        RenderService* mRenderService;
        std::unique_ptr<MeshInstance> mMeshInstance;
    };
}

