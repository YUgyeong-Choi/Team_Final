#pragma once

#include "../Default/framework.h"
#include <process.h>
#include "Client_Enum.h"
#include "Client_Struct.h"

//#ifdef USE_IMGUI
#pragma push_macro("new")
#undef new
#define IMGUI_DEFINE_MATH_OPERATORS
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

// ���� �� - �� �Ǹ� �����
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mfobjects.h>
#include <mferror.h>
#include <iostream>
#include <queue>

// �ε� ���� �����ϱ� ����?
#include <atomic>

namespace Client
{

	const unsigned int			g_iWinSizeX = 1920; //1920 1600
	const unsigned int			g_iWinSizeY = 1080; //1080 900


	const float g_fBGMSoundVolume = 1.f; // BGM ����
	const float g_fInteractSoundVolume = 0.7f; // NPC��ȭ ����
	const float g_fPlayerSoundVolume = 0.6f; // OBJECT ����
}

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

using namespace Client;

#define TEST_FIRE_EATER_MAP //Ǫ���ڸʸ� �����
#define TESTMAP //�׽�Ʈ�� �ϱ� ������ �ּ��Ͻÿ� (����)

#pragma region �ѿ���
#define PATH_NONANIM "../Bin/Resources/Models/Bin_NonAnim"
#define PRE_TRANSFORMMATRIX_SCALE 0.004f
#define INSTANCE_THRESHOLD 30 //�� ���� �ʰ� �𵨷ε�� �ν��Ͻ����� �ε��Ѵ�.
#pragma endregion

#pragma region MANAGER

#define EFFECT_MANAGER CEffect_Manager::Get_Instance()
#define MAKE_EFFECT(LEVEL, EC_NAME, DESC)\
		EFFECT_MANAGER->Make_EffectContainer(LEVEL, EC_NAME, DESC)
#define MAKE_SINGLEEFFECT(LEVEL, EFFECT_NAME, LAYER_NAME, PRESETPOSX, PRESETPOSY, PRESETPOSZ, PDESC) \
		EFFECT_MANAGER->Make_Effect(LEVEL, EFFECT_NAME, LAYER_NAME, _float3{ PRESETPOSX, PRESETPOSY, PRESETPOSZ }, PDESC)


#define CAMERA_MANAGER CCamera_Manager::Get_Instance()

#pragma endregion

#define GET_PLAYER(iLevelIndex) static_cast<CPlayer*>(CGameInstance::Get_Instance()->Get_LastObject(iLevelIndex, TEXT("Layer_Player")))