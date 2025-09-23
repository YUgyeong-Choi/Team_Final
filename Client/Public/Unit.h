#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXActor;
class CPhysXDynamicActor;
class CSoundController;
class CTexture;
NS_END

NS_BEGIN(Client)
class CUnit : public CGameObject
{
	/* [ 모든 움직이는 객체의 부모클래스입니다. ] */
	/*  ( 플레이어, 몬스터, 보스몬스터, NPC )  */


public:
	typedef struct tagUnitDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;
		LEVEL			eMeshLevelID;
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };
		//_float4x4	WorldMatrix = _float4x4(
		//	1.f, 0.f, 0.f, 0.f,
		//	0.f, 1.f, 0.f, 0.f,
		//	0.f, 0.f, 1.f, 0.f,
		//	0.f, 0.f, 0.f, 1.f
		//);
		wstring wsNavName = {}; //어떤 네비를 탈 것인가? //STATION, HOTEL...
		SPAWN_TYPE eSpawnType = { SPAWN_TYPE::END };
	}UNIT_DESC;

protected:
	CUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUnit(const CUnit& Prototype);
	virtual ~CUnit() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual HRESULT Render_Fury() override;

	void SetCascadeShadow();
	


protected: /* [ Setup 함수 ] */
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	HRESULT Ready_Collider();

	virtual void Register_Events() {}

public: /* [ 피직스 관련 ] */
	PxRigidActor* Get_Actor(CPhysXActor* actor);
	_vector Get_RayOffset() { return m_vRayOffset; }
	virtual void RayCast(CPhysXActor* actor);
#ifdef USE_IMGUI
	class CAnimator* Get_Animator() const { return m_pAnimator; }
#endif

protected: /* [ 충돌 시 공통으로 실행 ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

public:
	_float4& Get_LockonPos() { return m_vLockonPos; }
	//락온에서 hp 0이하인거는 제외할라고(죽는모션할떄 락온되서)
	_bool Get_UseLockon() { return m_bUseLockon; }

	EUnitType& Get_UnitType() { return m_eUnitType; }


public: /* [ 셰이더를 켜고 끈다. ] */
	void OnEmissive(_float fTimeDelta);
	void OffEmissive(_float fTimeDelta);
	void OnSecondEmissive(_float fTimeDelta);
	void OffSecondEmissive(_float fTimeDelta);
	void OnFury(_float fTimeDelta);
	void OffFury(_float fTimeDelta);
	void ToggleEmissive(_float fEmissiveSpeed);
	void SwitchEmissive(_bool bEmissive, _float fEmissiveSpeed);
	void SwitchSecondEmissive(_bool bSecondEmissive, _float fSecondEmissiveSpeed);
	void SwitchFury(_bool bFury, _float fFurySpeed);


public:
	// 데미지를 준다
	virtual void Attack(CGameObject* pOther, COLLIDERTYPE eColliderType) {};
	// 무기를 통해 데미지를 준다
	virtual void AttackWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType) {};
	// 데미지를 받는다
	virtual void ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType) {};

	// 이거 일단 나중에? 
	// 데미지를 무기를 통해 받는다? 플레이어 타격 판정 후하게 주려면 괜찮을지도? 
	virtual void ReceiveDamageWithWeapon(CGameObject* pOther, COLLIDERTYPE eColliderType) {};

	AABBBOX GetWorldAABB() const;

	virtual void Block_Reaction() {};

	CPhysXDynamicActor* Get_BodyActor() { return m_pPhysXActorCom; }
public:
	_float Get_CurrentDamage() { return m_fDamage; }
	void Set_CurrentDamage(_float fDamage) { m_fDamage = fDamage; }

protected:
	void Spawn_MonsterHit_Effect(const _float3& vPos);

public:
	_float GetHP() const { return m_fHp; }

protected:
	SPAWN_TYPE m_eSpawnType = { SPAWN_TYPE::IDLE };

protected: /* [ 플레이어 ] */
	CGameObject* m_pPlayer = { nullptr };
	_bool m_bIsPlayer = { false };

protected:
	_float				m_fHp = {};
	_float				m_fMaxHp = {};

protected:
	_bool				m_bEmissive = {};
	_float				m_fEmissive = {};
	_float				m_fEmissiveSpeed = {};

	_bool				m_bFurySwitch = {};
	_float				m_fFurySwitch = {};
	_float				m_fFurySpeed = {};

	class CTexture*		m_pSecondEmissiveMap = { nullptr };
	_float				m_fSecondEmissive = {};
	_bool				m_bSecondEmissive = {};
	_bool				m_bSecondEmissiveLoad = {};
	_float				m_fSecondEmissiveSpeed = {};

	class CTexture* m_pWetTexture = { nullptr };
	_float	m_fWetIntensity = {};
	_bool	m_bWet = {};
	_float	m_fWetSpeed = { 1.f };


protected:              /* [ 컴포넌트 ] */
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	CAnimator*			m_pAnimator = { nullptr };
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CSoundController*	m_pSoundCom = { nullptr };
	CTexture*			m_pNoiseMap = { nullptr };
	CTexture*			m_pTextureCom = { nullptr };

protected:				/* [ 기본 속성 ] */
	_float				m_fSpeedPerSec = 5.f;
	_float				m_fRotationPerSec = XMConvertToRadians(90.f);
	_float				m_fDamage = {};
	_float3				m_InitPos = {};
	_float3				m_InitScale = {};
	_int				m_iRender = {};
	_bool				m_bUseLockon = {};
	_bool				m_bDoOnce = {};

protected: 				/* [ 기본 타입 ] */
	const _tchar*		m_szName = { nullptr };
	const _tchar*		m_szMeshID = { nullptr };
	LEVEL				m_eMeshLevelID = { LEVEL::END };

protected:				/* [ 그림자 관련 ] */
	SHADOW				m_eShadow = SHADOW::SHADOW_END;

protected:				/* [ 레이캐스트 변수 ] */
	PxVec3				m_vRayHitPos = {};
	_bool				m_bRayHit = {};
	_vector				m_vRayOffset = {};

protected:				/* [ 중력관련 변수 ] */
	_bool				m_bOnGround = {};
	PxVec3				m_vVelocity = PxVec3{ 0.f, 0.f, 0.f };
	_float3				m_vGravityVelocity = _float3{ 0.f, 0.f, 0.f };

	class CCamera_Orbital* m_pCamera_Orbital = { nullptr };

	
protected: // .... 
	_float4			    m_vLockonPos = {};
	_int				m_iLockonBoneIndex = {};

	EUnitType			m_eUnitType = EUnitType::NONE;


public:
	static CUnit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END