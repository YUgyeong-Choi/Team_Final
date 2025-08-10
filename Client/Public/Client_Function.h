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


inline AABBBOX AABB_FromCenterHalfExtents(const _float3& c, const _float3& e)
{
	return { { c.x - e.x, c.y - e.y, c.z - e.z },
			 { c.x + e.x, c.y + e.y, c.z + e.z } };
}

inline AABBBOX TransformAABB(const AABBBOX& local, const DirectX::XMMATRIX& world)
{
    using namespace DirectX;
    XMFLOAT3 mn = local.vMin, mx = local.vMax;
    XMFLOAT3 corners[8] = {
        {mn.x,mn.y,mn.z},{mx.x,mn.y,mn.z},{mx.x,mn.y,mx.z},{mn.x,mn.y,mx.z},
        {mn.x,mx.y,mn.z},{mx.x,mx.y,mn.z},{mx.x,mx.y,mx.z},{mn.x,mx.y,mx.z},
    };
    XMFLOAT3 outMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    XMFLOAT3 outMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (int i = 0; i < 8; ++i) {
        XMVECTOR p = XMVector3Transform(XMLoadFloat3(&corners[i]), world);
        XMFLOAT3 w; XMStoreFloat3(&w, p);
        outMin.x = (std::min)(outMin.x, w.x);
        outMin.y = (std::min)(outMin.y, w.y);
        outMin.z = (std::min)(outMin.z, w.z);
        outMax.x = (std::max)(outMax.x, w.x);
        outMax.y = (std::max)(outMax.y, w.y);
        outMax.z = (std::max)(outMax.z, w.z);
    }
    return { outMin, outMax };
}