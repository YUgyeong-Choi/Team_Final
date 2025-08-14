#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CWeapon : public CGameObject
{
	/* [ 모든 무기객체의 부모클래스입니다. ] */


public:
	typedef struct tagWeaponDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szMeshID;
		LEVEL			eLevelID;
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };

		const _float4x4* pSocketMatrix = { nullptr };
		const _float4x4* pParentWorldMatrix = { nullptr };
	}WEAPON_DESC;

protected:
	CWeapon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWeapon(const CWeapon& Prototype);
	virtual ~CWeapon() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected:
	void SetWeaponWorldMatrix(_float fTimeDelta);

protected: /* [ Setup 함수 ] */
	HRESULT Bind_Shader();
	HRESULT Ready_Components();


protected: /* [ 충돌 시 공통으로 실행 ] */
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

public: /* [ 활성화 , 비활성화 ] */
	void SetbIsActive(_bool IsActive) { m_bIsActive = IsActive; }
	_bool GetbIsActive() const { return m_bIsActive; }

	// 레벨 업이나 강화 시 공격력을 바꾸도록
	void SetBasicDamage(_float fDamage) { m_bDamage = fDamage; }

	// 상태에 따라 공격 할 수 있는지 없는지
	// 데미지 배율을 줘서 공격할 수 있도록
	void SetisAttack(_bool isAttack) { m_isAttack = isAttack; }
	_bool GetisAttack() { return m_isAttack; }
	
	// 상태 마다 배율을 바꿔주기
	void SetDamageRatio(_float fRatio) { m_fDamageRatio = fRatio; }

	// 기본 데미지에 배율을 곱해서 데미지를 줄 수 있게
	_int Get_CurrentDamage() { return _int(m_bDamage * m_fDamageRatio); }


	void Clear_CollisionObj() { m_CollisonObjects.clear(); }

public:
	_wstring Get_MeshName() { return (m_szMeshID != nullptr) ? wstring(m_szMeshID) : wstring(); }
	SKILL_DESC& Get_SkillDesc(_int iIndex) { return m_eSkillDesc[iIndex]; }
	const _float4x4* Get_CombinedWorldMatrix() const { return &m_CombinedWorldMatrix; }

protected:
	const _float4x4*	m_pParentWorldMatrix = { nullptr };
	const _float4x4*	m_pSocketMatrix = { nullptr };
	_float4x4			m_CombinedWorldMatrix = {};

protected:				/* [ 기본 속성 ] */
	_float				m_bDamage = { 100.f };
	_bool				m_bIsActive = {};
	_bool				m_isAttack = {};
	_float				m_fSpeedPerSec = 5.f;
	_float				m_fRotationPerSec = XMConvertToRadians(90.f);
	_float3				m_InitPos = {};
	_float3				m_InitScale = {};
	_int				m_iRender = {};
	
	_float				m_fDamageRatio = {1.f};

protected: 				/* [ 기본 타입 ] */
	const _tchar*		m_szName = { nullptr };
	const _tchar*		m_szMeshID = { nullptr };
	LEVEL				m_eLevelID = { LEVEL::END };

protected:				/* [ 그림자 관련 ] */
	SHADOW				m_eShadow = SHADOW::SHADOW_END;

protected:              /* [ 컴포넌트 ] */
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	CAnimator*			m_pAnimator = { nullptr };

protected:				/* [ 레이캐스트 변수 ] */
	PxVec3				m_vRayHitPos = {};
	_bool				m_bRayHit = {};

protected:		//		스킬용 변수?
	// 0번은 날 스킬, 1번은 자루 스킬
	SKILL_DESC			m_eSkillDesc[2];
	// 내구도
	_int				m_iDurability = {};
	_int				m_iMaxDurability = {100};

protected:
	_int				m_iBladeBoneIndex = {};
	_float4				m_vBladePos = {};

	
	vector<CGameObject*> m_CollisonObjects;

public:
	static CWeapon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END