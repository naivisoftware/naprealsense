// Local Includes
#include "pointcloudapp.h"

// External Includes
#include <utility/fileutils.h>
#include <nap/logger.h>
#include <inputrouter.h>
#include <rendergnomoncomponent.h>
#include <perspcameracomponent.h>
#include <renderablemeshcomponent.h>
#include <imguiutils.h>
#include "realsenserenderframecomponent.h"

namespace nap 
{    
    bool PointCloudApp::init(utility::ErrorState& error)
    {
		// Retrieve services
		mRenderService	= getCore().getService<nap::RenderService>();
		mSceneService	= getCore().getService<nap::SceneService>();
		mInputService	= getCore().getService<nap::InputService>();
		mGuiService		= getCore().getService<nap::IMGuiService>();

		// Fetch the resource manager
        mResourceManager = getCore().getResourceManager();

		// Get the render window
		mRenderWindow = mResourceManager->findObject<nap::RenderWindow>("Window");
		if (!error.check(mRenderWindow != nullptr, "unable to find render window with name: %s", "Window"))
			return false;

		// Get the scene that contains our entities and components
		mScene = mResourceManager->findObject<Scene>("Scene");
		if (!error.check(mScene != nullptr, "unable to find scene with name: %s", "Scene"))
			return false;

        mRealSenseDevice = mResourceManager->findObject<RealSenseDevice>("RealSenseDevice");
        if (!error.check(mRealSenseDevice != nullptr, "unable to find RealSenseDevice with name: %s", "RealSenseDevice"))
            return false;

        mRealSenseEntity = mScene->findEntity("RealSenseEntity");
        if (!error.check(mRealSenseEntity != nullptr, "unable to find Entity with name: %s", "RealSenseEntity"))
            return false;

		// Get the camera and origin Gnomon entity
		mCameraEntity = mScene->findEntity("CameraEntity");
        if (!error.check(mCameraEntity != nullptr, "unable to find Entity with name: %s", "CameraEntity"))
            return false;

        mRenderEntity = mScene->findEntity("RenderEntity");
        if (!error.check(mRenderEntity != nullptr, "unable to find Entity with name: %s", "RenderEntity"))
            return false;

		// All done!
        return true;
    }


    // Called when the window is going to render
    void PointCloudApp::render()
    {
		// Signal the beginning of a new frame, allowing it to be recorded.
		// The system might wait until all commands that were previously associated with the new frame have been processed on the GPU.
		// Multiple frames are in flight at the same time, but if the graphics load is heavy the system might wait here to ensure resources are available.
		mRenderService->beginFrame();

		// Begin recording the render commands for the main render window
		if (mRenderService->beginRecording(*mRenderWindow))
		{
			// Begin render pass
			mRenderWindow->beginRendering();

			// Get Perspective camera to render with
			auto& perp_cam = mCameraEntity->getComponent<PerspCameraComponentInstance>();

			// Render objects
			std::vector<nap::RenderableComponentInstance*> components_to_render;
            mRenderEntity->getComponentsOfType(components_to_render);
            if(!components_to_render.empty())
            {
                mRenderService->renderObjects(*mRenderWindow, perp_cam, components_to_render);
            }

			// Draw GUI elements
			mGuiService->draw();

			// Stop render pass
			mRenderWindow->endRendering();

			// End recording
			mRenderService->endRecording();
		}

		// Proceed to next frame
		mRenderService->endFrame();
    }


    void PointCloudApp::windowMessageReceived(WindowEventPtr windowEvent)
    {
		mRenderService->addEvent(std::move(windowEvent));
    }


    void PointCloudApp::inputMessageReceived(InputEventPtr inputEvent)
    {
		// If we pressed escape, quit the loop
		if (inputEvent->get_type().is_derived_from(RTTI_OF(nap::KeyPressEvent)))
		{
			nap::KeyPressEvent* press_event = static_cast<nap::KeyPressEvent*>(inputEvent.get());
			if (press_event->mKey == nap::EKeyCode::KEY_ESCAPE)
				quit();

			if (press_event->mKey == nap::EKeyCode::KEY_f)
				mRenderWindow->toggleFullscreen();
		}
		mInputService->addEvent(std::move(inputEvent));
    }


    int PointCloudApp::shutdown()
    {
		return 0;
    }


    void PointCloudApp::update(double deltaTime)
    {
		// Use a default input router to forward input events (recursively) to all input components in the scene
		// This is explicit because we don't know what entity should handle the events from a specific window.
		nap::DefaultInputRouter input_router(true);
		mInputService->processWindowEvents(*mRenderWindow, input_router, { &mScene->getRootEntity() });

        ImGui::Begin("RealSense");

        ImGui::Text(getCurrentDateTime().toString().c_str());
        ImGui::TextColored(mGuiService->getPalette().mHighlightColor2, "left mouse button to rotate, right mouse button to zoom");
        ImGui::Text(utility::stringFormat("Framerate: %.02f", getCore().getFramerate()).c_str());

        // Display render textures in GUI
        if (ImGui::CollapsingHeader("Textures"))
        {
            auto* color_renderer = mRealSenseEntity->findComponentByID<RealSenseRenderFrameComponentInstance>("RenderColorComponent");
            assert(color_renderer!= nullptr);
            if(color_renderer->isRenderTextureInitialized())
            {
                auto& color_texture = color_renderer->getRenderTexture();
                float col_width = ImGui::GetColumnWidth();
                float ratio = (float)color_texture.getHeight() / (float)color_texture.getWidth();
                ImGui::Text("Color Texture :");
                ImGui::Image(color_texture, ImVec2(col_width, col_width * ratio), ImVec2(0, 0), ImVec2(1, 1));
            }

            auto* depth_renderer = mRealSenseEntity->findComponentByID<RealSenseRenderFrameComponentInstance>("RenderDepthComponent");
            assert(depth_renderer!= nullptr);
            if(depth_renderer->isRenderTextureInitialized())
            {
                auto& depth_texture = depth_renderer->getRenderTexture();
                float col_width = ImGui::GetColumnWidth();
                float ratio = (float)depth_texture.getHeight() / (float)depth_texture.getWidth();
                ImGui::Text("Depth Texture :");
                ImGui::Image(depth_texture, ImVec2(col_width, col_width * ratio), ImVec2(0, 0), ImVec2(1, 1));
            }
        }

        ImGui::End();
    }
}
