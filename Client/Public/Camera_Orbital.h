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

	/* [ ���� ���� ���� ] */
	void Set_LockOn(CGameObject* pTarget, _bool bActive);

	/* [ ���� �ִ� pitch & yaw ����� ����Ż ��ġ ������ ] */
	_matrix Get_OrbitalWorldMatrix(_float fPitch, _float fYaw);

	/* [ ���� ���ͷ� Yaw Pich ��������� ] */
	void Set_TargetYawPitch(_vector vDir, _float fLerpSpeed);
	
	void Set_ActiveTalk(_bool bActive) { m_bTalkNpc = bActive; }
private:
	/* [ ��ҿ� ���� ] */
	void Update_CameraMatrix(_float fTimeDelta);
	/* [ TargetPitch & Yaw �� ���� ] */
	void Update_TargetCameraMatrix(_float fTimeDelta);
	/* [ ���� �Ǿ��� �� ] */
	void Update_LockOnCameraMatrix(_float fTimeDelta);
	/* [ ���� ���۰� ���� ] */
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
	const _float m_fPadding = 1.0f;     // �÷��̾� & Ÿ���� ����� ���� ���� �ּ� ������
	const _float m_fFrame = 0.5f;   // ȭ�� ������ �ȿ� �������� ������ ���� ����
	const _float m_fDistanceMin = 3.0f;     //  �÷��̾� & Ÿ���� �ʹ� ����� ���°� ����
	const _float m_fDistanceMax = 6.0f;     // �÷��̾� & Ÿ���� �־����� ���°� ����
	const _float fMinPlayerDepth = 1.2f;    // �÷��̾� ī�޶��� �ּ� �Ÿ�
	_float m_fCurDistance = 3.0f; //m_fDistanceMin ������
private:
	CGameObject*	m_pPlayer = { nullptr };
	// ������ �ȵ� ���� �׻� nullptr
	CGameObject*	m_pLockOnTarget = { nullptr }; 
public:
	static CCamera_Orbital* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

