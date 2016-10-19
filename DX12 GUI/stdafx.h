#pragma once
#define WIN32_LEAN_AND_MEAN

#pragma comment (lib, "DXTK12.lib")
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "D3DCompiler.lib")
#pragma comment (lib, "dwmapi.lib")

#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <d3dx12.h>

#include <mutex>
#include <DbgHelp.h>
#include <string>
#include <vector>
#include <stdarg.h>
#include <CommonStates.h>
#include <DDSTextureLoader.h>
#include <Effects.h>
#include <GeometricPrimitive.h>
#include <ResourceIndexer.h>
#include <ResourceUploadBatch.h>
#include <DescriptorHeap.h>
#include <Model.h>
#include <PrimitiveBatch.h>
#include <ScreenGrab.h>
#include <SpriteBatch.h>
#include <SimpleMath.h>
#include <SpriteFont.h>
#include <VertexTypes.h>
#include <DDSTextureLoader.h>
#include <DXGI.h>
#include <D3D11.h>
#include <D3DCompiler.h>
#include <DWrite.h>
#include <directxcolors.h>
#include <dwmapi.h>
#include <SimpleMath.h>
#include <limits>

using namespace DirectX;
using namespace DirectX::SimpleMath;