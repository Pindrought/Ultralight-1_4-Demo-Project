#pragma once
#define NOMINMAX
//These are 3 helper classes that I don't feel like including in every single source file so i'm just putting it as part of the precompiled header.
#include "StringConverter.h"
#include "ErrorHandler.h"
#include "DirectoryHelper.h"
#include "SimpleMath.h" //Also SimpleMath


#ifdef _DEBUG
#define MYLOGGER_LOG_INFO 1
#endif
#include "MyLogger.h" 


#include <fstream>
#include <chrono>
#include <string>
#include <array>
#include <atomic>
#include <vector>
#include <queue>
#include <set>
#include <tuple>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <memory>
#include <thread>
#include <comdef.h>
#include <Windows.h>

#include <dcomp.h>
#pragma comment(lib, "dcomp")

#include <dwrite_3.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <wrl/client.h>

#include <windowsx.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <versionhelpers.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Ultralight/Ultralight.h>
#include <Ultralight/MouseEvent.h>
#include <Ultralight/KeyEvent.h>
#include <Ultralight/platform/FontLoader.h>
#include <Ultralight/platform/Clipboard.h>
#include <JavaScriptCore/JSRetainPtr.h>
#include <JavaScriptCore/JSObjectRef.h>

#include <DirectXTex/DirectXTex.h>

class D3DClass;
class Engine;
class GraphicsContext;
class GraphicsDevice;
class InputController;
class KeyboardEvent;
class MouseEvent;
class Renderer;
class RenderTarget;
class RenderTargetContainer;
class ScrollEvent;
class UltralightManager;
class UltralightView;
class Window;
class WindowHelperForBorderlessResizable;

using std::list;
using std::map;
using std::pair;
using std::string;
using std::vector;
using std::set;
using std::shared_ptr;
using std::unique_ptr;
using std::unordered_map;
using std::make_pair;
using std::make_shared;
using Microsoft::WRL::ComPtr;

namespace ul = ultralight;

using namespace DirectX::SimpleMath;