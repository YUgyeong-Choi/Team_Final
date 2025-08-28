#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CCamera abstract : public CGameObject
{
public:
	typedef struct tagCameraDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float3			vEye;
		_float3			vAt;
		_float			fFov, fNear, fFar;
	}CAMERA_DESC;
protected:
	CCamera(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCamera(const CCamera& Prototype);
	virtual ~CCamera() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT Update_Camera();
	_fvector GetPosition();

public:
	_vector GetRightVector();
	_vector GetUpVector();
	_vector GetLookVector();

protected:
	_float				m_fFov = { };
	_float				m_fNear = { };
	_float				m_fFar = { };
	_float				m_fAspect = {};	

protected:
	void Bind_Matrices();

public:
	void StartShake(_float fIntensity, _float fDuration, _float fShakeFreqPos = 100.f, _float fShakeFreqRot = 40.f, _float fShakeUpdateInterval = 0.016f);
	void StartRot(_vector vRot, _float fDuration);
	void Update_Camera_Shake(_float fTimedelta);
	void Update_Camera_MoreRot(_float fTimedelta);

public:
	_vector GetPureCamPos() const { return m_vPureCamPos; }
	_bool GetIsShake() const { return m_bShake; }

public:
	const _matrix& GetViewMatrix() const { return m_ViewMatrix; }
	const _matrix& GetProjMatrix() const { return m_ProjMatrix; }

protected:
	_matrix m_ViewMatrix = XMMatrixIdentity();
	_matrix m_ProjMatrix = XMMatrixIdentity();

protected:
	_bool				m_bShake = {};
	_float				m_fShakeTime = {};
	_float				m_fShakeDuration = {};
	_float				m_fShakeFreqPos = {};
	_float				m_fShakeFreqRot = {};
	_float				m_fShakeIntensity = {};
	_float m_fShakeUpdateInterval = 0.016f;
	/*
	0.016f → 60FPS 기준으로 매 프레임 갱신
	0.033f → 60FPS 기준으로 2프레임마다 갱신
	0.05f → 60FPS 기준으로 3프레임 정도마다 갱신
	*/
	_float m_fShakeUpdateAccum = 0.f;

	_bool				m_bMoreRot = {};
	_float				m_fMoreRotTime = {};
	_float				m_fMoreRotDuration = {};
	_vector				m_vMoreRotFreq = {};

	_vector				m_vCurrentShakePos = {};
	_vector				m_vCurrentShakeRot = {};
	_vector				m_vPureCamPos = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;	
	virtual void Free() override;

};

NS_END