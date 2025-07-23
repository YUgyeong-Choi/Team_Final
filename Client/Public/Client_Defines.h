#pragma once

#include "../Default/framework.h"
#include <process.h>
#include "Client_Enum.h"
#include "Client_Struct.h"

#ifdef USE_IMGUI
#pragma push_macro("new")
#undef new
#include "imgui.h"
#include "imgui_internal.h"
#pragma pop_macro("new")
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ImGuizmo.h"
#include "ImGuiFileDialog.h"
#include "imnodes.h"
#include "ImSequencer.h"

using namespace ImNodes;
using namespace ImGui;
#endif

namespace Client
{
	const unsigned int			g_iWinSizeX = 1920;
	const unsigned int			g_iWinSizeY = 1080;
}

extern HWND g_hWnd;
extern HINSTANCE g_hInst;


using namespace Client;
