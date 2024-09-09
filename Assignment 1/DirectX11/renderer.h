#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib") //needed for runtime shader compilation. Consider compiling shaders before runtime 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 1C 
struct Vertex
{
	float x, y, z, w;
};

// TODO: Part 2B
//struct SHADER_VARS
//{
//	GW::MATH::GMATRIXF world;
//};

// TODO: Part 2G 
struct SHADER_VARS
{
	GW::MATH::GMATRIXF world;
	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection; // Added for Part 3A
	GW::MATH::GMATRIXF viewProjection; // Added for Part 3B
};

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

	// TODO: Part 2A 
	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMATRIXF temp;
	GW::MATH::GMatrix matrixProxy;

	// TODO: Part 2C 
	SHADER_VARS shaderVars;

	// TODO: Part 2D 
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

	// TODO: Part 2G
	GW::MATH::GMATRIXF viewMatrix;

	// TODO: Part 3A 
	GW::MATH::GMATRIXF projectionMatrix;
	GW::MATH::GMATRIXF viewProjectionMatrix; // Added for Part 3B

	// TODO: Part 3C
	std::vector<GW::MATH::GMATRIXF> worldMatrices;

	// TODO: Part 4A
	GW::INPUT::GInput gInput;
	GW::INPUT::GController gController;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;

		// TODO: Part 2A
		//InitializeWorldMatrix(); //<------ Added to InitializeWorldMatrices()

		// TODO: Part 2C 
		//shaderVars.world = worldMatrix; <------ Added to InitializeWorldMatrix()

		// TODO: Part 2G
		//InitializeViewMatrix();

		// TODO: Part 3A
		//InitializeProjectionMatrix();

		// TODO: Part 3B
		//InitializeViewProjectionMatrix();

		// TODO: Part 3C	
		InitializeWorldMatrices();
		InitializeViewProjectionMatrix();


		// TODO: Part 4A 
		gInput.Create(win);
		gController.Create();

		InitializeGraphics();
	}

private:
#pragma region Seperate Initializion of World, View, and Projection Matrices
	//void InitializeWorldMatrix()
	//{
	//	matrixProxy.Create();
	//	matrixProxy.IdentityF(worldMatrix);

	//	matrixProxy.RotateXGlobalF(worldMatrix, 90.0f * (G_PI_F / 180.0f), worldMatrix);

	//	GW::MATH::GVECTORF translationVector = { 0.0f, -0.5f, 0.0f, 0.0f };
	//	matrixProxy.TranslateGlobalF(worldMatrix, translationVector, worldMatrix);

	//	shaderVars.world = worldMatrix;
	//}

	//void InitializeViewMatrix()
	//{
	//	matrixProxy.IdentityF(viewMatrix);
	//	GW::MATH::GVECTORF cameraPosition = { 0.25f, -0.125f, -0.25f, 1.0f };
	//	GW::MATH::GVECTORF targetPosition = { 0.0f, -0.5f, 0.0f, 1.0f };
	//	GW::MATH::GVECTORF upDirection = { 0.0f, 1.0f, 0.0f, 0.0f };

	//	matrixProxy.LookAtLHF(cameraPosition, targetPosition, upDirection, viewMatrix);

	//	//shaderVars.view = viewMatrix;
	//}

	//void InitializeProjectionMatrix()
	//{
	//	float fov = 65.0f * (G_PI_F / 180.0f);
	//	float aspectRatio;
	//	float nearPlane = 0.1f;
	//	float farPlane = 100.0f;
	//	d3d.GetAspectRatio(aspectRatio);

	//	matrixProxy.IdentityF(projectionMatrix);
	//	matrixProxy.ProjectionDirectXLHF(fov, aspectRatio, nearPlane, farPlane, projectionMatrix);
	//	matrixProxy.MultiplyMatrixF(viewMatrix, projectionMatrix, projectionMatrix);

	//	shaderVars.projection = projectionMatrix;
	//}
#pragma endregion
	void InitializeWorldMatrices()
	{
		worldMatrices.resize(6);
		matrixProxy.Create();

		// Floor (1)
		matrixProxy.IdentityF(worldMatrix);
		matrixProxy.RotateXGlobalF(worldMatrix, 90.0f * (G_PI_F / 180.0f), worldMatrix);
		GW::MATH::GVECTORF translationVector = { 0.0f, -0.5f, 0.0f, 0.0f };
		matrixProxy.TranslateGlobalF(worldMatrix, translationVector, worldMatrix);

		worldMatrices[0] = worldMatrix;

		// Left Wall (2)
		matrixProxy.IdentityF(worldMatrices[1]);
		matrixProxy.RotateYGlobalF(worldMatrices[1], 90.0f * (G_PI_F / 180.0f), worldMatrices[1]);
		GW::MATH::GVECTORF translationVectorLWall = { -0.5f, 0.0f, 0.0f, 0.0f };
		matrixProxy.TranslateGlobalF(worldMatrices[1], translationVectorLWall, worldMatrices[1]);

		// Right Wall (3)
		matrixProxy.IdentityF(worldMatrices[2]);
		matrixProxy.RotateYGlobalF(worldMatrices[2], 90.0f * (G_PI_F * 180.0f), worldMatrices[2]);
		GW::MATH::GVECTORF translationVectorRWall = { 0.0f, 0.0f, 0.5f, 0.0f };
		matrixProxy.TranslateGlobalF(worldMatrices[2], translationVectorRWall, worldMatrices[2]);

		// Back Left Wall (4)
		matrixProxy.IdentityF(worldMatrices[3]);
		matrixProxy.RotateXGlobalF(worldMatrices[3], 90.0f * (G_PI_F * 180.0f), worldMatrices[3]);
		GW::MATH::GVECTORF translationVectorBLWall = { 0.0f, 0.0f, -0.5f, 0.0f };
		matrixProxy.TranslateGlobalF(worldMatrices[3], translationVectorBLWall, worldMatrices[3]);

		// Back Right Wall (5)
		matrixProxy.IdentityF(worldMatrices[4]);
		matrixProxy.RotateYGlobalF(worldMatrices[4], 90.0f * (G_PI_F / 180.0f), worldMatrices[4]);
		GW::MATH::GVECTORF translationVectorBRWall = { 0.5f, 0.0f, 0.0f, 0.0f };
		matrixProxy.TranslateGlobalF(worldMatrices[4], translationVectorBRWall, worldMatrices[4]);

		// Ceiling (6)
		matrixProxy.IdentityF(worldMatrices[5]);
		matrixProxy.RotateXGlobalF(worldMatrices[5], 90.0f * (G_PI_F / 180.0f), worldMatrices[5]);
		GW::MATH::GVECTORF translationVectorCeiling = { 0.0f, 0.5f, 0.0f, 0.0f };
		matrixProxy.TranslateGlobalF(worldMatrices[5], translationVectorCeiling, worldMatrices[5]);

		shaderVars.world = worldMatrices[0];
	}

	void InitializeViewProjectionMatrix()
	{
		matrixProxy.IdentityF(viewMatrix);
		GW::MATH::GVECTORF cameraPosition = { 0.25f, -0.125f, -0.25f, 1.0f };
		GW::MATH::GVECTORF targetPosition = { 0.0f, -0.5f, 0.0f, 1.0f };
		GW::MATH::GVECTORF upDirection = { 0.0f, 1.0f, 0.0f, 0.0f };

		matrixProxy.LookAtLHF(cameraPosition, targetPosition, upDirection, viewMatrix);

		float fovY = 65.0f * (G_PI_F / 180.0f);
		float aspectRatio;
		float nearPlane = 0.1f;
		float farPlane = 100.0f;
		d3d.GetAspectRatio(aspectRatio);

		matrixProxy.IdentityF(projectionMatrix);
		matrixProxy.ProjectionDirectXLHF(fovY, aspectRatio, nearPlane, farPlane, projectionMatrix);
		matrixProxy.MultiplyMatrixF(viewMatrix, projectionMatrix, viewProjectionMatrix);

		shaderVars.viewProjection = viewProjectionMatrix;
	}

	void CreateConstantBuffer(ID3D11Device* creator)
	{
		D3D11_SUBRESOURCE_DATA bData = { &shaderVars, 0, 0 };

		CD3D11_BUFFER_DESC bDesc(sizeof(SHADER_VARS), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &bData, constantBuffer.GetAddressOf());
	}

	//Constructor helper functions 
	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);

		//TODO: Part 2D 
		CreateConstantBuffer(creator);

		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void CreateGrid(Vertex* verts, int& vertexCount)
	{
		float gridSize = 0.5f;
		int gridLines = 26;
		float step = gridSize * 2 / (gridLines - 1);
		int index = 0;

		// Horizontal Lines
		for (int i = 0; i < gridLines; ++i)
		{
			verts[index++] = Vertex{ -gridSize, -gridSize + i * step, 0, 1 };
			verts[index++] = Vertex{ gridSize, -gridSize + i * step, 0, 1 };
		}

		// Vertical Lines
		for (int i = 0; i < gridLines; ++i)
		{
			verts[index++] = Vertex{ -gridSize + i * step, -gridSize, 0, 1 };
			verts[index++] = Vertex{ -gridSize + i * step, gridSize, 0, 1 };
		}

		vertexCount = index;
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1B 
		//float verts[] = {
		//	// Line 1
		//	0, 0.5f,
		//	0.5f, -0.5f,

		//	// Line 2
		//	0.5f, -0.5f,
		//	-0.5f, -0.5f,

		//	// Line 3
		//	-0.5f, -0.5f,
		//	0, 0.5f,
		//};

		// TODO: Part 1C 
		//Vertex verts[] = {
		//	// Line 1
		//	{0, 0.5f, 0, 1},
		//	{0.5f, -0.5f, 0, 1},

		//	// Line 2
		//	{0.5f, -0.5f, 0, 1},
		//	{-0.5f, -0.5f, 0, 1},

		//	// Line 3
		//	{-0.5f, -0.5f, 0, 1},
		//	{0, 0.5f, 0, 1},
		//};

		// TODO: Part 1D 
		const int numLines = 26;
		const int verticesPerLine = 2;
		const int totalVertices = numLines * verticesPerLine * 2;
		Vertex* verts = new Vertex[totalVertices];
		int vertexCount = 0;

		CreateGrid(verts, vertexCount);

		//float verts[] = {
		//	0,   0.5f,
		//	0.5f, -0.5f,
		//	-0.5f, -0.5f
		//};
		//CreateVertexBuffer(creator, &verts[0], sizeof(verts)); <----------- Original

		CreateVertexBuffer(creator, &verts[0], vertexCount * sizeof(Vertex));

		delete[] verts;
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
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
		// TODO: Part 1C 
		D3D11_INPUT_ELEMENT_DESC attributes[1];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // Updated
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);

		// TODO: Part 1B
		//curHandles.context->Draw(6, 0);

		// TODO: Part 1D
		int totalVertices = 26 * 4;
		//curHandles.context->Draw(totalVertices, 0);

		// TODO: Part 3D 
		for (size_t i = 0; i < worldMatrices.size(); ++i)
		{
			shaderVars.world = worldMatrices[i];

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &shaderVars, sizeof(SHADER_VARS));
			curHandles.context->Unmap(constantBuffer.Get(), 0);
			curHandles.context->Draw(totalVertices, 0);
		}
		//curHandles.context->Draw(3, 0); Original

		ReleasePipelineHandles(curHandles);
	}

	// TODO: Part 4B
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
		matrixProxy.InverseF(viewMatrix, cameraWorldMatrix);

		GW::MATH::GVECTORF localMovementVector = { totalXChange * perFrameSpeed, 0.0f, totalZChange * perFrameSpeed, 0.0f };
		matrixProxy.TranslateLocalF(cameraWorldMatrix, localMovementVector, cameraWorldMatrix);

		GW::MATH::GVECTORF globalMovementVector = { 0.0f, totalYChange * perFrameSpeed, 0.0f, 0.0f };
		matrixProxy.TranslateGlobalF(cameraWorldMatrix, globalMovementVector, cameraWorldMatrix);

		matrixProxy.RotateXLocalF(cameraWorldMatrix, totalPitch, cameraWorldMatrix);
		matrixProxy.RotateYGlobalF(cameraWorldMatrix, totalYaw, cameraWorldMatrix);

		matrixProxy.InverseF(cameraWorldMatrix, viewMatrix);

		matrixProxy.MultiplyMatrixF(viewMatrix, projectionMatrix, viewProjectionMatrix);

		shaderVars.viewProjection = viewProjectionMatrix;
	}

	// TODO: Part 4C <-\
	// TODO: Part 4D <--\
	// TODO: Part 4E <------------------------------------------------------------------- All added into UpdateCamera()
	// TODO: Part 4F <--/
	// TODO: Part 4G <-/

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};
	//Render helper functions
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

		//TODO: Part 2E 
		ID3D11Buffer* buffers[] = { constantBuffer.Get() };
		handles.context->VSSetConstantBuffers(0, 1, buffers);

		handles.context->IASetInputLayout(vertexFormat.Get());

		//handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); <- Original

		//TODO: Part 1B
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		// TODO: Part 1C 
		const UINT strides[] = { sizeof(Vertex) }; // Updated
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

public:
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet
	}
};
