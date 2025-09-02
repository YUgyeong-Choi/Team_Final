#pragma once
#include "EliteUnit.h"

NS_BEGIN(Client)
class CBossUnit : public CEliteUnit
{
protected:
	CBossUnit(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossUnit(const CBossUnit& Prototype);
	virtual ~CBossUnit() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;

public:
	void EnterCutScene();
	virtual void Reset() override;
protected:
	virtual void Ready_AttackPatternWeightForPhase1();
	virtual void Ready_AttackPatternWeightForPhase2();

	virtual void On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal);
protected:
	_bool    m_bIsPhase2{ false };
	_bool    m_bStartPhase2 = false;
	_float   m_fPhase2HPThreshold = 0.6f; // 60% 이하로 떨어지면 페이즈2 시작
	_float   m_fFirstChaseBeforeAttack = 2.f;
	_bool    m_bCutSceneOn = false;
public:
	static CBossUnit* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};
NS_END
