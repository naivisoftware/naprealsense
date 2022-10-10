#include "realsenserenderpointcloudcomponent.h"
#include "realsensedevice.h"
#include "renderglobals.h"

#include <renderservice.h>
#include <meshutils.h>

// RealSense includes
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API

RTTI_BEGIN_CLASS(nap::RealSenseRenderPointCloudComponent)
        RTTI_PROPERTY("Mesh", &nap::RealSenseRenderPointCloudComponent::mMesh, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("Device", &nap::RealSenseRenderPointCloudComponent::mDevice, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("RenderableMesh", &nap::RealSenseRenderPointCloudComponent::mRenderableMeshComponent, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("CameraTransform", &nap::RealSenseRenderPointCloudComponent::mCameraTransform, nap::rtti::EPropertyMetaData::Required)
        RTTI_PROPERTY("PointSize", &nap::RealSenseRenderPointCloudComponent::mPointSize, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RealSenseRenderPointCloudComponentInstance)
    RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PointCloudMesh)
    RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    PointCloudMesh::PointCloudMesh(Core& core) :
        mRenderService(core.getService<RenderService>())
    { }

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
        // Construct bounding rect
        float dsizex = (0.0f - (mSize.x / 2.0f)) + mPosition.x;
        float dsizey = (0.0f - (mSize.y / 2.0f)) + mPosition.y;
        math::Rect rect(dsizex, dsizey, mSize.x, mSize.y);

        // Create plane
        assert(mRenderService != nullptr);
        mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);

        constructPointCloud(*mMeshInstance, 100000);

        // Store rect
        mRect = rect;

        return true;
    }


    void PointCloudMesh::constructPointCloud(nap::MeshInstance& mesh, size_t vertCount)
    {
        // Get incremental stepping values
        std::vector<glm::vec3> vertices(vertCount, { 0.0f,0.0f,0.0f });
        std::vector<glm::vec3> normals(vertCount, { 0.0f,0.0f,1.0f });
        std::vector<glm::vec3> uvs(vertCount, { 0.0f,0.0f,0.0f });

        Vec3VertexAttribute& position_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::position);
        Vec3VertexAttribute& normal_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::normal);
        Vec3VertexAttribute& uv_attribute = mesh.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
        Vec4VertexAttribute& color_attribute = mesh.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));

        // Set the number of vertices to use
        mesh.setNumVertices(vertCount);
        mesh.setDrawMode(EDrawMode::Points);
        mesh.setUsage(mUsage);
        mesh.setCullMode(mCullMode);
        mesh.setPolygonMode(mPolygonMode);

        // Push vertex data
        position_attribute.setData(vertices.data(), vertCount);
        normal_attribute.setData(normals.data(), vertCount);
        uv_attribute.setData(uvs.data(), vertCount);
        color_attribute.setData({vertCount, mColor.toVec4()});

        std::vector<uint32> indices(vertCount, 0);
        for(size_t i = 0; i < vertCount; i++)
        {
            indices[i] = i;
        }

        MeshShape& shape = mesh.createShape();
        shape.setIndices(indices.data(), indices.size());
    }

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


    bool RealSenseRenderPointCloudComponentInstance::onInit(utility::ErrorState& errorState)
    {
        auto* resource = getComponent<RealSenseRenderPointCloudComponent>();
        mMesh = resource->mMesh.get();
        mDevice = resource->mDevice.get();
        mPointSize = resource->mPointSize;

        frameSetReceived.connect([this](const rs2::frameset& frameset){ onTrigger(frameset); });

        return true;
    }


    void RealSenseRenderPointCloudComponentInstance::update(double deltaTime)
    {
        const auto& camera_intrincics = mDevice->getIntrincicsMap();
        const auto& intrincics = camera_intrincics.find(ERealSenseStreamType::REALSENSE_STREAMTYPE_DEPTH)->second;
        auto& material_instance = mRenderableMesh->getMaterialInstance();
        auto* ubo = material_instance.getOrCreateUniform("cam_intrinsics");
        float depth_scale = mDevice->getDepthScale();
        ubo->getOrCreateUniform<UniformFloatInstance>("width")->setValue(intrincics.mWidth);
        ubo->getOrCreateUniform<UniformFloatInstance>("height")->setValue(intrincics.mHeight);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppx")->setValue(intrincics.mPPX);
        ubo->getOrCreateUniform<UniformFloatInstance>("ppy")->setValue(intrincics.mPPY);
        ubo->getOrCreateUniform<UniformFloatInstance>("fx")->setValue(intrincics.mFX);
        ubo->getOrCreateUniform<UniformFloatInstance>("fy")->setValue(intrincics.mFY);
        ubo->getOrCreateUniform<UniformIntInstance>("model")->setValue(static_cast<int>(intrincics.mModel));
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[0], 0);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[1], 1);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[2], 2);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[3], 3);
        ubo->getOrCreateUniform<UniformFloatArrayInstance>("coeffs")->setValue(intrincics.mCoeffs[4], 4);

        ubo = material_instance.getOrCreateUniform("UBO");
        ubo->getOrCreateUniform<UniformVec3Instance>("camera_world_position")->setValue(math::extractPosition(mCameraTransform->getGlobalTransform()));
        ubo->getOrCreateUniform<UniformFloatInstance>("realsense_depth_scale")->setValue(depth_scale);
        ubo->getOrCreateUniform<UniformFloatInstance>("point_size_scale")->setValue(mPointSize);
    }


    void RealSenseRenderPointCloudComponentInstance::onTrigger(const rs2::frameset& frameset)
    {

        /*
        auto color = frameset.get_color_frame();

        // For cameras that don't have RGB sensor, we'll map the pointcloud to infrared instead of color
        if (!color)
            color = frameset.get_infrared_frame();

        // Tell pointcloud object to map to this color frame
        mImplementation->mPointCloud.map_to(color);

        auto depth = frameset.get_depth_frame();

        // Generate the pointcloud and texture mappings
        mImplementation->mPoints = mImplementation->mPointCloud.calculate(depth);

        mDirty = true;

        auto& mesh_instance = mMesh->getMeshInstance();

        if(mImplementation->mPoints.size()!=mesh_instance.getNumVertices())
        {
            mMesh->constructPointCloud(mesh_instance, mImplementation->mPoints.size());
        }

        auto& vertex_attribute = mesh_instance.getAttribute<glm::vec3>(vertexid::position);
        auto& uv_attribute = mesh_instance.getAttribute<glm::vec3>(vertexid::uv);
        auto& color_attribute = mesh_instance.getAttribute<glm::vec4>(vertexid::color);
        auto& vertex_positions = vertex_attribute.getData();
        auto& tex_coords = uv_attribute.getData();
        auto& colors = color_attribute.getData();
        auto* pc_vertices   = mImplementation->mPoints.get_vertices();              // get vertices
        auto* pc_tex_coords = mImplementation->mPoints.get_texture_coordinates(); // and texture coordinates
        for(size_t i = 0; i < mImplementation->mPoints.size(); i++)
        {
            if(pc_vertices[i].z)
            {
                glm::vec3 p = glm::vec3(pc_vertices[i].x, pc_vertices[i].y, pc_vertices[i].z);
                vertex_positions[i] = p;
                tex_coords[i] = glm::vec3(pc_tex_coords[i].u, pc_tex_coords[i].v, 0.0f);
                colors[i] = glm::vec4(1,1,1,1);
            }else
            {
                colors[i] = glm::vec4(0,0,0,0);
            }
        }

        utility::ErrorState errorState;
        if(!mesh_instance.update(errorState))
        {
            nap::Logger::warn(errorState.toString());
        }*/
    }
}