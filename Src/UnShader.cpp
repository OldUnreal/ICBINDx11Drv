#include "ICBINDx11Drv.h"

// Metallicafan212:	TODO! This should be moved out I think....
D3D11_INPUT_ELEMENT_DESC FBasicInLayout[] =
{
	"POSITION",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"COLOR",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
#if EXTRA_VERT_INFO
#if !COMPLEX_SURF_MANUAL_UVs
	"TEXCOORD",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		4, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		5, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		6, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		7, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		8, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
#else
	"TEXCOORD",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	"TEXCOORD",		2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0,
	//"TEXCOORD",		3, DXGI_FORMAT_R32G32_FLOAT,		1, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0,
#endif
#endif
};

// Metallicafan212:	Shader interface
void FD3DShader::Init()
{
	guard(FD3DShader::Init);

	UINT Flags = 0;//D3DCOMPILE_DEBUG;

	HRESULT hr = S_OK;

	// Metallicafan212:	Compile the shaders
	if (VertexFunc.Len())
	{
		TArray<BYTE>* ShaderBytes = ParentDevice->ShaderManager->GetShaderBytes(VertexFile, VertexFunc, ParentDevice->MaxVSLevel, GET_MACRO_PTR(Macros), Flags);

		// Metallicafan212:	Get it as a vertex shader
		hr = ParentDevice->m_D3DDevice->CreateVertexShader(ShaderBytes->GetData(), ShaderBytes->Num(), nullptr, &VertexShader);

		// Metallicafan212:	IDK, do something here?
		ParentDevice->ThrowIfFailed(hr);


		// Metallicafan212:	Now create the layout pointer
		if (VertexShader != nullptr)
		{
			hr = ParentDevice->m_D3DDevice->CreateInputLayout(InputDesc, InputCount, ShaderBytes->GetData(), ShaderBytes->Num(), &InputLayout);

			// Metallicafan212:	IDK, do something here?
			ParentDevice->ThrowIfFailed(hr);
		}
	}

	// Metallicafan212:	Now the pixel shader
	if (PixelFunc.Len())
	{
		TArray<BYTE>* ShaderBytes = ParentDevice->ShaderManager->GetShaderBytes(PixelFile, PixelFunc, ParentDevice->MaxPSLevel, GET_MACRO_PTR(Macros), Flags);

		// Metallicafan212:	Get it as a pixel shader
		hr = ParentDevice->m_D3DDevice->CreatePixelShader(ShaderBytes->GetData(), ShaderBytes->Num(), nullptr, &PixelShader);

		// Metallicafan212:	IDK, do something here?
		ParentDevice->ThrowIfFailed(hr);
	}

	// Metallicafan212:	Now the geometry shader
	if (ParentDevice->bUseGeoShaders && GeoFunc.Len())
	{
		TArray<BYTE>* ShaderBytes = ParentDevice->ShaderManager->GetShaderBytes(GeoFile, GeoFunc, ParentDevice->MaxGSLevel, GET_MACRO_PTR(Macros), Flags);

		// Metallicafan212:	Get it as a pixel shader
		hr = ParentDevice->m_D3DDevice->CreateGeometryShader(ShaderBytes->GetData(), ShaderBytes->Num(), nullptr, &GeoShader);

		// Metallicafan212:	IDK, do something here?
		ParentDevice->ThrowIfFailed(hr);
	}

	// Metallicafan212:	Allow child shaders to setup the constant buffer how they want
	//					TODO! The shared variables should be optimized so that they only upload once per frame/only when they change
	SetupConstantBuffer();

	unguard;
}

// Metallicafan212:	TODO! This should be re-evalled and the bound textures moved to another constant buffer
//					Probably causing a lot of issues having multiple, but I'll combine more of them together later
void FD3DShader::Bind(ID3D11DeviceContext* UseContext)
{
	guardSlow(FD3DShader::Bind);

	UBOOL bShaderIsUs = 1;

	if (ParentDevice->CurrentShader != this)
	{
		ParentDevice->EndBuffering();

		ParentDevice->CurrentShader = this;

		bShaderIsUs = 0;
	}

	// Metallicafan212:	Only do this if the current shader isn't ours!!!!!
	if (!bShaderIsUs)
	{
#if !DO_BUFFERED_DRAWS
		// Metallicafan212:	Setup this shader for rendering
		UseContext->VSSetShader(VertexShader, nullptr, 0);

		// Metallicafan212:	Bind the optional geometry shader
		UseContext->GSSetShader(GeoShader, nullptr, 0);

		UseContext->PSSetShader(PixelShader, nullptr, 0);

		UseContext->IASetInputLayout(InputLayout);
#else
		// Metallicafan212:	Set the shader
		//					TODO!!!!!! 
		ParentDevice->CurrentDraw->Shader		= this;
		ParentDevice->CurrentDraw->bSetShader	= 1;

#endif
	}

#if !DO_BUFFERED_DRAWS
	// Metallicafan212:	Map the matrix(s)
	D3D11_MAPPED_SUBRESOURCE Map;
	HRESULT hr = UseContext->Map(ShaderConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Map);

	// Metallicafan212:	IDK, do something here?
	ParentDevice->ThrowIfFailed(hr);

	// Metallicafan212:	Copy over this shader's constants
	//					The child shaders can choose to copy more vars to the memory buffer
	WriteConstantBuffer(Map.pData);

	// Metallicafan212:	Now unmap it
	UseContext->Unmap(ShaderConstantsBuffer, 0);

	// Metallicafan212:	Now finally set it as a resource
	if (VertexShader != nullptr)
		UseContext->VSSetConstantBuffers(FIRST_USER_CONSTBUFF, 1, &ShaderConstantsBuffer);

	if (GeoShader != nullptr)
		UseContext->GSSetConstantBuffers(FIRST_USER_CONSTBUFF, 1, &ShaderConstantsBuffer);

	if (PixelShader != nullptr)
		UseContext->PSSetConstantBuffers(FIRST_USER_CONSTBUFF, 1, &ShaderConstantsBuffer);

#else
	// Metallicafan212:	TODO!!!!!! Check for variable differences????
	ParentDevice->CurrentDraw->UserConstants.Empty();
	D3D11_BUFFER_DESC Desc;

	ShaderConstantsBuffer->GetDesc(&Desc);

	ParentDevice->CurrentDraw->UserConstants.Add(Desc.ByteWidth);

	// Metallicafan212:	Now write to it
	WriteConstantBuffer(&ParentDevice->CurrentDraw->UserConstants(0));

	ParentDevice->CurrentDraw->bSetUserConstants	= 1;
	ParentDevice->CurrentDraw->UserBuffer			= ShaderConstantsBuffer;
#endif

	unguardSlow;
}

// Metallicafan212:	Made this generic so we can copy vars into shaders
void FD3DShader::SetupConstantBuffer()
{
	guard(FD3DShader::SetupConstantBuffer);

	// Metallicafan212:	Matrix layout
	//					TODO! Ask the child shader for this?
	D3D11_BUFFER_DESC MatrixDesc = { sizeof(FShaderVarCommon), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, D3D11_CPU_ACCESS_WRITE, 0, 0 };

	HRESULT hr = ParentDevice->m_D3DDevice->CreateBuffer(&MatrixDesc, nullptr, &ShaderConstantsBuffer);

	// Metallicafan212:	IDK, do something here?
	ParentDevice->ThrowIfFailed(hr);

	unguard;
}

void FD3DShader::WriteConstantBuffer(void* InMem)
{
	guardSlow(FD3DShader::WriteConstantBuffer);

	// Metallicafan212:	Copy over
	FShaderVarCommon* MDef			= ((FShaderVarCommon*)InMem);

	// Metallicafan212:	Loop and tell the shader how many textures are bound
	for (INT i = 0; i < MAX_TEXTURES; i++)
	{
		MDef->BoundTextures[i] = (ParentDevice->BoundTextures[i].TexInfoHash != 0 || ParentDevice->BoundTextures[i].bIsRT);
	}

	unguardSlow;
}