#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CStargazerEffect final : public CGameObject
{
public:
	typedef struct tagStargazerEffectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		class CStargazer* pOwner = { nullptr };

	}DESC;

	enum class STARGAZER_STATUS { DEACTIVATE, ACTIVATE };

private:
	CStargazerEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStargazerEffect(const CStargazerEffect& Prototype);
	virtual ~CStargazerEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Activate_Stargazer_Reassemble();
	void Activate_Stargazer_Spread();

private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Components();
	HRESULT Ready_Effect();

private:
	class CStargazer*		m_pOwner = { nullptr };

	class CEffectContainer* m_pFloatingEffect = { nullptr };
	class CEffectContainer* m_pButterflyEffect[3] = {nullptr};
	CTransform*				m_pButterflyTrans[3] = { nullptr };

	STARGAZER_STATUS		m_eStatus = { STARGAZER_STATUS::DEACTIVATE };

	_float					m_fFlyingParticleTicker = {};
	_float					m_fFlyingParticleInterval = {0.7f};

public:
	static CStargazerEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END