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

	void Set_PitchYaw(_float fPitch, _float fYaw);
	_float Get_Pitch() { return m_fPitch; }
	_float Get_Yaw() { return m_fYaw; }

	/* [ 락온 시작 설정 ] */
	void Set_LockOn(CGameObject* pTarget, _bool bActive);

	/* [ 직접 주는 pitch & yaw 값대로 오비탈 위치 가져옴 ] */
	_matrix Get_OrbitalWorldMatrix(_float fPitch, _float fYaw);

	/* [ 방향 벡터로 Yaw Pich 가지고오기 ] */
	void Set_TargetYawPitch(_vector vDir, _float fLerpSpeed);
	
	void Set_ActiveTalk(_bool bActive) { m_bTalkNpc = bActive; }
private:
	/* [ 평소에 실행 ] */
	void Update_CameraMatrix(_float fTimeDelta);
	/* [ TargetPitch & Yaw 로 보간 ] */
	void Update_TargetCameraMatrix(_float fTimeDelta);
	/* [ 락온 되었을 때 ] */
	void Update_LockOnCameraMatrix(_float fTimeDelta);
	/* [ 락온 시작과 종료 ] */
	void Update_LockOnTransition(_float fTimeDelta);

	void Set_CameraMatrix(_float fTimeDelta);
private:
	_bool			m_bSetPitchYaw = false;

	_float			m_fYaw = 0.f;
	_float			m_fPitch = 0.f;
	_float			m_fTargetYaw = 0.f;
	_float			m_fTargetPitch = 0.f;
	_float			m_fTargetLerpSpeed = 0.f;

	_float			m_fDistance = 3.f;
	_float			m_fZoomSpeed = 0.5f;
	_float			m_fMouseSensor = { };

	_vector			m_vTargetCamPos = {};
	_vector			m_vPlayerPosition = {};
private:
	_bool			m_bLockOn = false;
	_bool			m_bLockOnTransition = false;
	_bool			m_bLockOnTransitionStart = false;
private:
	_bool			m_bTalkNpc = false;
private:
	const _float m_fPadding = 1.0f;     // 플레이어 & 타겟이 가까울 때를 위한 최소 반지름
	const _float m_fFrame = 0.5f;   // 화면 높이의 안에 들어오도록 여유를 위한 변수
	const _float m_fDistanceMin = 3.0f;     //  플레이어 & 타겟이 너무 가까워 지는거 방지
	const _float m_fDistanceMax = 6.0f;     // 플레이어 & 타겟이 멀어져서 지는거 방지
	const _float fMinPlayerDepth = 1.2f;    // 플레이어 카메라간의 최소 거리
	_float m_fCurDistance = 3.0f; //m_fDistanceMin 같도록
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

