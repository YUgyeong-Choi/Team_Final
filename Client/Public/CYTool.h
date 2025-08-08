#pragma once
#include "GameObject.h"

#include "Client_Defines.h"
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
	//HRESULT Make_Particles();
	HRESULT SequenceWindow();
	HRESULT Edit_Preferences();

	HRESULT Window_Sprite();
	HRESULT Window_Particle();
	HRESULT Window_Mesh();

	void Edit_Keyframes(class CEffectBase* pEffect);

	HRESULT Load_EffectModel();
	HRESULT Make_EffectModel_Prototypes(const string pModelFilePath);

	HRESULT Save_EffectSet();
	HRESULT Load_EffectSet();

	HRESULT Save_Effect();
	HRESULT Load_Effect();

	HRESULT Load_Textures();
	HRESULT Draw_TextureBrowser(class CEffectBase* pEffect);

	HRESULT Guizmo_Tool();

	void Key_Input();

private:
	CShader*				m_pShader = { nullptr };

private:
#pragma region Tool
	// 툴 용 변수들
	typedef struct TextureItem {
		const ID3D11ShaderResourceView* pSRV;
		string name;
	}EFFTEXTURE;
	vector<EFFTEXTURE>			m_Textures;
	_int						m_iSelectedTextureIdx = { -1 };

	_bool						m_bOpenSaveEffectOnly = { false };
	_bool						m_bOpenSaveEffectContainer = { false };
	_bool						m_bOpenLoadEffectOnly = { false };
	_bool						m_bOpenLoadEffectContainer = { false };

	const ID3D11ShaderResourceView* m_pSlotSRV[4] = {};
	string						m_SlotTexNames[4] = {};

	SPRITEEFFECT_PASS_INDEX		m_eSelectedPass_SE = { SE_DEFAULT };
	MESHEFFECT_PASS_INDEX		m_eSelectedPass_ME = { ME_DEFAULT };
	PARTICLEEFFECT_PASS_INDEX	m_eSelectedPass_PE = { PE_DEFAULT };

	_bool						m_isGizmoEnable = { false };
	ImGuizmo::OPERATION			m_eOperation = { ImGuizmo::TRANSLATE };
	ImGuizmo::MODE				m_eMode = { ImGuizmo::WORLD };
	_int						m_iSelectedKeyframe = { 0 };

	_uint						m_iRenderGroup = ENUM_CLASS(RENDERGROUP::RG_EFFECT_WB);
#pragma endregion


#pragma region BaseEffect
	// 공통 이펙트 용 변수들
	_int						m_iSelectedInterpolationType = { 0 };
	const _char*				m_InterpolationTypes[5] = { "Lerp(Default)", "EaseOutBack", "EaseOutCubic", "EaseInQuad", "EaseOutQuad" };
	_bool						m_bAnimateSprite = { false };

#pragma endregion

#pragma region Sprite
	// 스프라이트 이펙트 용 변수들
	//_int	m_iGridTileX = { 1 };
	//_int	m_iGridTileY = { 1 };

#pragma endregion

#pragma region Mesh
	// 메쉬 이펙트 용 변수들
	vector<string>			m_ModelNames;
	_int					m_iSelectedModelIdx = { -1 };
#pragma endregion


#pragma region Particle
	// 파티클 용 변수들
	_bool					m_isParticlePreview = { false };
	PARTICLETYPE			m_eParticleType = PTYPE_SPREAD;

	_float3					m_vPivot = { 0.f, 0.f, 0.f };
	_float2					m_vLifeTime = { 2.f, 4.f };
	_float2					m_vSpeed = { 5.f, 10.f };
	_bool					m_isLoop = { true };
	_int					m_iNumInstance = { 100 };
	_float3					m_vRange = { 5.f, 1.f, 5.f };
	_float2					m_vSize = { 1.f, 2.f };
	_float3					m_vCenter = { 0.f, 0.f, 0.f };
	_bool					m_bGravity = { false };
	_float					m_fGravity = { 9.8f };
	_bool					m_bOrbit = { false };
	_float					m_fRotationSpeed = {};
	class CToolParticle*	m_pToolParticle = { nullptr };

#pragma endregion

#pragma region ImSequence
	// 시퀀스 용 변수들

	class CEffectSequence* m_pSequence = { nullptr };

	_int		m_iCurFrame = {};
	_float		m_fCurFrame = {};
	_bool		m_bExpanded = { true };
	_int		m_iSelected = { -1 };
	_int		m_iFirstFrame = { 0 };
	_uint		m_iSeqItemColor = { D3DCOLOR_ARGB(255, 200, 60, 40) };
	EFFECT_TYPE	m_eEffectType = EFF_SPRITE;
	string		m_strSeqItemName = "Sprite";

	_bool		m_bPlaySequence = { false };
	_float		m_fTickPerSecond = { 60.f };

	_float		m_fTimeAcc = {};

#pragma endregion


public:
	static CCYTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END