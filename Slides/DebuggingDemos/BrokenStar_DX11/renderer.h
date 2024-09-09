// minimalistic code to draw a single triangle, this is not part of the API.
// required for compiling shaders on the fly, consider pre-compiling instead
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

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
	GW::GRAPHICS::GDirectX11Surface d3d;

	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;

		InitializeGraphics();
	}

private:
	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		// Raw array of floats is very hard to read, ideally we would use a vertex structure here instead.
		InitializeVertexBuffer(creator);
		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		std::vector<float> verts;
		BuildGoldenStar(verts);
		CreateVertexBuffer(&verts, sizeof(verts), creator);
	}

	void BuildGoldenStar(std::vector<float>& toFill)
	{
		// Build a golden star (to keep things simple you can assume this loop is correct, debug everything else though!)
		for (int i = 1, j = 0; i <= 360; i += 36, ++j) {
			toFill.push_back(sinf(G_DEGREE_TO_RADIAN(i)) * ((j % 2) ? 1.0f : 0.5f)); // A x
			toFill.push_back(-cosf(G_DEGREE_TO_RADIAN(i)) * ((j % 2) ? 1.0f : 0.5f)); // A y
			toFill.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // A r
			toFill.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // A g
			toFill.push_back(0); // A b

			toFill.push_back(0); // C x
			toFill.push_back(0); // C y
			toFill.push_back(1); // C r
			toFill.push_back(1); // C g
			toFill.push_back(1); // C b

			toFill.push_back(sinf(G_DEGREE_TO_RADIAN(i + 36)) * (((j + 1) % 2) ? 1.0f : 0.5f)); // B x
			toFill.push_back(-cosf(G_DEGREE_TO_RADIAN(i + 36)) * (((j + 1) % 2) ? 1.0f : 0.5f)); // B y
			toFill.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // B r
			toFill.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // B g
			toFill.push_back(0); // B b
		}
	}

	void CreateVertexBuffer(void* data, unsigned int sizeInBytes, ID3D11Device* creator)
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
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

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
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

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
		D3D11_INPUT_ELEMENT_DESC format[2];
	
		format[0].SemanticName = "POSITION";
		format[0].SemanticIndex = 0;
		format[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		format[0].InputSlot = 0;
		format[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		format[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		format[0].InstanceDataStepRate = 0;

		format[1].SemanticName = "COLOR";
		format[1].SemanticIndex = 1;
		format[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		format[1].InputSlot = 0;
		format[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		format[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		format[1].InstanceDataStepRate = 0;

		creator->CreateInputLayout(format, ARRAYSIZE(format),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}


public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);
		curHandles.context->Draw(27, 0);
		ReleasePipelineHandles(curHandles);
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

		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		handles.context->IASetInputLayout(vertexFormat.Get());
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		const UINT strides[] = { sizeof(float) * 2 };
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
