#pragma once

#include <mesh.h>
#include <rect.h>

#include "realsenseframesetlistenercomponent.h"

namespace nap
{
    class RealSenseRenderPointCloudComponentInstance;
    class RenderService;

    /**
	 */
    class NAPAPI PointCloudMesh : public IMesh
    {
    RTTI_ENABLE(IMesh)
    public:
        PointCloudMesh(Core& core);

        /**
         * Sets up, initializes and uploads the plane to the GPU based on the provided parameters.
         * @param errorState contains the error message if the mesh could not be created.
         * @return if the mesh was successfully created and initialized.
         */
        virtual bool init(utility::ErrorState& errorState) override;

        /**
        * Creates and prepares the mesh instance but doesn't initialize it.
        * Call this when you want to prepare a grid without creating the GPU representation.
        * You have to manually call init() on the mesh instance afterwards.
        * @param error contains the error code if setup fails
        * @return if setup succeeded
        */
        bool setup(utility::ErrorState& error);

        /**
         *	@return the mesh used for rendering
         */
        virtual MeshInstance& getMeshInstance() override					{ return *mMeshInstance; }

        /**
         *	@return the mesh used for rendering
         */
        virtual const MeshInstance& getMeshInstance() const override		{ return *mMeshInstance; }

        /**
         *	@return the plane as a rectangle
         */
        const math::Rect& getRect()											{ return mRect; }

        // property: the size of the plane
        glm::vec2		mSize			= { 1.0, 1.0 };						///< Property: 'Size' the size of the plane in units
        glm::vec2		mPosition		= { 0.0,0.0 };						///< Property: 'Position' where the plane is positioned in object space
        RGBAColorFloat	mColor			= { 1.0f, 1.0f, 1.0f, 1.0f };		///< Property: 'Color' color of the plane
        EMemoryUsage	mUsage			= EMemoryUsage::DynamicWrite;		///< Property: 'Usage' If the plane is uploaded once or frequently updated.
        ECullMode		mCullMode		= ECullMode::None;					///< Property: 'CullMode' Plane cull mode, defaults to no culling
        EPolygonMode	mPolygonMode	= EPolygonMode::Point;				///< Property: 'PolygonMode' Polygon rasterization mode (fill, line, points)

        void constructPointCloud(nap::MeshInstance& mesh, size_t vertCount);

    private:
        RenderService* mRenderService;
        std::unique_ptr<MeshInstance> mMeshInstance;
        math::Rect mRect;
    };

    class NAPAPI RealSenseRenderPointCloudComponent : public RealSenseFrameSetListenerComponent
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponent)
    DECLARE_COMPONENT(RealSenseRenderPointCloudComponent, RealSenseRenderPointCloudComponentInstance)
    public:
        ResourcePtr<PointCloudMesh> mMesh;
    };

    class NAPAPI RealSenseRenderPointCloudComponentInstance : public RealSenseFrameSetListenerComponentInstance
    {
    RTTI_ENABLE(RealSenseFrameSetListenerComponentInstance)
    public:
        RealSenseRenderPointCloudComponentInstance(EntityInstance& entity, Component& resource);

        virtual ~RealSenseRenderPointCloudComponentInstance();
    protected:
        bool onInit(utility::ErrorState& errorState) override;

        void onTrigger(const rs2::frameset& frameset);

        void update(double deltaTime) override;
    private:
        struct Impl;
        std::unique_ptr<Impl> mImplementation;

        PointCloudMesh* mMesh;
        bool mDirty = false;
    };
}