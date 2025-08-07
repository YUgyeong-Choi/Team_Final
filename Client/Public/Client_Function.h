#pragma once
#include "Client_Defines.h"
#include "GameObject.h"
#include "GameInstance.h"
static _float2 Get_MousePos()
{
	POINT pt;
	GetCursorPos(&pt); // ��ũ�� ���� ��ǥ
	ScreenToClient(g_hWnd, &pt); // Ŭ���̾�Ʈ(������ ����) �������� ��ȯ

	_float2 vMousePos = { static_cast<_float>(pt.x), static_cast<_float>(pt.y) };

	return vMousePos;
}

/* ���߿� �÷��̾� static���� ���� ������ ���� */
CGameObject* Get_Player(_uint iLevelIndex)
{
	return CGameInstance::Get_Instance()->Get_LastObject(iLevelIndex, TEXT("Layer_Player"));
}