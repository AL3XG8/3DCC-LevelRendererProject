// This is a sample of how to load a level in a object oriented fashion.
// Feel free to use this code as a base and tweak it for your needs.

// This reads .h2b files which are optimized binary .obj+.mtl files
#include "h2bParser.h"

struct Vertex
{
	float x, y, z, w;
};

struct MeshData
{
	GW::MATH::GMATRIXF worldMatrix;
	H2B::ATTRIBUTES material;
};

struct SceneData
{
	GW::MATH::GVECTORF sunDirection, sunColor, sunAmbient, cameraPos;
	GW::MATH::GMATRIXF viewProjectionMatrix;
};

struct PipelineHandles
{
	ID3D11DeviceContext* context;
	ID3D11RenderTargetView* targetView;
	ID3D11DepthStencilView* depthStencil;
};


Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer;
Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;

MeshData mData;
SceneData sData;

GW::MATH::GMATRIXF view;
GW::MATH::GMATRIXF projection;
GW::MATH::GMATRIXF viewProjection;
GW::MATH::GMatrix matrixProxy;

bool changedLevel = false;

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// class Model contains everyhting needed to draw a single 3D model
class Model {
public:
	// Name of the Model in the GameLevel (useful for debugging)
	std::string name;
	// Loads and stores CPU model data from .h2b file
	H2B::Parser cpuModel; // reads the .h2b format
	// Shader variables needed by this model. 
	GW::MATH::GMATRIXF world;// TODO: Add matrix/light/etc vars..

private:

	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;

	// TODO: API Rendering vars here (unique to this model)
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

	// Index Buffer
	// Pipeline/State Objects
	// Uniform/ShaderVariable Buffer
	// Vertex/Pixel Shaders

	//Constructor helper functions 
private:

	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexIndexBuffers(creator);

		InitializePipeline(creator);

		InitializeMeshSceneBuffers(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexIndexBuffers(ID3D11Device* creator)
	{
		CreateVertexBuffer(creator, cpuModel.vertices.data(), cpuModel.vertexCount * sizeof(H2B::VERTEX));
		CreateIndexBuffer(creator, cpuModel.indices.data(), cpuModel.indexCount * sizeof(unsigned int));
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	void CreateIndexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, indexBuffer.GetAddressOf());
	}

	void InitializeMeshSceneBuffers(ID3D11Device* creator)
	{
		CreateMeshBuffer(creator, &mData, sizeof(MeshData));
		CreateSceneBuffer(creator, &sData, sizeof(SceneData));
	}

	void CreateMeshBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA iData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &iData, meshConstantBuffer.GetAddressOf());
	}

	void CreateSceneBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA iData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &iData, sceneConstantBuffer.GetAddressOf());
	}

	void PrintCurrentWorkingDirectory()
	{
		DWORD size = MAX_PATH;
		std::vector<wchar_t> buffer(size);

		if (GetCurrentDirectoryW(size, buffer.data()) != 0)
		{
			// Use wcout for wide characters
			std::wcout << L"Current working directory: " << buffer.data() << std::endl;
		}
		else
		{
			std::wcout << L"Failed to get current working directory." << std::endl;
		}
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
		std::string vertexShaderSource;
		//PrintCurrentWorkingDirectory();
		if (!changedLevel)
		{
			vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");
		}
		else
		{
			vertexShaderSource = ReadFileIntoString("../../Shaders/VertexShader.hlsl");
		}

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
		std::string pixelShaderSource;

		if (!changedLevel)
		{
			pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");
		}
		else
		{
			pixelShaderSource = ReadFileIntoString("../../Shaders/PixelShader.hlsl");
		}

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
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POS";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		attributes[1].SemanticName = "UVW";
		attributes[1].SemanticIndex = 0;
		attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[1].InputSlot = 0;
		attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[1].InstanceDataStepRate = 0;

		attributes[2].SemanticName = "NRM";
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

	void SetVertexIndexBuffers(PipelineHandles& curHandles)
	{
		const UINT strides[] = { sizeof(H2B::VERTEX) };
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const vBuffs[] = { vertexBuffer.Get() };
		curHandles.context->IASetVertexBuffers(0, ARRAYSIZE(vBuffs), vBuffs, strides, offsets);

		curHandles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	}

	void SetShaders(PipelineHandles& curHandles)
	{
		curHandles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		curHandles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

public:

	inline void SetName(std::string modelName) {
		name = modelName;
	}
	inline void SetWorldMatrix(GW::MATH::GMATRIXF worldMatrix) {
		world = worldMatrix;
	}
	bool LoadModelDataFromDisk(const char* h2bPath) {
		// if this succeeds "cpuModel" should now contain all the model's info
		return cpuModel.Parse(h2bPath);
	}
	bool UploadModelData2GPU(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d) {
		// TODO: Use chosen API to upload this model's graphics data to GPU
		win = _win;
		d3d = _d3d;

		//matrixProxy.Create();
		//matrixProxy.IdentityF(world);
		mData.material = cpuModel.materials[0].attrib;
		mData.worldMatrix = world;

		InitializeGraphics();

		return true;
	}
	bool DrawModel(PipelineHandles& curHandles, D3D11_MAPPED_SUBRESOURCE& mappedResource) {
		//TODO: Use chosen API to setup the pipeline for this model and draw it
		SetVertexIndexBuffers(curHandles);
		SetShaders(curHandles);
		curHandles.context->IASetInputLayout(vertexFormat.Get());

		for (int meshIndex = 0; meshIndex < cpuModel.meshes.size(); ++meshIndex)
		{
			auto& mesh = cpuModel.meshes[meshIndex];
			auto& batch = cpuModel.batches[mesh.materialIndex];

			mData.worldMatrix = world;
			mData.material = cpuModel.materials[mesh.materialIndex].attrib;

			curHandles.context->Map(meshConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			memcpy(mappedResource.pData, &mData, sizeof(MeshData));
			curHandles.context->Unmap(meshConstantBuffer.Get(), 0);

			curHandles.context->DrawIndexed(batch.indexCount, batch.indexOffset, 0);
		}

		return true;
	}

	bool FreeResources()
	{
		if (vertexBuffer) vertexBuffer.Reset();
		if (indexBuffer) indexBuffer.Reset();
		if (vertexShader) vertexShader.Reset();
		if (pixelShader) pixelShader.Reset();
		if (vertexFormat) vertexFormat.Reset();
		if (meshConstantBuffer) meshConstantBuffer.Reset();
		if (sceneConstantBuffer) sceneConstantBuffer.Reset();

		return true;
	}
};


// * NOTE: *
// Unlike the DOP version, this class was not designed to reuse data in anyway or process it efficiently.
// You can find ways to make it more efficient by sharing pointers to resources and sorting the models.
// However, this is tricky to implement and can be prone to errors. (OOP data isolation becomes an issue)
// A huge positive is that everything you want to draw is totally self contained and easy to see/find.
// This means updating matricies, adding new objects & removing old ones from the world is a breeze. 
// You can even easily load brand new models from disk at run-time without much trouble.
// The last major downside is trying to do things like dynamic lights, shadows and sorted transparency. 
// Effects like these expect your model set to be processed/traversed in unique ways which can be awkward.   

// class Level_Objects is simply a list of all the Models currently used by the level
class Level_Objects {
	// store all our models
	// TODO: This could be a good spot for any global data like cameras or lights
public:
	std::list<Model> allObjectsInLevel;
	Model model;

	const std::list<Model>& GetAllObjectsInLevel() const
	{
		return allObjectsInLevel;
	}

	// Imports the default level txt format and creates a Model from each .h2b
	bool LoadLevel(const char* gameLevelPath,
		const char* h2bFolderPath,
		GW::SYSTEM::GLog log) {
		// What this does:
		// Parse GameLevel.txt 
		// For each model found in the file...
			// Create a new Model class on the stack.
				// Read matrix transform and add to this model.
				// Load all CPU rendering data for this model from .h2b
			// Move the newly found Model to our list of total models for the level 

		log.LogCategorized("EVENT", "LOADING GAME LEVEL [OBJECT ORIENTED]");
		log.LogCategorized("MESSAGE", "Begin Reading Game Level Text File.");

		UnloadLevel();// clear previous level data if there is any
		GW::SYSTEM::GFile file;
		file.Create();
		if (-file.OpenTextRead(gameLevelPath)) {
			log.LogCategorized(
				"ERROR", (std::string("Game level not found: ") + gameLevelPath).c_str());
			return false;
		}
		char linebuffer[1024];
		while (+file.ReadLine(linebuffer, 1024, '\n'))
		{
			// having to have this is a bug, need to have Read/ReadLine return failure at EOF
			if (linebuffer[0] == '\0')
				break;
			if (std::strcmp(linebuffer, "MESH") == 0)
			{
				Model newModel;
				file.ReadLine(linebuffer, 1024, '\n');
				log.LogCategorized("INFO", (std::string("Model Detected: ") + linebuffer).c_str());
				// create the model file name from this (strip the .001)
				newModel.SetName(linebuffer);
				std::string modelFile = linebuffer;
				modelFile = modelFile.substr(0, modelFile.find_last_of("."));
				modelFile += ".h2b";

				// now read the transform data as we will need that regardless
				GW::MATH::GMATRIXF transform;
				for (int i = 0; i < 4; ++i) {
					file.ReadLine(linebuffer, 1024, '\n');
					// read floats
					std::sscanf(linebuffer + 13, "%f, %f, %f, %f",
						&transform.data[0 + i * 4], &transform.data[1 + i * 4],
						&transform.data[2 + i * 4], &transform.data[3 + i * 4]);
				}
				std::string loc = "Location: X ";
				loc += std::to_string(transform.row4.x) + " Y " +
					std::to_string(transform.row4.y) + " Z " + std::to_string(transform.row4.z);
				log.LogCategorized("INFO", loc.c_str());

				// Add new model to list of all Models
				log.LogCategorized("MESSAGE", "Begin Importing .H2B File Data.");
				modelFile = std::string(h2bFolderPath) + "/" + modelFile;
				newModel.SetWorldMatrix(transform);
				// If we find and load it add it to the level
				if (newModel.LoadModelDataFromDisk(modelFile.c_str())) {
					// add to our level objects, we use std::move since Model::cpuModel is not copy safe.
					allObjectsInLevel.push_back(std::move(newModel));
					log.LogCategorized("INFO", (std::string("H2B Imported: ") + modelFile).c_str());
				}
				else {
					// notify user that a model file is missing but continue loading
					log.LogCategorized("ERROR",
						(std::string("H2B Not Found: ") + modelFile).c_str());
					log.LogCategorized("WARNING", "Loading will continue but model(s) are missing.");
				}
				log.LogCategorized("MESSAGE", "Importing of .H2B File Data Complete.");
			}
		}
		log.LogCategorized("MESSAGE", "Game Level File Reading Complete.");
		// level loaded into CPU ram
		log.LogCategorized("EVENT", "GAME LEVEL WAS LOADED TO CPU [OBJECT ORIENTED]");
		return true;
	}
	// Upload the CPU level to GPU
	void UploadLevelToGPU(GW::SYSTEM::GWindow& win, GW::GRAPHICS::GDirectX11Surface& d3d) {
		// iterate over each model and tell it to draw itself	

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

		GW::MATH::GVECTORF lightDirection = { -1.0f, -1.0f, 2.0f, 0.0f };
		GW::MATH::GVECTORF normalizedLightDirection;
		GW::MATH::GVector::NormalizeF(lightDirection, normalizedLightDirection);

		sData.sunDirection = normalizedLightDirection;
		sData.sunColor = { 0.9f, 0.9f, 1.0f, 1.0f };
		sData.sunAmbient = { 1.0f, 1.0f, 1.0f, 1.0f };

		for (auto& e : allObjectsInLevel) {
			e.UploadModelData2GPU(win, d3d);
		}
	}
	// Draws all objects in the level
	void RenderLevel(PipelineHandles& curHandles, D3D11_MAPPED_SUBRESOURCE& mappedResource)
	{

		for (Model& model : allObjectsInLevel)
		{
			model.DrawModel(curHandles, mappedResource);
		}
	}

	void FreeResources()
	{
		for (Model& model : allObjectsInLevel)
		{
			model.FreeResources();
		}
	}
	// used to wipe CPU & GPU level data between levels
	void UnloadLevel()
	{
		/*for (Model& model : allObjectsInLevel)
		{
			model.FreeResources();
		}*/

		allObjectsInLevel.clear();
	}
	// *THIS APPROACH COMBINES DATA & LOGIC* 
	// *WITH THIS APPROACH THE CURRENT RENDERER SHOULD BE JUST AN API MANAGER CLASS*
	// *ALL ACTUAL GPU LOADING AND RENDERING SHOULD BE HANDLED BY THE MODEL CLASS* 
	// For example: anything that is not a global API object should be encapsulated.
	void PrintAllObjectsInLevel(const std::list<Model>& models)
	{
		std::cout << "Number of models in level: " << models.size() << std::endl;

		for (const auto& model : models)
		{
			std::cout << "Model Name: " << model.name << std::endl;
		}
	}
};

