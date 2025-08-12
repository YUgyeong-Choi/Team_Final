#pragma once

#include "../Default/framework.h"
#include <process.h>
#include "Client_Enum.h"
#include "Client_Struct.h"

//#ifdef USE_IMGUI
#pragma push_macro("new")
#undef new
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "ImGuizmo.h"
#include "ImGuiFileDialog.h"
#include "imnodes.h"
#include "imnodes_internal.h"
#include "ImSequencer.h"
#pragma pop_macro("new")

using namespace ImNodes;
using namespace ImGui;
//#endif

// 영상 용
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mferror.h>
#include <iostream>

// 로딩 비율 설정하기 위한?
#include <atomic>

namespace Client
{
	const unsigned int			g_iWinSizeX = 1600;
	const unsigned int			g_iWinSizeY = 900;
}

extern HWND g_hWnd;
extern HINSTANCE g_hInst;


using namespace Client;

#pragma region 한영웅
#define PATH_NONANIM "../Bin/Resources/Models/Bin_NonAnim"
#define PRE_TRANSFORMMATRIX_SCALE 0.004f
#define INSTANCE_THRESHOLD 30 //이 갯수 초과 모델로드시 인스턴싱으로 로드한다.
#pragma endregion

#pragma region MANAGER

#define EFFECT_MANAGER CEffect_Manager::Get_Instance()
#define CAMERA_MANAGER CCamera_Manager::Get_Instance()

#pragma endregion

#define GET_PLAYER(iLevelIndex) static_cast<CPlayer*>(CGameInstance::Get_Instance()->Get_LastObject(iLevelIndex, TEXT("Layer_Player")))