#include "D3D11Drv.h"

// Metallicafan212:	TODO!
IMPLEMENT_CLASS(UD3D11RenderDevice);
IMPLEMENT_PACKAGE(D3D11Drv);

void UD3D11RenderDevice::SetupDevice()
{
	guard(UD3D11RenderDevice::SetupDevice);

	// Metallicafan212:	Release all the fonts
	for (TMap<FString, IDWriteTextFormat*>::TIterator It(FontMap); It; ++It)
	{
		It.Value()->Release();
	}
	FontMap.Empty();

	// Metallicafan212:	Cleanout the texture cache
	Flush(0);

	// Metallicafan212:	Cleanup the blend states
	for (TMap<QWORD, ID3D11BlendState*>::TIterator It(BlendMap); It; ++It)
	{
		It.Value()->Release();
	}
	BlendMap.Empty();

	// Metallicafan212:	Cleanup the sampler map
	FlushTextureSamplers();


	SAFE_RELEASE(m_D3DQuery);
	SAFE_RELEASE(m_D3DDebug);
	SAFE_RELEASE(m_D3DQueue);
	SAFE_RELEASE(m_D3DDevice);
	SAFE_RELEASE(m_D3DSwapChain);
	SAFE_RELEASE(VertexBuffer);
	SAFE_RELEASE(IndexBuffer);
	SAFE_RELEASE(m_D2DRT);
	SAFE_RELEASE(m_D2DFact);
	SAFE_RELEASE(m_D2DWriteFact);
	SAFE_RELEASE(m_DXGISurf);
	SAFE_RELEASE(BlankTexture);
	SAFE_RELEASE(BlankResourceView);
	SAFE_RELEASE(BlankSampler);
	SAFE_RELEASE(m_DefaultZState);
	SAFE_RELEASE(m_DefaultNoZState);
	//SAFE_RELEASE(m_DefaultRasterState);
	FlushRasterStates();

	CurrentRasterState = DXRS_MAX;


	// Metallicafan212:	TODO! Do we need to make shaders each time?
	if (FTileShader != nullptr)
	{
		delete FTileShader;
		FTileShader = nullptr;
	}

	// Metallicafan212:	Same here
	if (FGenShader != nullptr)
	{
		delete FGenShader;
		FGenShader = nullptr;
	}

	if (FMeshShader != nullptr)
	{
		delete FMeshShader;
		FMeshShader = nullptr;
	}

	if (FSurfShader != nullptr)
	{
		delete FSurfShader;
		FSurfShader = nullptr;
	}

	if (FLineShader != nullptr)
	{
		delete FLineShader;
		FLineShader = nullptr;
	}

#if USE_COMPUTE_SHADER
	if (FMshLghtCompShader != nullptr)
	{
		delete FMshLghtCompShader;
		FMshLghtCompShader = nullptr;
	}
#endif

	// Metallicafan212:	Init DX11
	//					We want to use feature level 11_1 for compute shaders
	D3D_FEATURE_LEVEL FLList[7] = 
	{ 
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	INT FLCount = ARRAY_COUNT(FLList);

	D3D_FEATURE_LEVEL* FLPtr = &FLList[0];

	// Metallicafan212:	Make it single threaded for more performance?
	UINT Flags =	D3D11_CREATE_DEVICE_BGRA_SUPPORT
				//|	D3D11_CREATE_DEVICE_SINGLETHREADED
				|	D3D11_CREATE_DEVICE_DEBUG
				;
	
	GLog->Logf(TEXT("DX11: Creating device with the maximum feature level"));

MAKE_DEVICE:
	HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, Flags, FLPtr, FLCount, D3D11_SDK_VERSION, &m_D3DDevice, &m_FeatureLevel, &m_D3DDeviceContext);

	// Metallicafan212:	Check if it failed due to the debug layer
	if (FAILED(hr) && Flags & D3D11_CREATE_DEVICE_DEBUG)
	{
		GLog->Logf(TEXT("DX11: Removing the debug layer from the device flags"));
		Flags &= ~D3D11_CREATE_DEVICE_DEBUG;

		goto MAKE_DEVICE;
	}
	else if (FAILED(hr))
	{
		// Metallicafan212:	Try a lower feature level???
		FLCount--;

		if (FLCount <= 0)
		{
			// Metallicafan212:	We failed, so throw us out
			ThrowIfFailed(hr);
		}

		// Metallicafan212:	Update the ptr
		FLPtr++;

		goto MAKE_DEVICE;
	}

	ThrowIfFailed(hr);

	// Metallicafan212:	Log the feature level
	TCHAR* FLStr = nullptr;

	switch (m_FeatureLevel)
	{
		case D3D_FEATURE_LEVEL_11_1:
		{
			FLStr = TEXT("11.1");
			break;
		}

		case D3D_FEATURE_LEVEL_11_0:
		{
			FLStr = TEXT("11.0");
			break;
		}

		case D3D_FEATURE_LEVEL_10_1:
		{
			FLStr = TEXT("10.1");
			break;
		}

		case D3D_FEATURE_LEVEL_10_0:
		{
			FLStr = TEXT("10.0");
			break;
		}

		case D3D_FEATURE_LEVEL_9_3:
		{
			FLStr = TEXT("9.3");
			break;
		}

		case D3D_FEATURE_LEVEL_9_2:
		{
			FLStr = TEXT("9.2");
			break;
		}

		case D3D_FEATURE_LEVEL_9_1:
		{
			FLStr = TEXT("9.1");
			break;
		}
	}

	GLog->Logf(TEXT("DX11: Using feature level %s"), FLStr);

	// Metallicafan212:	Make the query
	D3D11_QUERY_DESC qDesc = { D3D11_QUERY_EVENT, 0 };
	m_D3DDevice->CreateQuery(&qDesc, &m_D3DQuery);

	GLog->Logf(TEXT("DX11: Creating shaders"));

	// Metallicafan212:	Make the shader
	FTileShader			= new FD3DTileShader(this);

	FGenShader			= new FD3DGenericShader(this);

#if USE_COMPUTE_SHADER
	FMshLghtCompShader	= new FD3DLghtMshCompShader(this);
#endif

	FMeshShader			= new FD3DMeshShader(this);

	FSurfShader			= new FD3DSurfShader(this);

	FLineShader			= new FD3DLineShader(this);

	// Metallicafan212:	Setup the debug info
#if 1//_DEBUG

	if (Flags & D3D11_CREATE_DEVICE_DEBUG)
	{
		GLog->Logf(TEXT("DX11: Grabbing debug interface"));

		hr = m_D3DDevice->QueryInterface(__uuidof(ID3D11Debug), (void**)&m_D3DDebug);

		ThrowIfFailed(hr);

		//m_D3DQueue = (ID3D11InfoQueue*)m_D3DDebug;
		hr = m_D3DDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&m_D3DQueue);

		// Metallicafan212:	To catch issues, will be removed when the renderer... works...
		m_D3DQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
		m_D3DQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);

		D3D11_MESSAGE_ID hide[] =
		{
			D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
			// TODO: Add more message IDs here as needed.
		};
		D3D11_INFO_QUEUE_FILTER filter = {};
		filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
		filter.DenyList.pIDList = hide;
		m_D3DQueue->AddStorageFilterEntries(&filter);
	}
#endif

	// Metallicafan212:	Setup vertex buffers here
	m_VertexBuffSize			= sizeof(FD3DVert)	* VBUFF_SIZE;
	m_IndexBuffSize				= sizeof(_WORD)		* IBUFF_SIZE;
	D3D11_BUFFER_DESC VertDesc	= { m_VertexBuffSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_VERTEX_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL, 0 };
	D3D11_BUFFER_DESC IndexDesc = { m_IndexBuffSize, D3D11_USAGE_DYNAMIC, D3D11_BIND_INDEX_BUFFER, D3D11_CPU_ACCESS_WRITE, NULL, 0 };

	GLog->Logf(TEXT("DX11: Creating vertex buffer"));

	hr = m_D3DDevice->CreateBuffer(&VertDesc, nullptr, &VertexBuffer);

	ThrowIfFailed(hr);

	GLog->Logf(TEXT("DX11: Creating index buffer"));

	// Metallicafan212:	Index buffer
	hr = m_D3DDevice->CreateBuffer(&IndexDesc, nullptr, &IndexBuffer);

	ThrowIfFailed(hr);

	GLog->Logf(TEXT("DX11: Creating D2D1 factory1"));

	// Metallicafan212:	D2D manager
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, IID_PPV_ARGS(&m_D2DFact));

	ThrowIfFailed(hr);

	GLog->Logf(TEXT("DX11: Creating DWrite factory"));

	// Metallicafan212:	Text related factory
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory1),  (IUnknown**)&m_D2DWriteFact);

	ThrowIfFailed(hr);

	// Metallicafan212:	This has been changed to a map, so we can change the raster flags on the fly
	//					Since we can't just change individual functions, we have to use flags to choose the right raster...
	/*
	// Metallicafan212:	Make the default raster state (don't cull backfaces)
	D3D11_RASTERIZER_DESC RSDesc;
	appMemzero(&RSDesc, sizeof(RSDesc));

	RSDesc.CullMode					= D3D11_CULL_NONE;
	RSDesc.DepthBias				= 0.0f;
	RSDesc.FillMode					= D3D11_FILL_SOLID;
	RSDesc.DepthClipEnable			= TRUE;
	RSDesc.AntialiasedLineEnable	= TRUE;
	RSDesc.MultisampleEnable		= TRUE;

	GLog->Logf(TEXT("DX11: Creating default raster state"));

	hr = m_D3DDevice->CreateRasterizerState(&RSDesc, &m_DefaultRasterState);

	ThrowIfFailed(hr);
	*/

	SetRasterState(DXRS_Normal);

	// Metallicafan212:	Reset the shader state
	GlobalShaderVars = FGlobalShaderVars();

	unguard;
}

// Metallicafan212:	UGH, I'm a fucking idiot. You can't mix MSAA and non-MSAA rendering in DX11..... So I'm going to have to come up with some alternative method to render font tiles correctly
//					Per this page: https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_rasterizer_desc
//					"Note  For feature levels 9.1, 9.2, 9.3, and 10.0, if you set MultisampleEnable to FALSE, 
//					 the runtime renders all points, lines, and triangles without anti-aliasing even for render targets with a sample count greater than 1. 
//					 For feature levels 10.1 and higher, the setting of MultisampleEnable has no effect on points and triangles with regard to MSAA and impacts only the selection of the line-rendering algorithm"
void UD3D11RenderDevice::SetRasterState(DWORD State)
{
	guard(UD3D11RenderDevice::SetRasterState);

	// Metallicafan212:	See if the raster state differs
	//					TODO! Add more flags
	State &= (DXRS_Wireframe | DXRS_NoAA);

	// Metallicafan212:	Add on the extra raster flags
	State |= ExtraRasterFlags;

	if (State != CurrentRasterState)
	{
		// Metallicafan212:	End whatever rendering we're doing right now!
		EndBuffering();

		// Metallicafan212:	Find what needs to be added on to make it, if it doesn't exist yet
		ID3D11RasterizerState* m_s = RasterMap.FindRef(State);

		if (m_s == nullptr)
		{
			D3D11_RASTERIZER_DESC Desc;

			// Metallicafan212:	Default vars
			Desc.CullMode					= D3D11_CULL_NONE;
			Desc.DepthBias					= 0;
			Desc.DepthBiasClamp				= 0.0f;
			Desc.DepthClipEnable			= TRUE;
			Desc.FrontCounterClockwise		= FALSE;
			Desc.ScissorEnable				= FALSE;

			// Metallicafan212:	Now check the flags
			if (State & DXRS_Wireframe)
			{
				Desc.FillMode = D3D11_FILL_WIREFRAME;
			}
			else
			{
				Desc.FillMode = D3D11_FILL_SOLID;
			}

			if (State & DXRS_NoAA)
			{
				Desc.AntialiasedLineEnable	= FALSE;
				Desc.MultisampleEnable		= FALSE;
			}
			else
			{
				Desc.AntialiasedLineEnable	= TRUE;
				Desc.MultisampleEnable		= TRUE;
			}

			HRESULT hr = m_D3DDevice->CreateRasterizerState(&Desc, &m_s);

			ThrowIfFailed(hr);

			// Metallicafan212:	Now set it on the map
			RasterMap.Set(State, m_s);
		}

		// Metallicafan212:	Set it
		if(m_s != nullptr)
			m_D3DDeviceContext->RSSetState(m_s);

		CurrentRasterState = State;
	}

	unguard;
}

// Metallicafan212:	Generalized functions for render device stuff (Init, Lock, Unlock, etc.)
UBOOL UD3D11RenderDevice::Init(UViewport* InViewport, INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen)
{
	guard(UD3D11RenderDevice::Init);

	// Metallicafan212:	Init pointers
	FTileShader			= nullptr;
	FGenShader			= nullptr;
#if USE_COMPUTE_SHADER
	FMshLghtCompShader	= nullptr;
#endif
	m_D3DDevice			= nullptr;
	m_D3DSwapChain		= nullptr;
	m_D3DDebug			= nullptr;
	m_D3DQueue			= nullptr;
	m_D3DScreenRTV		= nullptr;
	m_D3DScreenDSV		= nullptr;
	depthStencil		= nullptr;
	m_D2DRT				= nullptr;
	m_D2DFact			= nullptr;
	m_DXGISurf			= nullptr;
	FMeshShader			= nullptr;
	FSurfShader			= nullptr;
	FLineShader			= nullptr;

	BlankTexture		= nullptr;
	BlankResourceView	= nullptr;
	BlankSampler		= nullptr;
	m_DefaultZState		= nullptr;
	m_DefaultNoZState	= nullptr;

	m_TextParams		= nullptr;
	//m_DefaultRasterState= nullptr;

	CurrentRasterState	= DXRS_MAX;

	bLastFullscreen = bFullscreen = 0;

	appMemzero(BoundTextures, sizeof(BoundTextures));

	CurrentPolyFlags	= 0;

	Viewport			= InViewport;

	// Metallicafan212:	Save the values
	//					We also have to clamp here since unreal could pass bad values (the editor opening, for example)
	SizeX		= Max(NewX, 2);
	SizeY		= Max(NewY, 2);

	bFullscreen	= Fullscreen;

	GLog->Logf(TEXT("DX11: Starting"));

	SetupDevice();

	// Metallicafan212:	Set res???
	SetRes(SizeX, SizeY, NewColorBytes, bFullscreen);

	// Metallicafan212:	Get all display modes
	//					Copied and modified from the DX9 driver (since I'm a lazy bastard)
	for (INT i = 0; ; i++)
	{
		DEVMODE Tmp;
		appMemzero(&Tmp, sizeof(Tmp));
		Tmp.dmSize = sizeof(Tmp);

		if (!EnumDisplaySettings(NULL, i, &Tmp))
		{
			break;
		}
		Modes.AddUniqueItem(FPlane(Tmp.dmPelsWidth, Tmp.dmPelsHeight, Tmp.dmBitsPerPel, Tmp.dmDisplayFrequency));
	}

	GLog->Logf(TEXT("DX11: Registering supported texture formats"));

	// Metallicafan212:	Assemble the supported texture types
	RegisterTextureFormat(TEXF_P8, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 4, &FD3DTexType::RawPitch, nullptr, P8ToRGBA);

	RegisterTextureFormat(TEXF_RGBA7, DXGI_FORMAT_B8G8R8A8_UNORM, 1, 4, &FD3DTexType::RawPitch, nullptr, RGBA7To8);
	
	// Metallicafan212:	Raw ARGB texture
	RegisterTextureFormat(TEXF_RGBA8, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 4, &FD3DTexType::RawPitch);

	// Metallicafan212:	These are all supported by DX11.1
	//					In the future, I will query for support (or use the DX feature level???)
	RegisterTextureFormat(TEXF_DXT1, DXGI_FORMAT_BC1_UNORM, 0, 8, &FD3DTexType::BlockCompressionPitch);

	RegisterTextureFormat(TEXF_DXT3, DXGI_FORMAT_BC2_UNORM, 0, 16, &FD3DTexType::BlockCompressionPitch);

	RegisterTextureFormat(TEXF_DXT5, DXGI_FORMAT_BC3_UNORM, 0, 16, &FD3DTexType::BlockCompressionPitch);

	RegisterTextureFormat(TEXF_BC4, DXGI_FORMAT_BC4_UNORM, 0, 8, &FD3DTexType::BlockCompressionPitch);

	RegisterTextureFormat(TEXF_BC5, DXGI_FORMAT_BC5_UNORM, 0, 16, &FD3DTexType::BlockCompressionPitch);

	RegisterTextureFormat(TEXF_BC6, DXGI_FORMAT_BC6H_UF16, 0, 16, &FD3DTexType::BlockCompressionPitch);

	RegisterTextureFormat(TEXF_BC7, DXGI_FORMAT_BC7_UNORM, 0, 16, &FD3DTexType::BlockCompressionPitch);

	return 1;

	unguard;
}

void UD3D11RenderDevice::SetupResources()
{
	guard(UD3D11RenderDevice::SetupResources);

	GLog->Logf(TEXT("DX11: Setting up resources"));

	// Metallicafan212:	Clamp the user options!
	ClampUserOptions();

	// Metallicafan212:	Reset our bound textures
	for (INT i = 0; i < MAX_TEXTURES; i++)
	{
		//SetTexture(0, nullptr, 0);
		m_D3DDeviceContext->PSSetShaderResources(i, 0, nullptr);
		m_D3DDeviceContext->PSSetSamplers(i, 0, nullptr);
	}

	// Metallicafan212:	Clear the RT textures
	ClearRTTextures();

	// Metallicafan212:	CATCH RT TEXTURES!!!!
	RestoreRenderTarget();

	// Metallicafan212:	Clear any set RT/DC
	m_D3DDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	m_D3DDeviceContext->Flush();
	m_D3DDeviceContext->ClearState();

	// Metallicafan212:	Reset the raster state
	CurrentRasterState = DXRS_MAX;

	//m_D3DDeviceContext->RSSetState(m_DefaultRasterState);
	SetRasterState(0);

	// Metallicafan212:	Reset the views we have now
	SAFE_RELEASE(m_BackBuffTex);
	SAFE_RELEASE(m_ScreenBuffTex);
	SAFE_RELEASE(m_D3DScreenRTV);
	SAFE_RELEASE(m_D3DScreenDSV);
	SAFE_RELEASE(depthStencil);
	SAFE_RELEASE(m_D2DRT);
	SAFE_RELEASE(m_DXGISurf);
	SAFE_RELEASE(BlankTexture);
	SAFE_RELEASE(BlankResourceView);
	SAFE_RELEASE(BlankSampler);
	SAFE_RELEASE(m_DefaultZState);
	SAFE_RELEASE(m_DefaultNoZState);

	SAFE_RELEASE(m_TextParams);

	// Metallicafan212:	Recreate the texture samplers
	FlushTextureSamplers();

	// Metallicafan212:	Now flush
	m_D3DDeviceContext->Flush();

	// Metallicafan212:	Create or resize the swap chain
	HRESULT hr = S_OK;

	if (m_D3DSwapChain == nullptr)
	{
	SetupSwap:

		GLog->Logf(TEXT("DX11: Setting up swap chain"));

		DXGI_SWAP_CHAIN_DESC Swap;
		appMemzero(&Swap, sizeof(DXGI_SWAP_CHAIN_DESC));

		// Metallicafan212:	Setup the swap chain info
		//					We need the DXGI device for all this
		//					This is all copied from a M$ sample, sue me
		//					https://github.com/walbourn/directx-vs-templates/blob/main/d3d11game_win32/Game.cpp
		IDXGIDevice1* dxgiDevice;
		hr = m_D3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);

		// Identify the physical adapter (GPU or card) this device is running on.
		IDXGIAdapter1* dxgiAdapter;
		hr = dxgiDevice->GetAdapter((IDXGIAdapter**)&dxgiAdapter);

		ThrowIfFailed(hr);

		// Metallicafan212:	Get the GPU info
		DXGI_ADAPTER_DESC1 AdDesc;

		dxgiAdapter->GetDesc1(&AdDesc);

		debugf(TEXT("Metallicafan212's farm-raised Directx 11 renderer"));
		debugf(TEXT("Version                 : %s"),	D3D_DRIVER_VERSION);
		debugf(TEXT("Compiled                : %s"),	COMPILED_AT);
		//debugf(TEXT("D3D adapter driver      : %s"), appFromAnsi(ident.Driver));
		debugf(TEXT("D3D adapter description : %s"),	AdDesc.Description);
		debugf(TEXT("D3D adapter VRam        : %dGB (%lluMB)"), appRound((AdDesc.DedicatedVideoMemory / 1073741824.0)), (AdDesc.DedicatedVideoMemory / 1048576));
		//debugf(TEXT("D3D adapter name        : %s"), appFromAnsi(ident.DeviceName));
		//debugf(TEXT("D3D adapter id          : 0x%04X:0x%04X"), ident.VendorId, ident.DeviceId);


		// Metallicafan212:	If to use the new Windows 10 modes. I only test if we're actually running on 10
		//					!GIsEditor is here because using the tearing mode does something fucky in DWM, changing the window in such a way that normal non-DX11 renderers can't draw to it
		//					I need to analyse and see what exactly it's modifying about the window and reverse that change
		bAllowTearing = (!GIsEditor && GWin10);

		if (bAllowTearing)
		{
			GLog->Logf(TEXT("DX11: Setting up swap chain with tearing support"));
		}

		// And obtain the factory object that created it.

		IDXGIFactory5* dxgiFactory5 = nullptr;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory5));

		IDXGIFactory2* dxgiFactory;

		if (FAILED(hr))
		{
			// Metallicafan212:	Get the DXGI factory2 only
		}
		else
		{
			// Metallicafan212:	See if it supports the effect
			BOOL bSupportsTearing = 0;

			hr = dxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bSupportsTearing, sizeof(BOOL));

			if (FAILED(hr) || !bSupportsTearing)
			{
				if (FAILED(hr))
				{
					GLog->Logf(TEXT("DX11: Device does not support tearing"));
				}
				bAllowTearing = 0;
			}

		}

		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

		ThrowIfFailed(hr);

		// Metallicafan212:	Describe the non-aa swap chain (MSAA is resolved in Unlock)
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width					= SizeX;
		swapChainDesc.Height				= SizeY;
		swapChainDesc.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.SampleDesc.Count		= 1;
		swapChainDesc.SampleDesc.Quality	= 0;
		swapChainDesc.BufferUsage			= DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
		swapChainDesc.BufferCount			= 2;
		//swapChainDesc.Scaling				= DXGI_SCALING_NONE;
		// Metallicafan212:	If we're on windows 10 or above, use the better DXGI mode
		swapChainDesc.SwapEffect			= (bAllowTearing ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD);
		swapChainDesc.Flags					|= (bAllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
		fsSwapChainDesc.Windowed			= !bFullscreen;//TRUE;

		GLog->Logf(TEXT("DX11: Creating swap chain for the window"));

		// Metallicafan212:	Create the swap chain now
		hr = dxgiFactory->CreateSwapChainForHwnd(
			m_D3DDevice,
			(HWND)Viewport->GetWindow(),
			&swapChainDesc,
			&fsSwapChainDesc,
			nullptr,
			&m_D3DSwapChain
		);

		ThrowIfFailed(hr);

		// Metallicafan212:	Make it stop messing with the window itself
		dxgiFactory->MakeWindowAssociation((HWND)Viewport->GetWindow(), DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

		// Metallicafan212:	Remove alt+enter since Unreal already does that
		//dxgiFactory->MakeWindowAssociation((HWND)Viewport->GetWindow(), DXGI_MWA_NO_ALT_ENTER);

		// Metallicafan212:	Release all the pointers
		dxgiFactory->Release();
		dxgiAdapter->Release();
		dxgiDevice->Release();

		SAFE_RELEASE(dxgiFactory5);
	}
	else
	{
		GLog->Logf(TEXT("DX11: Resizing swap chain"));

		if (bLastFullscreen != bFullscreen)
		{
			GLog->Logf(TEXT("DX11: Toggling fullscreen"));
			hr = m_D3DSwapChain->SetFullscreenState(bFullscreen, nullptr);

			ThrowIfFailed(hr);
		}

		// Metallicafan212:	Resize it
		hr = m_D3DSwapChain->ResizeBuffers(2, SizeX, SizeY, DXGI_FORMAT_B8G8R8A8_UNORM, (bAllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0));

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			// Metallicafan212:	TODO! Recreate the device!!!
			// RecreateDevice();
			SetupDevice();
			goto SetupSwap;
		}
		else if(FAILED(hr))
		{
			appErrorf(TEXT("Failed to resize buffers with %lu"), hr);
		}
	}

	bLastFullscreen = bFullscreen;

	GLog->Logf(TEXT("DX11: Setting up Render targets, Depth/Stencil, and Depth states"));

	// Metallicafan212:	Now obtain the back buffer
	hr = m_D3DSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_BackBuffTex));

	ThrowIfFailed(hr);

	// Metallicafan212:	Now create the MSAA target
	//					ClampUserOptions already checks what levels of MSAA are supported, and clamps to that
	CD3D11_TEXTURE2D_DESC RTMSAA = CD3D11_TEXTURE2D_DESC();
	RTMSAA.Width				= SizeX;
	RTMSAA.Height				= SizeY;
	RTMSAA.BindFlags			= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	RTMSAA.Format				= DXGI_FORMAT_B8G8R8A8_UNORM;
	RTMSAA.MipLevels			= 1;
	RTMSAA.SampleDesc.Count		= NumAASamples;
	RTMSAA.SampleDesc.Quality	= 0;//D3D11_STANDARD_MULTISAMPLE_PATTERN;//0;
	RTMSAA.ArraySize			= 1;

	hr = m_D3DDevice->CreateTexture2D(&RTMSAA, nullptr, &m_ScreenBuffTex);

	ThrowIfFailed(hr);


	// Metallicafan212:	Now create the views
	hr = m_D3DDevice->CreateRenderTargetView(m_ScreenBuffTex, nullptr, &m_D3DScreenRTV);

	ThrowIfFailed(hr);

	// Metallicafan212:	Make the depth and stencil buffer
	//					TODO! Possibly use a higher quality format????
	CD3D11_TEXTURE2D_DESC depthStencilDesc = CD3D11_TEXTURE2D_DESC();
	depthStencilDesc.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.ArraySize			= 1;
	depthStencilDesc.Width				= SizeX;
	depthStencilDesc.Height				= SizeY;
	depthStencilDesc.MipLevels			= 1;
	depthStencilDesc.SampleDesc.Count	= NumAASamples;
	depthStencilDesc.SampleDesc.Quality = 0;//D3D11_STANDARD_MULTISAMPLE_PATTERN;//0;
	depthStencilDesc.BindFlags			= D3D11_BIND_DEPTH_STENCIL;

	
	hr = m_D3DDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil);

	ThrowIfFailed(hr);

	hr = m_D3DDevice->CreateDepthStencilView(depthStencil, nullptr, &m_D3DScreenDSV);

	ThrowIfFailed(hr);

	// Metallicafan212:	Get the D2D render target
	hr = m_ScreenBuffTex->QueryInterface(IID_PPV_ARGS(&m_DXGISurf));//m_D3DSwapChain->GetBuffer(0, IID_PPV_ARGS(&m_DXGISurf));

	ThrowIfFailed(hr);

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

	hr = m_D2DFact->CreateDxgiSurfaceRenderTarget(m_DXGISurf, &props, &m_D2DRT);

	ThrowIfFailed(hr);

	// Metallicafan212:	Set the main surface
	m_CurrentD2DRT = m_D2DRT;

	/*
	// Metallicafan212:	IMPORTANT!!! If we have AA, turn off AA in D2D!
	if (NumAASamples > 1)
	{
		m_D2DRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		m_D2DRT->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
		
		// Metallicafan212:	Get the defaults
		IDWriteRenderingParams* Def = nullptr;
		m_D2DRT->GetTextRenderingParams(&Def);

		if (Def != nullptr)
		{
			// Metallicafan212:	Create the text rendering parameters from the defaults
			hr = m_D2DWriteFact->CreateCustomRenderingParams(Def->GetGamma(), Def->GetEnhancedContrast(), Def->GetClearTypeLevel(), Def->GetPixelGeometry(), DWRITE_RENDERING_MODE_GDI_NATURAL, &m_TextParams);

			ThrowIfFailed(hr);
		}
		else
		{
			// Metallicafan212:	Make our own....
			hr = m_D2DWriteFact->CreateCustomRenderingParams(1.0f, 0.0f, 0.0f, DWRITE_PIXEL_GEOMETRY_RGB, DWRITE_RENDERING_MODE_GDI_NATURAL, &m_TextParams);

			ThrowIfFailed(hr);
		}

		m_D2DRT->SetTextRenderingParams(m_TextParams);
	}
	*/

	// Metallicafan212:	Make a totally blank texture
	D3D11_TEXTURE2D_DESC Desc;
	appMemzero(&Desc, sizeof(Desc));

	Desc.Format				= DXGI_FORMAT_R8_UNORM;
	Desc.Width				= 2;
	Desc.Height				= 2;
	Desc.MipLevels			= 1;
	Desc.Usage				= D3D11_USAGE_DEFAULT;
	Desc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	Desc.ArraySize			= 1;
	Desc.CPUAccessFlags		= D3D11_CPU_ACCESS_WRITE;
	Desc.SampleDesc.Count	= 1;
	hr = m_D3DDevice->CreateTexture2D(&Desc, nullptr, &BlankTexture);

	ThrowIfFailed(hr);

	// Metallicafan212:	Now the resource view for that
	D3D11_SHADER_RESOURCE_VIEW_DESC vDesc;
	appMemzero(&vDesc, sizeof(vDesc));

	vDesc.ViewDimension				= D3D11_SRV_DIMENSION_TEXTURE2D;
	vDesc.Texture2D.MipLevels		= 1;
	vDesc.Texture2D.MostDetailedMip = 0;
	vDesc.BufferEx.FirstElement		= 0;
	vDesc.Format					= Desc.Format;

	hr = m_D3DDevice->CreateShaderResourceView(BlankTexture, &vDesc, &BlankResourceView);

	ThrowIfFailed(hr);

	// Metallicafan212:	Now make a sampler
	D3D11_SAMPLER_DESC SDesc;
	appMemzero(&SDesc, sizeof(SDesc));

	SDesc.Filter			= D3D11_FILTER_ANISOTROPIC;
	SDesc.AddressU			= D3D11_TEXTURE_ADDRESS_WRAP;
	SDesc.AddressV			= D3D11_TEXTURE_ADDRESS_WRAP;
	SDesc.AddressW			= D3D11_TEXTURE_ADDRESS_WRAP;
	SDesc.MinLOD			= -FLT_MAX;
	SDesc.MaxLOD			= FLT_MAX;
	SDesc.MaxAnisotropy		= NumAFSamples;
	SDesc.ComparisonFunc	= D3D11_COMPARISON_NEVER;

	hr = m_D3DDevice->CreateSamplerState(&SDesc, &BlankSampler);

	ThrowIfFailed(hr);

	m_CurrentBuff = BT_None;

	// Metallicafan212:	Reset our bound textures
	for (INT i = 0; i < MAX_TEXTURES; i++)
	{
		SetTexture(0, nullptr, 0);
	}

	// Metallicafan212:	Setup the depth stencil state
	//					From a MSDN page https://learn.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-depth-stencil
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable					= true;
	dsDesc.DepthWriteMask				= D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc					= D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable				= true;
	dsDesc.StencilReadMask				= 0xFF;
	dsDesc.StencilWriteMask				= 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc		= D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp		= D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp	= D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp		= D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	hr = m_D3DDevice->CreateDepthStencilState(&dsDesc, &m_DefaultZState);

	ThrowIfFailed(hr);

	// Metallicafan212:	Set it
	m_D3DDeviceContext->OMSetDepthStencilState(m_DefaultZState, 1);

	// Metallicafan212:	And now a version with no z writing
	dsDesc.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc						= D3D11_COMPARISON_LESS_EQUAL;
	hr = m_D3DDevice->CreateDepthStencilState(&dsDesc, &m_DefaultNoZState);
	ThrowIfFailed(hr);

	// Metallicafan212:	Set the index and vertex buffers now (since we don't swap them in and out)
	m_D3DDeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	UINT Stride = sizeof(FD3DVert);
	UINT Offset = 0;
	m_D3DDeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);

	// Metallicafan212:	Keep the lock version updated
	LastAASamples = NumAASamples;

	unguard;
}

UBOOL UD3D11RenderDevice::SetRes(INT NewX, INT NewY, INT NewColorBytes, UBOOL Fullscreen)
{
	guard(UD3D11RenderDevice::SetRes);

	// Metallicafan212:	Save the values
	//					We also have to clamp here since unreal could pass bad values (the editor opening, for example)
	SizeX			= Max(NewX, 2);
	SizeY			= Max(NewY, 2);

	bFullscreen		= Fullscreen;

	if (Viewport != nullptr)
	{
		Viewport->ResizeViewport(BLIT_HardwarePaint | BLIT_Direct3D, SizeX, SizeY, NewColorBytes);
	}

	// Metallicafan212:	Set the viewport now
	D3D11_VIEWPORT viewport = {0.0f, 0.0f, SizeX, SizeY, 0.f, 1.f };
	m_D3DDeviceContext->RSSetViewports(1, &viewport);

	// Metallicafan212:	Resetup resources that need to be sized
	SetupResources();

	return 1;

	unguard;
}

void UD3D11RenderDevice::Exit()
{
	guard(UD3D11RenderDevice::Exit);

	Flush(0);

	// Metallicafan212:	Clear the RT textures
	ClearRTTextures();

	// Metallicafan212:	TODO! When supporting textures, clear textures
	//					Also shaders lol
	delete FTileShader;

	delete FGenShader;

#if USE_COMPUTE_SHADER
	delete FMshLghtCompShader;
#endif

	delete FMeshShader;

	delete FSurfShader;

	delete FLineShader;

	// Metallicafan212:	Cleanup all the fonts
	for (TMap<FString, IDWriteTextFormat*>::TIterator It(FontMap); It; ++It)
	{
		It.Value()->Release();
	}

	FontMap.Empty();

	// Metallicafan212:	Cleanup the blend states
	for (TMap<QWORD, ID3D11BlendState*>::TIterator It(BlendMap); It; ++It)
	{
		It.Value()->Release();
	}
	BlendMap.Empty();

	// Metallicafan212:	Cleanup the sampler map
	FlushTextureSamplers();

	m_D3DSwapChain->SetFullscreenState(0, nullptr);

	// Metallicafan212:	Clear any set RT/DC
	if(m_D3DDeviceContext != nullptr)
		m_D3DDeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	// Metallicafan212:	Remove the swap chain early, so we can clear
	//					Also all the other resources
	SAFE_RELEASE(m_D3DSwapChain);

	SAFE_RELEASE(m_D3DQuery);

	SAFE_RELEASE(m_D2DFact);
	SAFE_RELEASE(m_D2DWriteFact);
	SAFE_RELEASE(m_D2DRT);
	SAFE_RELEASE(m_DXGISurf);
	SAFE_RELEASE(BlankTexture);
	SAFE_RELEASE(BlankResourceView);
	SAFE_RELEASE(BlankSampler);
	SAFE_RELEASE(m_DefaultZState);
	
	SAFE_RELEASE(m_DefaultNoZState);

	SAFE_RELEASE(m_TextParams);

	//SAFE_RELEASE(m_DefaultRasterState);
	FlushRasterStates();


	// Metallicafan212:	Reset the views we have now
	SAFE_RELEASE(m_BackBuffTex);
	SAFE_RELEASE(m_ScreenBuffTex);
	SAFE_RELEASE(m_D3DScreenRTV);
	SAFE_RELEASE(m_D3DScreenDSV);
	SAFE_RELEASE(depthStencil);

	// Metallicafan212:	Now flush
	m_D3DDeviceContext->Flush();
	m_D3DDeviceContext->ClearState();


	SAFE_RELEASE(m_D3DDeviceContext);
	SAFE_RELEASE(m_D3DDevice);

	unguard;
}

void UD3D11RenderDevice::Flush(UBOOL AllowPrecache)
{
	guard(UD3D11RenderDevice::Flush);

	// Metallicafan212:	Loop and flush out the textures
	//					TODO! There might be a more efficient way to do this!
	for (TMap<D3DCacheId, FD3DTexture>::TIterator It(TextureMap); It; ++It)
	{
		// Metallicafan212:	Release all this info
		SAFE_RELEASE(It.Value().m_View);
		SAFE_RELEASE(It.Value().m_Tex);
	}

	TextureMap.Empty();

	appMemzero(BoundTextures, sizeof(BoundTextures));

	unguard;
}

UBOOL UD3D11RenderDevice::Exec(const TCHAR* Cmd, FOutputDevice& Ar)
{
	guard(UD3D11RenderDevice::Exec);

	if (URenderDevice::Exec(Cmd, Ar))
	{
		return 1;
	}

	// Metallicafan212:	Directly copied from the DX9 driver
	if (ParseCommand(&Cmd, TEXT("GetRes")))
	{
		TArray<FPlane> Relevant;
		INT i;

		// Metallicafan212:	Optimization hint
		INT num = Modes.Num();

		for (i = 0; i < num; i++)
		{
			if (Modes(i).Z == (Viewport->ColorBytes * 8))
				if
					((Modes(i).X != 320 || Modes(i).Y != 200)
						&& (Modes(i).X != 640 || Modes(i).Y != 400))
					Relevant.AddUniqueItem(FPlane(Modes(i).X, Modes(i).Y, 0, 0));
		}
		appQsort(&Relevant(0), Relevant.Num(), sizeof(FPlane), (QSORT_COMPARE)CompareRes);
		FString Str;

		// Metallicafan212:	Optimization hint
		INT rel_num = Relevant.Num();

		for (i = 0; i < rel_num; i++)
		{
			Str += FString::Printf(TEXT("%ix%i "), (INT)Relevant(i).X, (INT)Relevant(i).Y);
		}
		// Metallicafan212:	This causes the HP2 UC to not add on 1920x1080
		Ar.Log(*Str);//.LeftChop(1));
		return 1;
	}

	return 0;

	unguard;
}

void UD3D11RenderDevice::Lock(FPlane InFlashScale, FPlane InFlashFog, FPlane ScreenClear, DWORD RenderLockFlags, BYTE* HitData, INT* HitSize)
{
	guard(UD3D11RenderDevice::Lock);

	// Metallicafan212:	Check if our lock flags changed
	if (LastAASamples != NumAASamples || LastAFSamples != NumAFSamples)
	{
		if (LastAASamples != NumAASamples)
		{
			// Metallicafan212:	Doing this will also flush the sampler state
			SetupResources();
		}
		else
		{
			FlushTextureSamplers();
		}
	}

	// Metallicafan212:	Check for wireframe
	if (Viewport->IsWire())
	{
		ExtraRasterFlags = DXRS_Wireframe;
	}
	else
	{
		ExtraRasterFlags = 0;
	}

	// Metallicafan212:	Only clear if we're in the editor
	if (GIsEditor || (RenderLockFlags & LOCKR_ClearScreen))
	{
		m_D3DDeviceContext->ClearRenderTargetView(m_D3DScreenRTV, &ScreenClear.X);
	}
	// Metallicafan212:	TODO! Only do this in the editor?
	m_D3DDeviceContext->ClearDepthStencilView(m_D3DScreenDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Metallicafan212:	Make sure we're always using the right RT
	m_D3DDeviceContext->OMSetRenderTargets(1, &m_D3DScreenRTV, m_D3DScreenDSV);
	BoundRT = nullptr;

	// Metallicafan212:	Hold onto the flash fog for future render
	FlashScale	= InFlashScale;
	FlashFog	= InFlashFog;

	// Metallicafan212:	Hold onto the hit related info
	m_HitData		= HitData;
	m_HitSize		= HitSize;
	m_HitCount		= 0;

	if (m_HitData != nullptr)
	{
		// Metallicafan212:	Reset the pixel hit state
		PixelHitInfo.AddItem(FPixHitInfo());
		PixelTopIndex = -1;

		m_HitBufSize	= *m_HitSize;

		// Metallicafan212:	Tell unreal there was no hits (so far)
		*m_HitSize		= 0;
	}

	// Metallicafan212:	TODO! Don't set this each lock???
	/*
	m_D3DDeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	UINT Stride = sizeof(FD3DVert);
	UINT Offset = 0;
	m_D3DDeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
	*/

	// Metallicafan212:	Setup the buffers
	LockVertexBuffer(0, 0);
	LockIndexBuffer(0, 0);
	UnlockVertexBuffer();
	UnlockIndexBuffer();

	// Metallicafan212:	Reset the buffered state
	EndBuffering();

	if (GlobalShaderVars.bDoDistanceFog || GlobalShaderVars.bFadeFogValues)
		TickDistanceFog();

	unguard;
}

void UD3D11RenderDevice::Unlock(UBOOL Blit)
{
	guard(UD3D11RenderDevice::Unlock);

	// Metallicafan212:	Reset the buffered state
	EndBuffering();

	// Metallicafan212:	Get the selection
	if (m_HitData != nullptr)
	{
		// Metallicafan212:	Check the hits now
		DetectPixelHit();

		// Metallicafan212:	Clear them now
		PixelHitInfo.Empty();

		//ClearPixelHits();
		*m_HitSize = m_HitCount;

		// Metallicafan212:	TODO! Add this as a debug option
		//Blit = 1;
	}

	// Metallicafan212:	TODO! Sample code

	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.

	if (Blit)
	{
		// Metallicafan212:	Copy to the screen
		if (NumAASamples > 1)
			m_D3DDeviceContext->ResolveSubresource(m_BackBuffTex, 0, m_ScreenBuffTex, 0, DXGI_FORMAT_B8G8R8A8_UNORM);
		else
			m_D3DDeviceContext->CopyResource(m_BackBuffTex, m_ScreenBuffTex);

		DXGI_PRESENT_PARAMETERS Parm{ 0, nullptr, nullptr, nullptr };
		HRESULT hr = m_D3DSwapChain->Present1(0, (bAllowTearing && !bFullscreen ? DXGI_PRESENT_ALLOW_TEARING : 0), &Parm);//m_D3DSwapChain->Present(0, 0);

		// Metallicafan212:	Check if DXGI needs a resize (alt+tab in fullscreen for example)
		if (hr == DXGI_ERROR_INVALID_CALL)
		{
			SetupResources();
		}
		else if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
		{
			SetupDevice();
			SetupResources();
		}
		else if(!SUCCEEDED(hr))//hr != DXGI_STATUS_OCCLUDED)
		{
			ThrowIfFailed(hr);
		}
	}

	//if (m_HitData != nullptr)
	//{
	//	appSleep(1.0f);
	//}

	unguard;
}

void UD3D11RenderDevice::ClearZ(FSceneNode* Frame)
{
	guard(UD3D11RenderDevice::ClearZ);

	EndBuffering();
	m_D3DDeviceContext->ClearDepthStencilView(m_D3DScreenDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	unguard;
}

void UD3D11RenderDevice::EndFlash()
{
	guard(UD3D11RenderDevice::EndFlash);

	// Metallicafan212:	Test for a very small, but not 0 float, since the code seems to keep running for some reason
	if ((1.0f - Min(FlashScale.W * 2.0f, 1.0f)) <= 0.0001f)
	{
		return;
	}

	// Metallicafan212:	Draw it as a tile, but using the generic shader
	if ((FlashScale != FPlane(0.5f, 0.5f, 0.5f, 0.5f)) || (FlashFog != FPlane(0.0f, 0.0f, 0.0f, 0.0f)))
	{
		EndBuffering();

		// Metallicafan212:	Order of operations, make sure the alpha rejection is set
		SetBlend(PF_Highlighted);

		FGenShader->Bind();
		
		SetTexture(0, nullptr, 0);

		FPlane Color = FPlane(FlashFog.X, FlashFog.Y, FlashFog.Z, 1.0f - Min(FlashScale.W * 2.0f, 1.0f));

		FLOAT Z		= 0.5f;
		FLOAT X		= 0.0f;
		FLOAT Y		= 0.0f;
		FLOAT XL	= m_sceneNodeX;
		FLOAT YL	= m_sceneNodeY;

		// Metallicafan212:	Now make 2 triangles
		//					I copied this all from the tile rendering, since I'm such a lazy fucking bastard
		FLOAT PX1	= X - (m_sceneNodeX * 0.5f);
		FLOAT PX2	= PX1 + XL;
		FLOAT PY1	= Y - (m_sceneNodeY * 0.5f);
		FLOAT PY2	= PY1 + YL;

		FLOAT RPX1	= m_RFX2 * PX1;
		FLOAT RPX2	= m_RFX2 * PX2;
		FLOAT RPY1	= m_RFY2 * PY1;
		FLOAT RPY2	= m_RFY2 * PY2;

		RPX1 *= Z;
		RPX2 *= Z;
		RPY1 *= Z;
		RPY2 *= Z;


		// Metallicafan212:	Disable depth lmao
		ID3D11DepthStencilState* CurState = nullptr;
		UINT Sten = 0;
		m_D3DDeviceContext->OMGetDepthStencilState(&CurState, &Sten);
		m_D3DDeviceContext->OMSetDepthStencilState(m_DefaultNoZState, 0);

		LockVertexBuffer(6 * sizeof(FD3DVert));

		// Metallicafan212:	Start buffering now
		StartBuffering(BT_ScreenFlash);

		m_VertexBuff[0].Color	= Color;
		m_VertexBuff[0].X		= RPX1;
		m_VertexBuff[0].Y		= RPY1;
		m_VertexBuff[0].Z		= Z;

		m_VertexBuff[1].Color	= Color;
		m_VertexBuff[1].X		= RPX2;
		m_VertexBuff[1].Y		= RPY1;
		m_VertexBuff[1].Z		= Z;

		m_VertexBuff[2].Color	= Color;
		m_VertexBuff[2].X		= RPX2;
		m_VertexBuff[2].Y		= RPY2;
		m_VertexBuff[2].Z		= Z;

		m_VertexBuff[3].Color	= Color;
		m_VertexBuff[3].X		= RPX1;
		m_VertexBuff[3].Y		= RPY1;
		m_VertexBuff[3].Z		= Z;

		m_VertexBuff[4].Color	= Color;
		m_VertexBuff[4].X		= RPX2;
		m_VertexBuff[4].Y		= RPY2;
		m_VertexBuff[4].Z		= Z;

		m_VertexBuff[5].Color	= Color;
		m_VertexBuff[5].X		= RPX1;
		m_VertexBuff[5].Y		= RPY2;
		m_VertexBuff[5].Z		= Z;

		UnlockVertexBuffer();

		m_D3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		AdvanceVertPos(6);

		// Metallicafan212:	Draw
		EndBuffering();

		// Metallicafan212:	Reset Z state
		m_D3DDeviceContext->OMSetDepthStencilState(CurState, Sten);
	}

	unguard;
}

void UD3D11RenderDevice::GetStats(TCHAR* Result)
{

}

void UD3D11RenderDevice::DrawStats(FSceneNode* Frame)
{

}

void UD3D11RenderDevice::SetSceneNode(FSceneNode* Frame)
{
	guard(UD3D11RenderDevice::SetSceneNode);

	// Metallicafan212:	End any buffering that was requested???
	EndBuffering();

	// Set the viewport.
	D3D11_VIEWPORT viewport = {Frame->XB, Frame->YB, 
		Frame->FX, Frame->FY, 0.f, 1.f };
	m_D3DDeviceContext->RSSetViewports(1, &viewport);

	SizeX		= Frame->X;
	SizeY		= Frame->Y;

	// Metallicafan212:	All of this is copied from the DX9 driver
	// Precompute stuff.
	FLOAT rcpFrameFX = 1.0f / Frame->FX;
	m_Aspect	= Frame->FY * rcpFrameFX;

	// Metallicafan212:	This is HP2 specific! Since I have a viewport FOV that is calcuated to be a hor+ FOV, so 90 @ 16x9 is 109
	m_RProjZ	= appTan(Viewport->FOVAngle * PI / 360.0);//Viewport->Actor->FovAngle * PI / 360.0);
	m_RFX2		= 2.0f * m_RProjZ * rcpFrameFX;
	m_RFY2		= 2.0f * m_RProjZ * rcpFrameFX;

	//Remember Frame->X and Frame->Y
	m_sceneNodeX = Frame->X;
	m_sceneNodeY = Frame->Y;

	SetProjectionStateNoCheck(0, 1);

	unguard;
}

// Metallicafan212:	Shamfully copied from the DX9 renderer
void UD3D11RenderDevice::SetProjectionStateNoCheck(UBOOL bRequestingNearRangeHack, UBOOL bForceUpdate)
{
	float left, right, bottom, top, zNear, zFar;
	float invRightMinusLeft, invTopMinusBottom, invNearMinusFar;

	if (m_nearZRangeHackProjectionActive != bRequestingNearRangeHack)
		EndBuffering();
	// Metallicafan212:	Check if it was set at least once!
	else if(!bForceUpdate && m_CurrentBuff != BT_None)
		return;

	//Save new Z range hack projection state
	m_nearZRangeHackProjectionActive = bRequestingNearRangeHack;

	//Set default zNearVal
	FLOAT zNearVal = 0.5f;

	FLOAT zScaleVal = 1.0f;
	if (bRequestingNearRangeHack)
	{
		zScaleVal = 0.125f;

		// Metallicafan212:	Attempt to fix some near-z clipping issues
		zNearVal = 0.5f;
	}

	left	= -m_RProjZ * zNearVal;
	right	= +m_RProjZ * zNearVal;
	bottom	= -m_Aspect * m_RProjZ * zNearVal;
	top		= +m_Aspect * m_RProjZ * zNearVal;
	zNear	= 1.0f		* zNearVal;

	//Set zFar
	zFar = 49152.0f;

	if (bRequestingNearRangeHack)
	{
		zFar *= zScaleVal;
	}

	invRightMinusLeft = 1.0f / (right - left);
	invTopMinusBottom = 1.0f / (top - bottom);
	invNearMinusFar = 1.0f / (zNear - zFar);

	Proj.m[0][0] = 2.0f * zNear * invRightMinusLeft;
	Proj.m[0][1] = 0.0f;
	Proj.m[0][2] = 0.0f;
	Proj.m[0][3] = 0.0f;

	Proj.m[1][0] = 0.0f;
	Proj.m[1][1] = 2.0f * zNear * invTopMinusBottom;
	Proj.m[1][2] = 0.0f;
	Proj.m[1][3] = 0.0f;

	Proj.m[2][0] = 1.0f / (FLOAT)m_sceneNodeX;
	Proj.m[2][1] = -1.0f / (FLOAT)m_sceneNodeY;
	Proj.m[2][2] = zScaleVal * (zFar * invNearMinusFar);
	Proj.m[2][3] = -1.0f;

	Proj.m[3][0] = 0.0f;
	Proj.m[3][1] = 0.0f;
	Proj.m[3][2] = zScaleVal * zScaleVal * (zNear * zFar * invNearMinusFar);
	Proj.m[3][3] = 0.0f;

	//Proj = DirectX::XMMatrixPerspectiveFovLH(m_RProjZ, ((FLOAT)m_sceneNodeX) / ((FLOAT)m_sceneNodeY), zScaleVal * zNear, zScaleVal * zFar);

	//Proj = DirectX::XMMatrixPerspectiveOffCenterLH(0.0f, m_sceneNodeX, m_sceneNodeY, 0.0f, zNear * zScaleVal, zFar * zScaleVal);//DirectX::XMMatrixPerspectiveFovLH(m_RProjZ, m_Aspect, zNear * zScaleVal, zFar * zScaleVal);
	// Metallicafan212:	Transpose???
	FLOAT Temp[4][4];
	Temp[0][0]	= Proj.m[0][0];
	Temp[0][1]	= -Proj.m[1][0];
	Temp[0][2]	= -Proj.m[2][0];
	Temp[0][3]	= Proj.m[3][0];
	Temp[1][0]	= Proj.m[0][1];
	Temp[1][1]	= -Proj.m[1][1];
	Temp[1][2]	= -Proj.m[2][1];
	Temp[1][3]	= Proj.m[3][1];
	Temp[2][0]	= Proj.m[0][2];
	Temp[2][1]	= -Proj.m[1][2];
	Temp[2][2]  = -Proj.m[2][2];
	Temp[2][3]  = Proj.m[3][2];
	Temp[3][0]  = Proj.m[0][3];
	Temp[3][1]  = -Proj.m[1][3];
	Temp[3][2]  = -Proj.m[2][3];
	Temp[3][3]  = Proj.m[3][3];

	// Metallicafan212:	Now back????
	appMemcpy(&Proj.m[0][0], &Temp[0][0], sizeof(FLOAT[4][4]));
}

void UD3D11RenderDevice::PrecacheTexture(FTextureInfo& Info, QWORD PolyFlags) 
{
	guard(UD3D11RenderDevice::PrecacheTexture);

	// Metallicafan212:	Upload it now
	CacheTextureInfo(Info, PolyFlags);

	unguard;
}