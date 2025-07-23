#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Engine)
class CShader;
NS_END

NS_BEGIN(Client)

class CCYTool final : public CGameObject
{
private:
	CCYTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCYTool(const CCYTool& Prototype);
	virtual ~CCYTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Render_EffectTool();
	HRESULT Make_Particles();
	HRESULT Test_Sequence();

	HRESULT Save_Particles();
	HRESULT Load_Particles();

	void Key_Input();

private:
	CShader*				m_pShader = { nullptr };

	_bool					m_isParticlePreview = { false };

	enum PARTICLE_TYPE { PTYPE_SPREAD, PTYPE_DROP, PTYPE_SPDROP, PTYPE_DROPGRAV, PTYPE_END };
	PARTICLE_TYPE			m_eParticleType = PTYPE_END;

	_float3					m_vPivot = { 0.f, 0.f, 0.f };
	_float2					m_vLifeTime = { 2.f, 4.f };
	_float2					m_vSpeed = { 5.f, 10.f };
	_bool					m_isLoop = { true };
	_int					m_iNumInstance = { 100 };
	_float3					m_vRange = { 10.f, 10.f, 30.f };
	_float2					m_vSize = { 1.f, 2.f };
	_float3					m_vCenter = { 0.f, 0.f, 0.f };
	class CToolParticle* m_pToolParticle = { nullptr };

	/******************************/

	class CEffectSequence* m_pSequence = { nullptr };

public:
	static CCYTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END