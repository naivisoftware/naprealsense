#include <renderservice.h>
#include <renderglobals.h>

#include "pointcloudmesh.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointCloudMesh)
    RTTI_CONSTRUCTOR(nap::Core&)
    RTTI_PROPERTY("Rows", &nap::PointCloudMesh::mRows, nap::rtti::EPropertyMetaData::Default)
    RTTI_PROPERTY("Colums", &nap::PointCloudMesh::mColums, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
    PointCloudMesh::PointCloudMesh(Core& core) : mRenderService(core.getService<RenderService>()) {}

    bool PointCloudMesh::init(utility::ErrorState& errorState)
    {
        // Setup plane
        if (!setup(errorState))
            return false;

        // Initialize instance
        return mMeshInstance->init(errorState);
    }

    bool PointCloudMesh::setup(utility::ErrorState& error)
    {
        assert(mRenderService != nullptr);
        mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

        constructPointCloud(*mMeshInstance);

        return true;
    }


    void PointCloudMesh::constructPointCloud(nap::MeshInstance& mesh)
    {
        size_t vert_count = mRows * mColums;

        // Get incremental stepping values
        std::vector<glm::vec3> vertices(vert_count, {0.0f, 0.0f, 0.0f});
        std::vector<glm::vec3> normals(vert_count, {0.0f, 0.0f, 1.0f});
        std::vector<glm::vec3> uvs(vert_count, {0.0f, 0.0f, 0.0f});

        Vec3VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
        Vec3VertexAttribute& normal_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::normal);
        Vec3VertexAttribute& uv_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
        Vec4VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

        // Set the number of vertices to use
        mesh.setNumVertices(vert_count);
        mesh.setDrawMode(EDrawMode::Points);
        mesh.setUsage(mUsage);
        mesh.setCullMode(mCullMode);
        mesh.setPolygonMode(mPolygonMode);

        // Push vertex data
        std::vector<uint32> indices(vert_count, 0);
        for (size_t i = 0; i < vert_count; i++) {
            float x = static_cast<float>(i % mRows) / static_cast<float>(mColums);
            float y = static_cast<float>(i / mColums) / static_cast<float>(mRows);

            uvs[i] = {x, y, 0};
            indices[i] = i;
        }

        position_attribute.setData(vertices.data(), vert_count);
        normal_attribute.setData(normals.data(), vert_count);
        uv_attribute.setData(uvs.data(), vert_count);
        color_attribute.setData({vert_count, {1, 1, 1, 1}});

        MeshShape& shape = mesh.createShape();
        shape.setIndices(indices.data(), indices.size());
    }
}