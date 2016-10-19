#pragma once

enum Descriptors
{
	DXFont,
	Count
};


class DX12Manager
{
public:
	static bool Initialize(UINT _x, UINT  _y, UINT _sx, UINT _sy);
	static HWND GetWindow()	{ return m_hwnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};

class DX12Renderer
{
private:
	static const UINT FrameCount = 2;

	Microsoft::WRL::ComPtr<IDXGISwapChain3>								   m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device>								   m_device;
	Microsoft::WRL::ComPtr<ID3D12Resource>								   m_renderTargets[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>						   m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>							   m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12RootSignature>							   m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>						   m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12PipelineState>							   m_pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>					   m_commandList;
	Microsoft::WRL::ComPtr<ID3D12Resource>								   m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Fence>									   m_fence;

	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>> m_batch;
	std::unique_ptr<DirectX::BasicEffect>								   m_batchEffects;
	std::unique_ptr<DirectX::CommonStates>								   m_commonStates;
	std::unique_ptr<DirectX::SpriteBatch>								   m_spriteBatch;
	std::unique_ptr<DirectX::EffectFactory>								   m_fxFactory;
	std::unique_ptr<DirectX::GraphicsMemory>                               m_graphicsMemory;
	std::unique_ptr<DirectX::CommonStates>                                 m_states;
	std::unique_ptr<DirectX::DescriptorHeap>							   m_resourceDescriptors;
	std::unique_ptr<DirectX::SpriteFont>								   m_font;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;
	UINT m_frameIndex;
	UINT m_rtvDescriptorSize;

	virtual void Begin();
	virtual void End();

	void LoadPipeline();
	void LoadResources();
	void WaitFrame();

public:
	DX12Renderer(UINT _x, UINT _y, UINT _width, UINT _height, std::wstring name);

	void Init();
	void Render();
	void Destroy();

	Vector2 ScreenPos;
	Vector2 ScreenSize;

	int CurrentTick = 0, LastTick = 0, LastFPS = 0, FPS = 0;

	float GetScreenX()  { return ScreenPos.x; }
	float GetScreenY()  { return ScreenPos.y; }
	float GetScreenSX() { return ScreenSize.x; }
	float GetScreenSY() { return ScreenSize.y; }
};


extern DX12Renderer* DX12;