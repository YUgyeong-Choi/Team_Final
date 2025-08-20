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

	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);

	/* Ray�� ���� �浹(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);


public: /* [ Setup �Լ� ] */
	HRESULT Ready_Components();
	HRESULT Ready_Actor(void* pArg);
	HRESULT Ready_PartObject();
	//HRESULT Ready_Weapon(); ����� �ʿ��� �ֵ鸸 �����
	
	
	void	RootMotionActive(_float fTimeDelta);

	void    LoadAnimDataFromJson();

	void    Update_Collider();

	CGameObject* Find_Player(_int iLevel);

	virtual void	Update_State() {};

	_bool Check_Detect();

	MONSTER_DIR Calc_HitDir(_vector vOtherPos);

	// ��� lookat���� ���� idle������ turn ��Ű��
	// ���������� ������ ����?

	_bool Check_Turn();

	MONSTER_DIR Calc_TurnDir(_vector vOtherPos);

protected:


	STATE_MONSTER	m_eCurrentState = {};
	//CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };
	CNavigation* m_pNaviCom = { nullptr };
	
	CUI_MonsterHP_Bar* m_pHPBar = {nullptr};

	
	_float			m_fHp = {};
	_float			m_fMaxHp = {};

	_float			m_fDamage = {};
	

	// Ư�� ���¿��� ��� �ٶ󺸰� �ؾ�?
	_bool			m_isLookAt = {};

	// �ൿ ������ ���� ������
	MONSTER_DIR		m_eDir = { MONSTER_DIR::END };
	_float			m_fGroggyThreshold = {};
	_bool			m_isCanGroggy = {};
	_bool			m_isDetect = { false };
	_float			m_fDetectDist = {};
	string			m_strStateName = {};

	//
	_float			m_fHeight = {};
	

private: /* [ ��Ʈ��� ���� ���� ] */
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

