#include "stdafx.h"
#include "DX12.h"


HWND DX12Manager::m_hwnd = nullptr;
DX12Renderer* DX12 = nullptr;

inline void ThrowIfFailed(HRESULT hr);
void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);


void DX12Renderer::Render()
{
	Begin();

	//You should use functions to do this like, FillQuad, DrawText, DrawEmtpyBox etc.
	const float x = 100, y = 150, x1 = 50, y1 = 75;

	FXMVECTOR c1 = { x, y };
	FXMVECTOR c2 = { x + x1, y };
	FXMVECTOR c3 = { x + x1, y + y1 };
	FXMVECTOR c4 = { x, y + y1 };
	FXMVECTOR colora = { 0.f, 1.f, 0.f, 1.f };
	VertexPositionColor d1(c1, colora);
	VertexPositionColor d2(c2, colora);
	VertexPositionColor d3(c3, colora);
	VertexPositionColor d4(c4, colora);
	m_batch->DrawQuad(d1, d2, d3, d4);

	FXMVECTOR erer = { 0.f, 1.f, 0.f, 1.f };
	FXMVECTOR erer2 = { 0.f, 1.f, 0.f, 1.f };
	FXMVECTOR shadow = { 0.f, 0.f, 0.f, 0.75f };
	std::wstring _fps = std::to_wstring(FPS);

	m_font->DrawString(m_spriteBatch.get(), _fps.c_str(), XMFLOAT2(1.f, 1.f), shadow, 0.f, XMFLOAT2(0.f, 0.f), 0.2f);
	m_font->DrawString(m_spriteBatch.get(), _fps.c_str(), XMFLOAT2(0.f, 0.f), erer, 0.f, XMFLOAT2(0.f, 0.f), 0.2f);
	m_font->DrawString(m_spriteBatch.get(), L"DX12 Tool Kit Usage Test", XMFLOAT2(101.f, 41.f), shadow, 0.f, XMFLOAT2(0, 0), 0.4f);
	m_font->DrawString(m_spriteBatch.get(), L"DX12 Tool Kit Usage Test", XMFLOAT2(99.f, 41.f), shadow, 0.f, XMFLOAT2(0, 0), 0.4f);
	m_font->DrawString(m_spriteBatch.get(), L"DX12 Tool Kit Usage Test", XMFLOAT2(100.f, 40.f), erer2, 0.f, XMFLOAT2(0, 0), 0.4f);

	End();
}

LRESULT CALLBACK DX12Manager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		DX12->Render();
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool DX12Manager::Initialize(UINT _x, UINT  _y, UINT _sx, UINT _sy)
{
	DX12 = new DX12Renderer(_x, _y, _sx, _sy, L"DX12 GUI");

	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = NULL;
	windowClass.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(NULL));
	windowClass.hIconSm = LoadIcon(NULL, MAKEINTRESOURCE(NULL));
	windowClass.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "DX12Class";
	RegisterClassEx(&windowClass);

	m_hwnd = CreateWindowEx(WS_EX_TRANSPARENT, windowClass.lpszClassName, "DX12", WS_OVERLAPPEDWINDOW, _x, _y, _sx, _sy, nullptr, nullptr, nullptr, nullptr);
	/*SetLayeredWindowAttributes(m_hwnd, 0, 0.f, LWA_ALPHA);
	SetLayeredWindowAttributes(m_hwnd, 0, RGB(0, 0, 0), LWA_COLORKEY);

	const MARGINS Margin = { -1 };
	DwmExtendFrameIntoClientArea(m_hwnd, &Margin);*/

	if (!m_hwnd)
		return false;

	DX12->Init();

	ShowWindow(m_hwnd, SW_SHOW);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	DX12->Destroy();

	return true;
}

DX12Renderer::DX12Renderer(UINT _x, UINT _y, UINT _width, UINT _height, std::wstring name) : m_frameIndex(0), m_viewport(), m_scissorRect(), m_rtvDescriptorSize(0)
{
	ScreenPos = Vector2(_x, _y);
	ScreenSize = Vector2(_width, _height);
	m_viewport.Width = static_cast<float>(_width);
	m_viewport.Height = static_cast<float>(_height);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;

	m_scissorRect.right = static_cast<LONG>(_width);
	m_scissorRect.bottom = static_cast<LONG>(_height);
}

void DX12Renderer::Init()
{
	LoadPipeline();
	LoadResources();

	m_graphicsMemory = std::make_unique<GraphicsMemory>(m_device.Get());
	m_states = std::make_unique<CommonStates>(m_device.Get());
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(m_device.Get());

	m_resourceDescriptors = std::make_unique<DescriptorHeap>(m_device.Get(),
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
		Descriptors::Count);

	ResourceUploadBatch resourceUpload(m_device.Get());

	resourceUpload.Begin();

	RenderTargetState rtState(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT);

	SpriteBatchPipelineStateDescription pd(rtState);

	m_spriteBatch = std::make_unique<SpriteBatch>(m_device.Get(), resourceUpload, pd);

	RenderTargetState rtState2(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_D32_FLOAT);
	EffectPipelineStateDescription pd2(
		&VertexPositionColor::InputLayout,
		CommonStates::AlphaBlend,
		CommonStates::DepthDefault,
		CommonStates::CullNone,
		rtState2);

	m_batchEffects = std::make_unique<BasicEffect>(m_device.Get(), EffectFlags::VertexColor, pd2);

	m_font = std::make_unique<SpriteFont>(m_device.Get(), resourceUpload,
		L"verdana.spritefont",
		m_resourceDescriptors->GetCpuHandle(Descriptors::DXFont),
		m_resourceDescriptors->GetGpuHandle(Descriptors::DXFont));

	auto uploadResourcesFinished = resourceUpload.End(m_commandQueue.Get());

	Matrix Projection = XMMatrixOrthographicOffCenterRH(-ScreenSize.x / 2.f, ScreenSize.x / 2.f, ScreenSize.y / 2.f, -ScreenSize.y / 2.f, 0.0f, 1.f);
	Projection.Translation(Vector3(-1.f, 1.f, 0.f));
	m_batchEffects->SetProjection(Projection);
	m_batchEffects->SetWorld(XMMatrixIdentity());
	m_batchEffects->SetView(XMMatrixIdentity());

	D3D12_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D12_VIEWPORT));

	viewport.TopLeftX = m_viewport.TopLeftX;
	viewport.TopLeftY = m_viewport.TopLeftY;
	viewport.Width = m_viewport.Width;
	viewport.Height = m_viewport.Height;
	viewport.MinDepth = m_viewport.MinDepth;
	viewport.MaxDepth = m_viewport.MaxDepth;
	m_spriteBatch->SetViewport(viewport);
}

void DX12Renderer::LoadPipeline()
{
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
	GetHardwareAdapter(factory.Get(), &hardwareAdapter);

	ThrowIfFailed(D3D12CreateDevice(
		hardwareAdapter.Get(),
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&m_device)
	));

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = ScreenSize.x;
	swapChainDesc.Height = ScreenSize.y;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		DX12Manager::GetWindow(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	ThrowIfFailed(factory->MakeWindowAssociation(DX12Manager::GetWindow(), DXGI_MWA_NO_ALT_ENTER));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

	D3D12_RENDER_TARGET_BLEND_DESC AlphaBlending = {};
	AlphaBlending.BlendEnable = true;

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT n = 0; n < FrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}


	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

void DX12Renderer::LoadResources()
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

	const static UINT compileFlags = 0;

	ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
	ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
	ThrowIfFailed(m_commandList->Close());
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	WaitFrame();
}

void DX12Renderer::Begin()
{
	ThrowIfFailed(m_commandAllocator->Reset());

	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap() };

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, DirectX::Colors::White, 0, nullptr); //background clean color
	m_commandList->SetDescriptorHeaps(_countof(heaps), heaps);
	m_batchEffects->Apply(m_commandList.Get());
	m_batch->Begin(m_commandList.Get());
	m_spriteBatch->Begin(m_commandList.Get(), SpriteSortMode_Deferred);

	//FPS UPDATER
	if (CurrentTick - LastTick >= 1000)
	{
		FPS = LastFPS;
		LastFPS = 0;
		LastTick = CurrentTick;
	}
	LastFPS++;
	CurrentTick = GetTickCount();
}

void DX12Renderer::End()
{
	m_batch->End();
	m_spriteBatch->End();

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_graphicsMemory->Commit(m_commandQueue.Get());
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, CommandListCast(m_commandList.GetAddressOf()));
	ThrowIfFailed(m_swapChain->Present(1, 0));
	WaitFrame();
}

void DX12Renderer::WaitFrame()  //NOT GOOD PRACTICE!
{
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DX12Renderer::Destroy()
{
	WaitFrame();
	CloseHandle(m_fenceEvent);
}


inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
		throw std::exception();
}

void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
{
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
			break;
	}

	*ppAdapter = adapter.Detach();
}
