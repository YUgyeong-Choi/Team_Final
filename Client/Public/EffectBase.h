#pragma once

#include "Client_Defines.h"
#include "BlendObject.h"

#include "Serializable.h"
NS_BEGIN(Engine)
class CShader;
class CTexture;
NS_END

NS_BEGIN(Client)

// 모든 이펙트의 부모가 되고싶은 클래스
class CEffectBase abstract : public CBlendObject, public ISerializable // 블렌드오브젝트를 상속받지만 무조건 RG_BLEND는 아님
{
public:
	typedef struct tagEffectBaseDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4*	pSocketMatrix = { nullptr };
		_int				iTileX = {};
		_int				iTileY = {};
		_bool				bBillboard = { true };
		_bool				bAnimation = { true };

		_uint				iShaderPass = { 0 };
		_bool				isLoop = { true };
		_bool				bTool = { false };
	}DESC;

	// Keyframes
	typedef struct tagEffectKeyFrame : public Engine::ISerializable
	{
		_float3			vScale = { 1.f, 1.f, 1.f };
		_float4			vRotation = { 0.f, 0.f, 0.f, 0.f };
		_float3			vTranslation = { 0.f, 0.f, 0.f };
		_float4			vColor = { 1.f, 1.f, 1.f, 1.f };
		_float			fIntensity = { 1.f };

		_float			fTrackPosition = {};
		INTERPOLATION	eInterpolationType = { INTERPOLATION_LERP };
	public:
		virtual json Serialize() override;
		virtual void Deserialize(const json& j) override;
	}EFFKEYFRAME;


protected:
	CEffectBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEffectBase(const CEffectBase& Prototype);
	virtual ~CEffectBase() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Update_Tool(_float fTimeDelta, _float fCurFrame);

	virtual _float Ready_Death() { return 0; }

protected:
	/* Keyframe Interpolation */
	void Update_Keyframes();
	_float Interpolate_Ratio(_float fRatio);

	virtual HRESULT Ready_Components() PURE;
	virtual HRESULT Bind_ShaderResources();

protected:
	CShader*			m_pShaderCom = { nullptr };

	_bool				m_bTextureUsage[TU_END];
	CTexture*			m_pTextureCom[TU_END] = { nullptr };
	_wstring			m_TextureTag[TU_END];

protected:
	const _float4x4*	m_pSocketMatrix = { nullptr };
	_float4x4			m_CombinedWorldMatrix{};

protected:
	// Basic Effect Preferences
	_float				m_fLifeTime = {};
	_bool				m_bBillboard = { true };
	_bool				m_bAnimation = { true };
	_uint				m_iShaderPass = {};
	_uint				m_iRenderGroup = {};
	_bool				m_isLoop = { false };
	_float				m_fTimeAcc = {};
	EFFECT_TYPE			m_eEffectType = { EFF_END };

	// Colors
	_float4				m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float				m_fIntensity = {};
	_float				m_fThreshold = {};
	_float4				m_vCenterColor = { 1.f, 1.f, 1.f, 1.f };
	_float				m_fEmissiveIntensity = { 0.f };
	//_float			m_fUVScrollSpeed = {};


	// TrackPositions
	_int				m_iDuration = {10};
	_int				m_iStartTrackPosition = {};
	_int				m_iEndTrackPosition = {20};
	_float				m_fCurrentTrackPosition = {};
	_float				m_fTickPerSecond = {};

	_uint				m_iNumKeyFrames = {};
	_uint				m_iCurKeyFrameIndex = {};
	vector<EFFKEYFRAME>	m_KeyFrames;

	// UV Grid variables
	_int				m_iTileX = {1}, m_iTileY = {1};
	_int				m_iTileCnt = { 1 };
	_int				m_iTileIdx = { 0 };
	_float				m_fTileIdx = {};
	_float2				m_fTileSize = {};
	_float2				m_fOffset = {};
	_bool				m_bFlipUV = { false };
	_float				m_fTileTickPerSec = { 60.f };
	
	// Tool
	_bool				m_bTool = { false };

public:
	vector<EFFKEYFRAME>& Get_KeyFrames() { return m_KeyFrames; }

#ifdef USE_IMGUI
	_int* Get_Duration_Ptr() { m_iDuration = m_iEndTrackPosition - m_iStartTrackPosition; return &m_iDuration; }
	_int* Get_EndTrackPosition_Ptr() { return &m_iEndTrackPosition; }
	_int* Get_StartTrackPosition_Ptr() { return &m_iStartTrackPosition; }
	_int* Get_TileX() { return &m_iTileX; }
	_int* Get_TileY() { return &m_iTileY; }
	_float* Get_TileTickPerSec() { return &m_fTileTickPerSec; }
	_bool* Get_Billboard_Ptr() { return &m_bBillboard; }
	_bool* Get_Animation_Ptr() { return &m_bAnimation; }
	_bool* Get_FlipUV_Ptr() { return &m_bFlipUV; }
	_float* Get_EmissiveIntensity_Ptr() { return &m_fEmissiveIntensity; }
	_uint* Get_RenderGroup_Ptr() { return &m_iRenderGroup; }
	_uint* Get_ShaderPass_Ptr() { return &m_iShaderPass; }
	void Set_TileXY(_int iX, _int iY) { m_iTileX = iX; m_iTileY = iY; }
	//void Set_Billboard(_bool bBillboard) { m_bBillboard = bBillboard; }
	HRESULT Change_Texture(_wstring strTextureName, TEXUSAGE eTex = TU_DIFFUSE);
	HRESULT Delete_Texture(TEXUSAGE eTex);
	//void Set_ShaderPass(_uint iPass) { m_iShaderPass = iPass; }
	//void Set_RenderGroup(_uint iRG) { m_iRenderGroup = iRG; }

	_float* Get_Threshold() { return &m_fThreshold; }
	_float4* Get_CenterColor() { return &m_vCenterColor; }
	_int Get_StartTrackPosition() { return m_iStartTrackPosition; }
	HRESULT Set_InterpolationType(_uint iKeyFrameIndex, INTERPOLATION eType) {
		if (iKeyFrameIndex >= m_KeyFrames.size())
			return E_FAIL;
		m_KeyFrames[iKeyFrameIndex].eInterpolationType = eType; return S_OK;
	}
	INTERPOLATION Get_InterpolationType(_uint iKeyFrameIndex) {
		if (iKeyFrameIndex >= m_KeyFrames.size())
			return INTERPOLATION_END;
		return m_KeyFrames[iKeyFrameIndex].eInterpolationType;
	}
#endif
	_int Get_EndTrackPosition() { return m_iEndTrackPosition; }
	void Reset_TrackPosition() { m_iCurKeyFrameIndex = 0; m_fCurrentTrackPosition = 0.f; /*m_fTickAcc = 0.f;*/ }

	
	// 키프레임 특성상 항상 인덱스 순서대로 진행되기 때문에 삭제도 무조건 가장 마지막 것 부터 pop_back 합니다
	void Delete_KeyFrame() { m_KeyFrames.pop_back(); }
	void Add_KeyFrame(EFFKEYFRAME tNewKeyframe) { m_KeyFrames.push_back(tNewKeyframe); }

	HRESULT Ready_Textures_Prototype();//이펙트매니저 이전 임시 함수
	HRESULT Ready_Textures_Prototype_Tool();

	virtual HRESULT Ready_Effect_Deserialize(const json& j);

	EFFECT_TYPE Get_EffectType() { return m_eEffectType; }

public:
	virtual CGameObject* Clone(void* pArg) PURE;
	virtual void Free() override;

public:
	virtual json Serialize();
	virtual void Deserialize(const json& j);
};

NS_END