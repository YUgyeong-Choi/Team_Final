#pragma once
#include "GameObject.h"
#include "Client_Defines.h"


NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXController;
NS_END

NS_BEGIN(Client)
class CTestAnimObject : public CGameObject
{
private:
	CTestAnimObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTestAnimObject(const CTestAnimObject& Prototype);
	virtual ~CTestAnimObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	PxRigidActor* Get_Actor() { return m_pControllerCom->Get_Actor(); }
private:
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	void Input_Test(_float fTimeDelta);

	HRESULT Ready_Collider();
	void SyncTransformWithController();

private:/* [ 캐스케이드 전용함수 ] */
	HRESULT UpdateShadowCamera();
private: /* [ 이동로직 ] */
	void SetMoveState(_float fTimeDelta);

	void RayCast();

private: /* 옵저버 관련*/
	void Set_StatusObserver();
	void Callback_HP();
	void Callback_Stamina();

	// 스탯 변화 테스트용
	void Update_Stat();

private:
	PxVec3 m_vRayHitPos = {};
	_bool m_bRayHit = false;

private:
	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };

private:
	_vector m_vShadowCam_Eye = {};
	_vector m_vShadowCam_At = {};

private:
	CModel* m_pModelCom = nullptr; // 모델 컴포넌트
	CShader* m_pShaderCom = nullptr; // 셰이더 컴포넌트
	CAnimator* m_pAnimator = nullptr; // 애니메이터
	CPhysXController* m_pControllerCom = { nullptr };
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_bool    m_bIsFirstFrame = true;
	_float   m_fRotSmoothSpeed = 8.0f; // 회전 스무딩 속도
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.1f; // 스무딩 임계값
	//중력땜시
private:
	bool m_bOnGround = false;
	PxVec3 m_vVelocity = PxVec3(0.f, 0.f, 0.f);
	XMFLOAT3 m_vGravityVelocity = { 0.f, 0.f, 0.f };  // 헤더에 선언

private: // 옵저버 관련
	// stat용
	class CObserver_Player_Status* m_pStatusObserver = { nullptr };
	_int m_iCurrentHP = {};
	_int m_iMaxHP = {200};
	_int m_iCurrentStamina = {};
	_int m_iMaxStamina = {100};
	// 일단 한칸에 50씩
	_int m_iCurrentMana = {};
	_int m_iMaxMana = { 150 };

	// 아이템?
	// 에르고(돈)
	// 무기

public:
	static CTestAnimObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

