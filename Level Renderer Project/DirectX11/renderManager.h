#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib") //needed for runtime shader compilation. Consider compiling shaders before runtime 

#include "load_object_oriented.h"
#include <commdlg.h>

GW::INPUT::GInput gInput;
GW::SYSTEM::GLog info;

std::string levelLoaded;

class RenderManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	Model model;
	Level_Objects gameLevel;

	GW::MATH::GMATRIXF world;
	

	GW::AUDIO::GAudio gAudio;
	GW::AUDIO::GMusic gMusic;

	bool fullScreen;

public:

	RenderManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface& _d3d)
	{
		win = _win;
		d3d = _d3d;

		info.Create("output.txt");
		info.EnableConsoleLogging(true);

		levelLoaded = gameLevel.LoadLevel("../Levels/Level 2/Level2.txt", "../Levels/Level 2/Models", info);

		//InitializeWorldMatrix();
		//InitializeViewProjectionMatrix();
		//InitializeLight();

		gInput.Create(win);
		gAudio.Create();

		if (gMusic.Create("../Audio/DSLofi.wav", gAudio) == GW::GReturn::SUCCESS)
		{
			gMusic.SetVolume(0.05f);
			gMusic.Play(true);
		}
		else
		{
			std::cout << "Failed to create or play music." << std::endl;
		}

		gameLevel.UploadLevelToGPU(win, d3d);
	}

private:

	void InitializeWorldMatrix()
	{
		matrixProxy.Create();
		matrixProxy.IdentityF(world);

		mData.worldMatrix = world;
	}

	void InitializeViewProjectionMatrix()
	{
		matrixProxy.IdentityF(view);
		GW::MATH::GVECTORF cameraPosition = { 0.0f, 15.0f, -40.0f, 1.0f };
		GW::MATH::GVECTORF targetPosition = { 0.0f, -15.0f, 0.0f, 1.0f };
		GW::MATH::GVECTORF upDirection = { 0.0f, 1.0f, 0.0f, 0.0f };
		matrixProxy.LookAtLHF(cameraPosition, targetPosition, upDirection, view);

		sData.cameraPos = cameraPosition;

		float fovY = 65.0f * (G_PI_F / 180.0f);
		float aspectRatio;
		float nearPlane = 0.1f;
		float farPlane = 200.0f;
		d3d.GetAspectRatio(aspectRatio);

		matrixProxy.IdentityF(projection);
		matrixProxy.ProjectionDirectXLHF(fovY, aspectRatio, nearPlane, farPlane, projection);
		matrixProxy.MultiplyMatrixF(view, projection, viewProjection);

		sData.viewProjectionMatrix = viewProjection;
	}

	void InitializeLight()
	{
		GW::MATH::GVECTORF lightDirection = { -1.0f, -1.0f, 2.0f, 0.0f };
		GW::MATH::GVECTORF normalizedLightDirection;

		GW::MATH::GVector::NormalizeF(lightDirection, normalizedLightDirection);

		sData.sunDirection = normalizedLightDirection;
		sData.sunColor = { 0.9f, 0.9f, 1.0f, 1.0f };
		sData.sunAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };
	}

public:

	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();

		SetUpPipeline(curHandles);

		D3D11_MAPPED_SUBRESOURCE mappedResource;

		curHandles.context->Map(sceneConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &sData, sizeof(SceneData));
		curHandles.context->Unmap(sceneConstantBuffer.Get(), 0);

		gameLevel.RenderLevel(curHandles, mappedResource);

		ReleasePipelineHandles(curHandles);
	}

	//void InitializeGraphics()
	//{
	//	ID3D11Device* creator;

	//	d3d.GetDevice((void**)&creator);

	//	InitializeMeshSceneBuffers(creator);

	//	std::cout << "InitializedGraphics";

	//	// free temporary handle
	//	creator->Release();
	//}

	void Unload()
	{
		meshConstantBuffer.Reset();
		sceneConstantBuffer.Reset();
	}

	void UpdateCamera()
	{
		static std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - lastTime);
		lastTime = currentTime;

		const float cameraSpeed = 10.0f; // Camera Speed
		const float mouseSensitivity = 0.001f; // Mouse Sensitivity
		float totalYChange = 0.0f; // Vertical
		float totalZChange = 0.0f; // Forward - Backward
		float totalXChange = 0.0f; // Left - Right
		float totalPitch = 0.0f; // Pitch
		float totalYaw = 0.0f; // Yaw

		// Vertical
		float spaceKeyState = 0.0f, leftShiftKeyState = 0.0f;
		gInput.GetState(G_KEY_SPACE, spaceKeyState);
		gInput.GetState(G_KEY_LEFTSHIFT, leftShiftKeyState);
		totalYChange += spaceKeyState;
		totalYChange -= leftShiftKeyState;

		// Forward - Backward
		float wKeyState = 0.0f, sKeyState = 0.0f;
		gInput.GetState(G_KEY_W, wKeyState);
		gInput.GetState(G_KEY_S, sKeyState);
		totalZChange += wKeyState;
		totalZChange -= sKeyState;

		// Left - Right
		float aKeyState = 0.0f, dKeyState = 0.0f;
		gInput.GetState(G_KEY_A, aKeyState);
		gInput.GetState(G_KEY_D, dKeyState);
		totalXChange += dKeyState;
		totalXChange -= aKeyState;

		// Mouse Sens Rotation
		float mouseXDelta = 0.0f;
		float mouseYDelta = 0.0f;
		GW::GReturn isMovingMouse = gInput.GetMouseDelta(mouseXDelta, mouseYDelta);
		if (isMovingMouse == GW::GReturn::REDUNDANT)
		{
			mouseXDelta = 0.0f;
			mouseYDelta = 0.0f;
		}
		/*else
		{
			std::cout << "Mouse X Delta: " << mouseXDelta << std::endl;
			std::cout << "Mouse Y Delta: " << mouseYDelta << std::endl;
		}*/
		totalPitch += mouseYDelta * mouseSensitivity;
		totalYaw += mouseXDelta * mouseSensitivity;

		float perFrameSpeed = cameraSpeed * deltaTime.count();

		GW::MATH::GMATRIXF cameraWorldMatrix;
		matrixProxy.InverseF(view, cameraWorldMatrix);

		GW::MATH::GVECTORF localMovementVector = { totalXChange * perFrameSpeed, 0.0f, totalZChange * perFrameSpeed, 0.0f };
		matrixProxy.TranslateLocalF(cameraWorldMatrix, localMovementVector, cameraWorldMatrix);

		GW::MATH::GVECTORF globalMovementVector = { 0.0f, totalYChange * perFrameSpeed, 0.0f, 0.0f };
		matrixProxy.TranslateGlobalF(cameraWorldMatrix, globalMovementVector, cameraWorldMatrix);

		matrixProxy.RotateXLocalF(cameraWorldMatrix, totalPitch, cameraWorldMatrix);
		matrixProxy.RotateYGlobalF(cameraWorldMatrix, totalYaw, cameraWorldMatrix);

		matrixProxy.InverseF(cameraWorldMatrix, view);

		matrixProxy.MultiplyMatrixF(view, projection, viewProjection);

		sData.viewProjectionMatrix = viewProjection;
	}

private:

	PipelineHandles GetCurrentPipelineHandles()
	{
		PipelineHandles retval;
		d3d.GetImmediateContext((void**)&retval.context);
		d3d.GetRenderTargetView((void**)&retval.targetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
		GCOLLISION_H
	}

	void SetUpPipeline(PipelineHandles handles)
	{
		SetRenderTargets(handles);
		//SetVertexIndexBuffers(handles);
		//SetShaders(handles);

		handles.context->VSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());

		handles.context->PSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());

		//handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}

public:
	~RenderManager()
	{
		gameLevel.UnloadLevel();
	}
};
