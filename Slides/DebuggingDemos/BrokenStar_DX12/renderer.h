// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")
#include "d3dx12.h" // official helper file provided by microsoft

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}


// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX12Surface d3d;
	// what we need at a minimum to draw a triangle
	D3D12_VERTEX_BUFFER_VIEW					vertexView;
	Microsoft::WRL::ComPtr<ID3D12Resource>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>	rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>	pipeline;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX12Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		InitializeGraphics();
	}

private:
	void InitializeGraphics()
	{
		ID3D12Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);


		InitializeGraphicsPipeline(creator);
		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D12Device* creator)
	{
		// Raw array of floats is very hard to read, ideally we would use a vertex structure here instead.
		std::vector<float> verts;
		BuildGoldenStar(verts);

		CreateVertexBuffer(creator, sizeof(verts));
		WriteToVertexBuffer(&verts, sizeof(verts));
		InitializeVertexView(sizeof(float) * 3, sizeof(verts));
	}

	void BuildGoldenStar(std::vector<float>& verts)
	{
		// Build a golden star (to keep things simple you can assume this loop is correct, debug everything else though!)
		for (int i = 1, j = 0; i <= 360; i += 36, ++j) {
			verts.push_back(sinf(G_DEGREE_TO_RADIAN(i)) * ((j % 2) ? 1.0f : 0.5f)); // A x
			verts.push_back(-cosf(G_DEGREE_TO_RADIAN(i)) * ((j % 2) ? 1.0f : 0.5f)); // A y
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // A r
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // A g
			verts.push_back(0); // A b

			verts.push_back(0); // C x
			verts.push_back(0); // C y
			verts.push_back(1); // C r
			verts.push_back(1); // C g
			verts.push_back(1); // C b

			verts.push_back(sinf(G_DEGREE_TO_RADIAN(i + 36)) * (((j + 1) % 2) ? 1.0f : 0.5f)); // B x
			verts.push_back(-cosf(G_DEGREE_TO_RADIAN(i + 36)) * (((j + 1) % 2) ? 1.0f : 0.5f)); // B y
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // B r
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // B g
			verts.push_back(0); // B b
		}
	}

	void CreateVertexBuffer(ID3D12Device* creator, unsigned int sizeInBytes)
	{
		creator->CreateCommittedResource( // using UPLOAD heap for simplicity
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // DEFAULT recommend  
			D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer));
	}
	
	void WriteToVertexBuffer(void* dataToWrite, unsigned int sizeInBytes)
	{
		UINT8* transferMemoryLocation;
		vertexBuffer->Map(0, &CD3DX12_RANGE(0, 0),
			reinterpret_cast<void**>(&transferMemoryLocation));
		memcpy(transferMemoryLocation, dataToWrite, sizeInBytes);
		vertexBuffer->Unmap(0, nullptr);
	}

	void InitializeVertexView(unsigned int strideInBytes, unsigned int sizeInBytes)
	{
		vertexView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexView.StrideInBytes = strideInBytes;
		vertexView.SizeInBytes = sizeInBytes;
	}

	void InitializeGraphicsPipeline(ID3D12Device* creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);
		CreateRootSignature(creator);
		CreatePipelineState(vsBlob, psBlob, creator);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D12Device* creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_5_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (FAILED(compilationResult))
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D12Device* creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_5_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (FAILED(compilationResult))
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;
	}

	void CreateRootSignature(ID3D12Device* creator)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> signature, errors;
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;

		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &errors);
		creator->CreateRootSignature(0, signature->GetBufferPointer(),
			signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	}
	
	void CreatePipelineState(Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob, Microsoft::WRL::ComPtr<ID3DBlob>& psBlob, ID3D12Device* creator)
	{
		//create input layout
		D3D12_INPUT_ELEMENT_DESC formats[2];
		formats[0].SemanticName = "POSITION";
		formats[0].SemanticIndex = 0;
		formats[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		formats[0].InputSlot = 0;
		formats[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		formats[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		formats[0].InstanceDataStepRate = 0;
		
		formats[1].SemanticName = "COLOR";
		formats[1].SemanticIndex = 0;
		formats[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		formats[1].InputSlot = 0;
		formats[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		formats[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		formats[1].InstanceDataStepRate = 0;


		D3D12_GRAPHICS_PIPELINE_STATE_DESC psDesc;
		ZeroMemory(&psDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psDesc.InputLayout = { formats, ARRAYSIZE(formats) };
		psDesc.pRootSignature = rootSignature.Get();
		psDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
		psDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob.Get());
		psDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psDesc.SampleMask = UINT_MAX;
		psDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psDesc.NumRenderTargets = 1;
		psDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psDesc.SampleDesc.Count = 1;
		creator->CreateGraphicsPipelineState(&psDesc, IID_PPV_ARGS(&pipeline));
	}


public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);

		// now we can draw
		curHandles.commandList->DrawInstanced(27, 1, 0, 0);

		// release temp handles
		curHandles.commandList->Release();
	}

private:
	struct PipelineHandles
	{
		ID3D12GraphicsCommandList* commandList;
		D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView;
		D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView;
	};

	PipelineHandles GetCurrentPipelineHandles()
	{
		PipelineHandles retval;
		d3d.GetCommandList((void**)&retval.commandList);
		d3d.GetCurrentRenderTargetView((void**)&retval.renderTargetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencilView);
		return retval;
	}

	void SetUpPipeline(PipelineHandles handles)
	{
		handles.commandList->SetGraphicsRootSignature(rootSignature.Get());
		handles.commandList->OMSetRenderTargets(1, &handles.renderTargetView, FALSE, &handles.depthStencilView);
		handles.commandList->SetPipelineState(pipeline.Get());
		handles.commandList->IASetVertexBuffers(0, 1, &vertexView);
		handles.commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}


public:
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet
	}
};
