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
	_fvector& GetPosition();

protected:
	_float				m_fFov = { };
	_float				m_fNear = { };
	_float				m_fFar = { };
	_float				m_fAspect = {};	

protected:
	void Bind_Matrices();

public:
	void StartShake(_float fIntensity, _float fDuration, _float fShakeFreqPos, _float fShakeFreqRot);
	void Update_Camera_Shake(_float fTimedelta);

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
	_vector				m_vCurrentShakePos = {};
	_vector				m_vCurrentShakeRot = {};
	_vector				m_vPureCamPos = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;	
	virtual void Free() override;

};

NS_END