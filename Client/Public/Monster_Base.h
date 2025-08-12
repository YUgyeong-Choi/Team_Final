#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "UI_MonsterHP_Bar.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXDynamicActor;
class CNavigation;
NS_END

NS_BEGIN(Client)

class CMonster_Base : public CUnit
{
public:
	enum class STATE_MONSTER { IDLE, WALK, RUN, TURN, DEAD, HIT, GROGGY, FATAL, ATTACK, END };
	enum class MONSTER_DIR { F, B, L, R, END };

	typedef struct eMonsterBaseTag : public CUnit::UNIT_DESC {
		
		_float3 vExtent;
		_float  fHeight;

	}MONSTER_BASE_DESC;

protected:
	CMonster_Base(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonster_Base(const CMonster_Base& Prototype);
	virtual ~CMonster_Base() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	/* Ray로 인항 충돌(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);


public: /* [ Setup 함수 ] */
	HRESULT Ready_Components();
	HRESULT Ready_Actor(void* pArg);
	HRESULT Ready_PartObject();
	//HRESULT Ready_Weapon(); 무기는 필요한 애들만 만들기
	
	
	void	RootMotionActive(_float fTimeDelta);

	void    LoadAnimDataFromJson();

	void    Update_Collider();

	CGameObject* Find_Player(_int iLevel);

	virtual void	Update_State() {};

	_bool Check_Detect();

	MONSTER_DIR Calc_HitDir(_vector vOtherPos);

protected:


	STATE_MONSTER	m_eCurrentState = {};
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CNavigation* m_pNaviCom = { nullptr };
	
	CUI_MonsterHP_Bar* m_pHPBar = {nullptr};

	
	

	// 특정 상태에만 계속 바라보게 해야?
	_bool			m_isLookAt = {};

	// 행동 결정을 위한 변수들
	MONSTER_DIR		m_eDir = { MONSTER_DIR::END };
	_int			m_iStamina = {100};
	_int			m_iGroggyThreshold = {};
	_bool			m_isCanGroggy = {};
	_bool			m_isDetect = { false };
	_float			m_fDetectDist = {};
	_float			m_fHeight = {};
	string			m_strStateName = {};

private: /* [ 루트모션 관련 변수 ] */
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_bool    m_bIsFirstFrame = true;
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.1f;


public:
	static CMonster_Base* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

