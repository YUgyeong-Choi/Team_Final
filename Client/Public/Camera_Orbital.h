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
	void SetPlayer(CGameObject* pPlayer) { m_pPlayer = pPlayer; }
	void Set_InitCam();

	void Set_PitchYaw(_float pitch, _float yaw);
	_float Get_Pitch() { return m_fPitch; }
	_float Get_Yaw() { return m_fYaw; }

	/* [ 락온 시작 설정 ] */
	void Set_LockOn(CGameObject* pTarget, _bool bActive);

	/* [ 직접 주는 pitch & yaw 값대로 오비탈 위치 가져옴 ] */
	_matrix Get_OrbitalWorldMatrix(_float pitch, _float yaw);

	/* [ 플레이어 뒤로 카메라 셋 ] */
	void Set_OrbitalPosBackLookFront();
private:
	void Update_CameraMatrix(_float fTimeDelta);
	void Update_CameraLockOnMatrix(_float fTimeDelta);
private:
	_bool			m_bLockOn = false;

	_float			m_fYaw = 0.f;
	_float			m_fPitch = 0.f;
	_float			m_fDistance = 3.f;
	_float			m_fZoomSpeed = 0.5f;
	_float			m_fMouseSensor = { };

	_vector			m_vTargetCamPos = {};
	_vector			m_vCorrectedCamPos = {};

	_vector			m_vPlayerPosition = {};

private:
	CGameObject*	m_pPlayer = { nullptr };
	// 락온이 안될 때는 항상 nullptr
	CGameObject*	m_pLockOnTarget = { nullptr }; 
public:
	static CCamera_Orbital* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

