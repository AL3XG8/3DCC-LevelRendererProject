#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#include "FSLogo.h"
#pragma comment(lib, "d3dcompiler.lib") 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 2B
struct MeshData
{
	GW::MATH::GMATRIXF worldMatrix;
	OBJ_ATTRIBUTES material;
};

//struct SceneData
//{
//	GW::MATH::GVECTORF sunDirection, sunColor;
//	GW::MATH::GMATRIXF viewProjectionMatrix;
//
//};

// TODO: Part 4E 
struct SceneData
{
	GW::MATH::GVECTORF sunDirection, sunColor, sunAmbient, cameraPos;
	GW::MATH::GMATRIXF viewProjectionMatrix;
};

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// Added for Part 1G
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// TODO: Part 2A
	GW::MATH::GMATRIXF world;
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;
	GW::MATH::GMATRIXF viewProjection;
	GW::MATH::GMATRIXF rotationMatrix;
	GW::MATH::GMatrix matrixProxy;

	// TODO: Part 2B
	MeshData mData;
	SceneData sData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;

	GW::INPUT::GInput gInput;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		// TODO: Part 2A
		InitializeWorldMatrix();
		InitializeViewProjectionMatrix();
		InitializeLight();

		gInput.Create(win);

		// TODO: Part 4E
		// Set In InitializeLight()

		//InitializeViewMatrix();
		//InitializeProjectionMatrix();

		// TODO: Part 2B
		// Added to the functions where they are needed.

		IntializeGraphics();
	}

	void UpdateRotationMatrix()
	{
		float rotationSpeed = 1.0f;
		float angleY = GetElapsedTime() * rotationSpeed;

		matrixProxy.RotateYGlobalF(world, angleY, rotationMatrix);
	}

	float GetElapsedTime()
	{
		static std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> deltaTime = currentTime - startTime;
		return deltaTime.count();
	}

private:

	void InitializeWorldMatrix()
	{
		matrixProxy.Create();
		matrixProxy.IdentityF(world);

		mData.worldMatrix = world;
	}

#pragma region Depricated View and Projection Matrices
	/*void InitializeViewMatrix()
	{
		matrixProxy.IdentityF(view);

		GW::MATH::GVECTORF cameraPosition = { 0.75f, 0.25f, -1.5f, 0.0f };
		GW::MATH::GVECTORF targetPosition = { 0.15f, 0.75f, 0.0f, 1.0f };
		GW::MATH::GVECTORF upDirection = { 0.0f, 1.0f, 0.0f, 0.0f };

		matrixProxy.LookAtLHF(cameraPosition, targetPosition, upDirection, view);

		sData.viewMatrix = view;
	}

	void InitializeProjectionMatrix()
	{
		matrixProxy.IdentityF(projection);

		float fovY = 65.0f * (G_PI_F / 180.0f);
		float aspectRatio;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		d3d.GetAspectRatio(aspectRatio);

		matrixProxy.ProjectionDirectXLHF(fovY, aspectRatio, nearPlane, farPlane, projection);

		sData.viewProjectionMatrix = projection;
	}*/
#pragma endregion

	void InitializeViewProjectionMatrix()
	{
		matrixProxy.IdentityF(view);
		GW::MATH::GVECTORF cameraPosition = { 0.75f, 0.25f, -1.5f, 0.0f };
		GW::MATH::GVECTORF targetPosition = { 0.15f, 0.75f, 0.0f, 1.0f };
		GW::MATH::GVECTORF upDirection = { 0.0f, 1.0f, 0.0f, 0.0f };
		matrixProxy.LookAtLHF(cameraPosition, targetPosition, upDirection, view);

		// Added for Part 4E
		sData.cameraPos = cameraPosition;

		float fovY = 65.0f * (G_PI_F / 180.0f);
		float aspectRatio;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		d3d.GetAspectRatio(aspectRatio);

		matrixProxy.IdentityF(viewProjection);
		matrixProxy.ProjectionDirectXLHF(fovY, aspectRatio, nearPlane, farPlane, projection);
		matrixProxy.MultiplyMatrixF(view, projection, viewProjection);

		sData.viewProjectionMatrix = viewProjection;
	}

	void InitializeLight()
	{
		GW::MATH::GVECTORF lightDirection = { -1.0f, -1.0f, 2.0f, 0.0f };
		sData.sunDirection = NormalizeVector(lightDirection);
		sData.sunColor = { 0.9f, 0.9f, 1.0f, 1.0f };

		// Added for Part 4E
		sData.sunAmbient = { 0.25f, 0.25f, 0.35f, 1.0f };
	}

	GW::MATH::GVECTORF NormalizeVector(GW::MATH::GVECTORF& vector)
	{
		float length = sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

		if (length != 0)
		{
			vector.x /= length;
			vector.y /= length;
			vector.z /= length;
		}

		return vector;
	}

	//constructor helper functions
	void IntializeGraphics()
	{
		ID3D11Device* creator;

		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);

		// TODO: Part 1G
		InitializeIndexBuffer(creator);

		// TODO: Part 2C 
		/*InitializeSceneConstantBuffer(creator);
		InitializeMeshConstantBuffer(creator);*/

		// Added For Part 3A
		InitializeMeshConstantBuffer(creator);
		InitializeSceneConstantBuffer(creator);

		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		/*float verts[] = { <---------------------------------- Original
			0,   0.5f,
			0.5f, -0.5f,
			-0.5f, -0.5f
		};*/

		// TODO: Part 1C
		unsigned int verticesSize = sizeof(FSLogo_vertices);

		CreateVertexBuffer(creator, FSLogo_vertices, verticesSize);
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	void InitializeIndexBuffer(ID3D11Device* creator)
	{
		unsigned int indicesSize = sizeof(FSLogo_indices);

		CreateIndexBuffer(creator, FSLogo_indices, indicesSize);
	}

	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA iData = { data, 0, 0 };
		CD3D11_BUFFER_DESC iDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&iDesc, &iData, indexBuffer.GetAddressOf());
	}

	void InitializeMeshConstantBuffer(ID3D11Device* creator)
	{
		unsigned int meshDataSize = sizeof(MeshData);

		CreateMeshConstantBuffer(creator, &mData, meshDataSize);
	}

	void CreateMeshConstantBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA iData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &iData, meshConstantBuffer.GetAddressOf());
	}

	void InitializeSceneConstantBuffer(ID3D11Device* creator)
	{
		unsigned int sceneDataSize = sizeof(SceneData);

		CreateSceneConstantBuffer(creator, &sData, sceneDataSize);
	}

	void CreateSceneConstantBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA iData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &iData, sceneConstantBuffer.GetAddressOf());
	}

	void InitializePipeline(ID3D11Device* creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);

		CreateVertexInputLayout(creator, vsBlob);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;
	}

	void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
	{
		/*D3D11_INPUT_ELEMENT_DESC attributes[1];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32_FLOAT; <-------------------------------------- Original
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;*/

		// TODO: Part 1E
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		attributes[1].SemanticName = "TEXCOORD";
		attributes[1].SemanticIndex = 0;
		attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[1].InputSlot = 0;
		attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[1].InstanceDataStepRate = 0;

		attributes[2].SemanticName = "NORMAL";
		attributes[2].SemanticIndex = 0;
		attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[2].InputSlot = 0;
		attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[2].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);

		// TODO: Part 1H 
		//unsigned int totalIndices = sizeof(FSLogo_indices) / sizeof(FSLogo_indices[0]);
		//curHandles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		//curHandles.context->DrawIndexed(totalIndices, 0, 0);

		// TODO: Part 3B
		//for (int i = 0; i < 2; ++i)
		//{
		//	mData.material = FSLogo_materials[FSLogo_meshes[i].materialIndex].attrib;

		//	// TODO: Part 3C 
		//	D3D11_MAPPED_SUBRESOURCE mappedResource;
		//	curHandles.context->Map(materialConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		//	memcpy(mappedResource.pData, &mData, sizeof(MeshData));
		//	curHandles.context->Unmap(materialConstantBuffer.Get(), 0);
		//	curHandles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, FSLogo_meshes[i].indexOffset * sizeof(unsigned int));
		//	curHandles.context->DrawIndexed(FSLogo_meshes[i].indexCount, 0, 0);
		//}	

		// TODO: Part 4D 
		UpdateRotationMatrix();

		mData.worldMatrix = world;
		UpdateAndDrawMesh(curHandles, 0);

		mData.worldMatrix = rotationMatrix;
		UpdateAndDrawMesh(curHandles, 1);

		//curHandles.context->Draw(3, 0); <--------------------- Original

		// TODO: Part 1D 
		//unsigned int totalVertices = sizeof(FSLogo_vertices) / sizeof(FSLogo_vertices[0]);
		//curHandles.context->Draw(totalVertices, 0);

		ReleasePipelineHandles(curHandles);
	}

	void UpdateCamera()
	{
		static std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<float> deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - lastTime);
		lastTime = currentTime;

		const float cameraSpeed = 0.3f; // Adjust this value as needed
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
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

	PipelineHandles GetCurrentPipelineHandles()
	{
		PipelineHandles retval;
		d3d.GetImmediateContext((void**)&retval.context);
		d3d.GetRenderTargetView((void**)&retval.targetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
	}

	void SetUpPipeline(PipelineHandles handles)
	{
		SetRenderTargets(handles);
		SetVertexBuffers(handles);
		SetShaders(handles);

		handles.context->PSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());

		handles.context->VSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());

		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		//const UINT strides[] = { sizeof(float) * 2 }; <----------------------------------- Original

		// TODO: Part 1E
		const UINT strides[] = { sizeof(float) * 9 };

		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}

	void UpdateAndDrawMesh(PipelineHandles handles, int meshIndex)
	{
		mData.material = FSLogo_materials[FSLogo_meshes[meshIndex].materialIndex].attrib;

		D3D11_MAPPED_SUBRESOURCE mappedResource;

		handles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &mData, sizeof(MeshData));
		handles.context->Unmap(meshConstantBuffer.Get(), 0);

		handles.context->Map(sceneConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(mappedResource.pData, &sData, sizeof(SceneData));
		handles.context->Unmap(sceneConstantBuffer.Get(), 0);

		handles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, FSLogo_meshes[meshIndex].indexOffset * sizeof(unsigned int));
		handles.context->DrawIndexed(FSLogo_meshes[meshIndex].indexCount, 0, 0);
	}

public:
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet 
	}
};
