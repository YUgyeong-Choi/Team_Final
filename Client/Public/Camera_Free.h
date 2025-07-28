#pragma once

#include "Client_Defines.h"
#include "Camera.h"

NS_BEGIN(Client)

class CCamera_Free final : public CCamera
{
public:
	typedef struct tagCameraFreeDesc : public CCamera::CAMERA_DESC
	{
		_float			fSensor;
	}CAMERA_FREE_DESC;
private:
	CCamera_Free(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Free(const CCamera_Free& Prototype);
	virtual ~CCamera_Free() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
public:
	void Set_Moveable(_bool bMoveable) {
		m_bMoveable = bMoveable;
	}

private:
	_float				m_fSensor{};
	_bool				m_bSprint = false;

	//맵툴에서 프리뷰랑 같이움직여서 불값 하나 추가
	_bool				m_bMoveable = { true };

	float m_fYaw = 0.f;
	float m_fPitch = 0.f; // Pitch는 클램핑 필요
public:
	static CCamera_Free* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END