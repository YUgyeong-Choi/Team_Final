#pragma once

#include "Client_Defines.h"
#include "Camera.h"


NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CCamera_Orbital final : public CCamera
{
public:
	typedef struct tagCameraOrbitalDesc : public CCamera::CAMERA_DESC
	{
		_float			fMouseSensor = {};
	}CAMERA_ORBITAL_DESC;

private:
	CCamera_Orbital(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera_Orbital(const CCamera_Orbital& Prototype);
	virtual ~CCamera_Orbital() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	void ActiveDialogView(_vector NPCPos, _vector NPCLook);

	void SetbActiveDialogView(_bool bActive) { m_bActiveDialogView = bActive; }
	_bool GetbActiveDialogView() const { return m_bActiveDialogView; }

public:
	void SetPlayer(CGameObject* pPlayer) { m_pPlayer = pPlayer; }

private:
	_float			m_fMouseSensor = { };

private:
	_bool			m_bRockOn = false;

	_float			m_fYaw = 0.f;
	_float			m_fPitch = 0.f;
	_float			m_fDistance = 4.5f;
	_float			m_fZoomSpeed = 0.5f;


	_vector			m_vTargetCamPos = {};
	_vector			m_vCorrectedCamPos = {};

	_vector			m_vPlayerPosition = {};

private:
	CGameObject*	m_pPlayer = { nullptr };

private:
	_vector			m_vDialogPostion = {};
	_bool			m_bActiveDialogView = false;

public:
	static CCamera_Orbital* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

