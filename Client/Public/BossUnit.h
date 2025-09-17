#pragma once
#include "EliteUnit.h"

NS_BEGIN(Client)
class CBossUnit : public CEliteUnit
{
public:
	enum class EFuryState
	{
		None,   // 평상시
		Fury    // 퓨리 상태
	};
protected:
	CBossUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossUnit(const CBossUnit& Prototype);
	virtual ~CBossUnit() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;

public:
	virtual void EnterCutScene();
	virtual void Reset() override;
	EFuryState GetFuryState() const { return m_eFuryState; }
	_bool HasCollided() const { return m_bPlayerCollided; }
protected:
	virtual void Ready_AttackPatternWeightForPhase1();
	virtual void Ready_AttackPatternWeightForPhase2();

protected:
	HRESULT Spawn_Decal(CBone* pBone, const wstring& NormalTag, const wstring& MaskTag);

	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
protected:
	_bool    m_bCutSceneOn = false;
	_bool    m_bIsPhase2{ false };
	_bool    m_bStartPhase2 = false;
	_bool	 m_bPlayerCollided = false;
	_float   m_fPhase2HPThreshold = 0.6f; // 60% 이하로 떨어지면 페이즈2 시작
	_float   m_fFirstChaseBeforeAttack = 2.f;

	EFuryState m_eFuryState = EFuryState::None;

	class CSpringBoneSys* m_pSpringBoneSys = { nullptr };

	const _float DAMAGE_LIGHT = 35.f;
	const _float DAMAGE_MEDIUM = 40.f;
	const _float DAMAGE_HEAVY = 45.f;
	const _float DAMAGE_FURY = 55.f;
public:
	static CBossUnit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END
