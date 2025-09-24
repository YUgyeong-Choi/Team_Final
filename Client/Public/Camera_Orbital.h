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
	void Set_InitCam(_float fPitch, _float fYaw);

	void Set_PitchYaw(_float fPitch, _float fYaw);
	_float Get_Pitch() { return m_fPitch; }
	_float Get_Yaw() { return m_fYaw; }

	/* [ ���� �ִ� pitch & yaw ����� ����Ż ��ġ ������ ] */
	_matrix Get_OrbitalWorldMatrix(_float fPitch, _float fYaw);

	_bool Get_bActive() { return m_bActive; }
	void Set_bActive(_bool bActive) { m_bActive = bActive; }

	/* [ ���� ���ͷ� Yaw Pich ��������� ] */
	void Set_TargetYawPitch(_vector vDir, _float fLerpSpeed, _bool bActivePitch = true);
	
	void Set_LockOn(CGameObject* pTarget, _bool bActive);
	void Set_ActiveTalk(_bool bActive, CGameObject*  pTarget, _bool bCanMove, _float fTalkOffSet);
	void Start_DistanceLerp(_float fTargetLerpDistance, _float fDistanceStartTime, _float fDistanceEndTime, _float fDistanceDelayTime = 0.f);
private:
	// ī�޶� Look ����
	/* [ ��ҿ� ���� ] */
	void Update_CameraLook(_float fTimeDelta);
	/* [ TargetPitch & Yaw �� ���� ] */
	void Update_TargetCameraLook(_float fTimeDelta);
	/* [ ���� �Ǿ��� �� ] */
	void Update_LockOnCameraLook(_float fTimeDelta);

	// ī�޶� Pos ����
	void Update_CameraPos(_float fTimeDelta);

	void Update_LerpDistacne(_float fTimeDelta);
private:
	_bool			m_bSetPitchYaw = false;

	_float			m_fYaw = 0.f;
	_float			m_fPitch = 0.f;
	_float			m_fTargetYaw = 0.f;
	_float			m_fTargetPitch = 0.f;
	_float			m_fTargetLerpSpeed = 0.f;

	_float			m_fDistance = 3.f;
	_float			m_fMouseSensor = { };

	_float4			m_vTargetCamPos = {};
	_float4			m_vPlayerPosition = {};
private:
	_bool			m_bLockOn = false;
private:
	_bool			m_bTalkStart = false;
	_bool			m_bTalkEnd = false;
	_bool			m_bTalkActive = false;
	_bool			m_bCanMoveTalk = true;
	_float			m_fTalkOffSet = 0.f;
private:
	_bool			m_bDistanceLerp = {};
	_float			m_fDistanceTime = {};
	_float			m_fDistanceTarget = {};
	_float			m_fDistnaceStartSpeed = {};
	_float			m_fDistnaceEndSpeed = {};
	_float			m_fDistanceLerpElapsed = {};
	_float			m_fDistanceDelayTime = 0.f;      
	_float			m_fDistanceDelayElapsed = 0.f;  

	_bool			m_bActive = false;


	_float			m_fAlwaysDistanceTarget = 3.f;
private:
	const _float m_fPadding = 1.0f;     // �÷��̾� & Ÿ���� ����� ���� ���� �ּ� ������
	const _float m_fFrame = 0.5f;   // ȭ�� ������ �ȿ� �������� ������ ���� ����
	const _float m_fDistanceMin = 3.0f;     //  �÷��̾� & Ÿ���� �ʹ� ����� ���°� ����
	const _float m_fDistanceMax = 6.0f;     // �÷��̾� & Ÿ���� �־����� ���°� ����
	const _float fMinPlayerDepth = 1.2f;    // �÷��̾� ī�޶��� �ּ� �Ÿ�
private:
	CGameObject*	m_pPlayer = { nullptr };
	// ������ �ȵ� ���� �׻� nullptr
	CGameObject*	m_pLockOnTarget = { nullptr }; 

	// Npc ��ȭ�� Ÿ��
	CGameObject* m_pNpcTalkTarget = { nullptr };

	// Look ������ ���� ������
	_float4 m_vPrevLookTarget = {};
	bool m_bPrevLookInit = false;
	float m_fLookLerpSpeed = 8.f; // �� Ŭ���� ������ ����
public:
	static CCamera_Orbital* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END

