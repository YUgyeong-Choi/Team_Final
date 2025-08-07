#pragma once
#include "Client_Defines.h"
#include "GameInstance.h"

static _float2 Get_MousePos()
{
	POINT pt;
	GetCursorPos(&pt); // 스크린 기준 좌표
	ScreenToClient(g_hWnd, &pt); // 클라이언트(윈도우 내부) 기준으로 변환

	_float2 vMousePos = { static_cast<_float>(pt.x), static_cast<_float>(pt.y) };

	return vMousePos;
}
