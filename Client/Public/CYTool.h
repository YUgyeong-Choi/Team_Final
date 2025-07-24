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
	HRESULT SequenceWindow();
	HRESULT Edit_Preferences();
	HRESULT Window_Sprite();
	HRESULT Window_Particle();
	HRESULT Window_Mesh();

	HRESULT Save_Particles();
	HRESULT Load_Particles();

	void Key_Input();

private:
	CShader*				m_pShader = { nullptr };

private:
#pragma region Sprite
	// 스프라이트 이펙트 용 변수들
	_int	m_iGridWidthCnt = { 1 };
	_int	m_iGridHeightCnt = { 1 };
	_bool	m_bAnimateSprite = { false };

#pragma endregion


#pragma region Particle
	// 파티클 용 변수들
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
	class CToolParticle*	m_pToolParticle = { nullptr };

#pragma endregion

#pragma region ImSequence
	// 시퀀스 용 변수들
	enum EFFECT_TYPE { EFF_SPRITE, EFF_PARTICLE, EFF_MESH, EFF_END };

	class CEffectSequence* m_pSequence = { nullptr };

	_int		m_iCurFrame = {};
	_bool		m_bExpanded = { true };
	_int		m_iSelected = { -1 };
	_int		m_iFirstFrame = { 0 };
	_uint		m_iSeqItemColor = { D3DCOLOR_ARGB(255, 200, 60, 40) };
	EFFECT_TYPE	m_eEffectType = EFF_SPRITE;
	string		m_strSeqItemName = "Sprite";

	_bool		m_bPlaySequence = { false };

	_float		m_fTimeAcc = {};

#pragma endregion


public:
	static CCYTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END