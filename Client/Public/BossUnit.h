#pragma once
#include "Unit.h"
#include "Client_Defines.h"
#include "PhysX_ControllerReport.h"
#include "UI_MonsterHP_Bar.h"

NS_BEGIN(Engine)
class CBone;
class CNavigation;
class CPhysXController;
class CPhysXDynamicActor;
class CAnimController;
NS_END

NS_BEGIN(Client)
class CBossUnit : public CUnit
{
public:
	enum class EBossAttackType
	{
		FURY_AIRBORNE,AIRBORNE,STRONG_KNOCKBACK,KNOCKBACK, NORMAL, FURY_STAMP, STAMP, NONE
	};
protected:
	enum class EBossState {
		IDLE, WALK, RUN, TURN, ATTACK, GROGGY, PARALYZATION, FATAL, DEAD, CUTSCENE,NONE
	};

	enum class EMoveDirection {
		FRONT, RIGHT, BACK, LEFT
	};



protected:
	CBossUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossUnit(const CBossUnit& Prototype);
	virtual ~CBossUnit() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;

public:
	const EBossAttackType& Get_BossAttackType() const { m_eBossAttackType; }
	void EnterCutScene() { 

		CUI_MonsterHP_Bar::HPBAR_DESC eDesc{};
		eDesc.strName = TEXT("���� �Ҳ� Ǫ����");
		eDesc.isBoss = true;
		eDesc.pHP = &m_fHP;
		eDesc.pIsGroggy = &m_isGroggy;

		m_pHPBar = static_cast<CUI_MonsterHP_Bar*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
			ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_HPBar"), &eDesc));

		m_pAnimator->SetPlaying(true);
		m_bCutSceneOn = true; }
protected:
	virtual void On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
	virtual void ReceiveDamage(CGameObject* pOther, COLLIDERTYPE eColliderType);
	/* Ray�� ���� �浹(HitPos& HitNormal) */
	virtual void On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);

	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);


	HRESULT LoadAnimationEventsFromJson(const string& modelName);
	HRESULT LoadAnimationStatesFromJson(const string& modelName);
	HRESULT LoadFromJson();
	virtual HRESULT Ready_Components(void* pArg);
	virtual HRESULT Ready_Actor();
	virtual void Ready_EffectNames() {};
	virtual void Ready_BoneInformation() {};

	virtual void Update_Collider();
	virtual void UpdateBossState(_float fTimeDelta);
	virtual void UpdateMovement(_float fDistance, _float fTimeDelta);
	virtual void UpdateAttackPattern(_float fDistance, _float fTimeDelta) ;
	virtual void UpdateStateByNodeID(_uint iNodeID);
	virtual void ProcessingEffects(const _wstring& stEffectTag) {}; // ����Ʈ ó��

	virtual void EnableColliders(_bool bEnable) {};
	_bool CanMove() const;
	_bool  IsTargetInFront(_float fDectedAngle = 60.f) const;

	_bool UpdateTurnDuringAttack(_float fTimeDelta);
	_float Get_DistanceToPlayer() const;
	_vector GetTargetDirection() const;
	_int GetYawSignFromDiretion() {
		// �÷��̾� ���� ����
		_vector vDir = GetTargetDirection();
		vDir = XMVectorSetY(vDir, 0.f);
		_vector vLook = XMVector3Normalize(m_pTransformCom->Get_State(STATE::LOOK));
		_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
		_vector vCross = XMVector3Cross(vLook, vDir);
		return (XMVectorGetY(vCross) < 0.f) ? 1 : 0;
	}

	void SetTurnTimeDuringAttack(_float fTime, _float fAddtivRotSpeed = 1.f)
	{
		if (m_fTurnTimeDuringAttack != 0.f)
			return;
		m_fTurnTimeDuringAttack = fTime;
		m_fAddtiveRotSpeed = fAddtivRotSpeed;
	}

	// �̵�
	void ApplyRootMotionDelta(_float fTimeDelta);
	void UpdateNormalMove(_float fTimeDelta);

	virtual void Ready_AttackPatternWeightForPhase1();
	virtual void Ready_AttackPatternWeightForPhase2();

	virtual void UpdateSpecificBehavior() {};

	_float CalculateCurrentHpRatio() const
	{
		return m_fHP / m_fMaxHP;
	}

	// ����Ʈ ��� ����
	virtual HRESULT EffectSpawn_Active(_int iPattern, _bool bActive, _bool bIsOnce = true) { return S_OK; }
	virtual HRESULT Spawn_Effect() { return S_OK; }

	virtual HRESULT Ready_Effect() { return S_OK; } // Initialize���� Loop�� ��� ���� ����Ʈ ready
	_bool CanGroggyActive() const { return m_bGroggyActive; } // �׷α⸦ ���� �� �ִ� ��������
protected:
	CNavigation* m_pNaviCom = { nullptr };

	EBossState m_eCurrentState = EBossState::NONE;
	_bool    m_bIsFirstAttack{ true }; // �ƾ��ϰ� ���� ó��
	_bool    m_bIsPhase2{ false };
	_bool    m_bStartPhase2 = false;

	// ü��
	_float   m_fHP = 100.f;
	_float   m_fMaxHP = 100.f;
	_float   m_fPhase2HPThreshold = 0.5f; // 50% ���Ϸ� �������� ������2 ����

	 _bool   m_bGroggyActive = false;
	_float   m_fGroggyGauge  = 0.f;       // ���� ��
	_float   m_fGroggyThreshold = 1.f;   // �ߵ� ����
	_float   m_fGroggyTimer = 5.f;       // ȭ��Ʈ ������ ���� �ð�
	_float	 m_fGroggyEndTimer = 0.f;   // ȭ��Ʈ ������ ���� �ð� ī��Ʈ
	_vector  m_PrevWorldDelta = XMVectorZero();
	_vector  m_PrevWorldRotation = XMVectorZero();
	_float   m_fRotSmoothSpeed = 8.0f;
	_float   m_fSmoothSpeed = 8.0f;
	_float   m_fSmoothThreshold = 0.05f;
	_float   m_fSlideClamp = 0.2f;
	_float   m_fWalkSpeed = 3.f;
	_float   m_fRunSpeed = 6.f;
	_float   m_fRootMotionAddtiveScale =1.2f; // ��Ʈ ��� �߰� ����
	_float   m_fChasingDistance = 1.5f; // �÷��̾� ���� �Ÿ�

	_float   m_fChangeMoveDirCooldown = 0.f; // �̵� ���� ���� ��Ÿ��
	_float   m_fAddtiveRotSpeed = 1.f; // ȸ�� �ӵ� �߰���
	_float   m_fTurnTimeDuringAttack = 0.f;

#ifdef _DEBUG
	_bool m_bDebugMode = false;
#endif // _DEBUG


	// ���� ����
	//_int   m_iPatternLimit = 3;
	//_float m_fBasePatternWeight = 100.f;
	//_float m_fMinWeight = 50.f;
	//_float m_fMaxWeight = 150.f;
	//_float m_fWeightDecreaseRate = 0.15f;
	//_float m_fWeightIncreaseRate = 0.12f;
	_float   m_fAttackCooldown = 0.f; // ���� ��Ÿ��
	_float   m_fAttckDleay = 4.f;
	_bool    m_bCutSceneOn = false;

	EBossAttackType m_eBossAttackType = EBossAttackType::NONE;

	unordered_map<_int, vector<_wstring>> m_EffectMap; // ����Ʈ �̸� �� (����, �̸�)
	list<pair<_wstring, _bool>> m_ActiveEffect; // Ȱ��ȭ�� ����Ʈ (�̸�, �ѹ��� ��������)

	static constexpr _float MINIMUM_TURN_ANGLE = 35.f;

	_bool m_isGroggy = {};

	CUI_MonsterHP_Bar* m_pHPBar = { nullptr };

public:
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END

