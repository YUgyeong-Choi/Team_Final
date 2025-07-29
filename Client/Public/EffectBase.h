#pragma once

#include "Client_Defines.h"
#include "BlendObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
NS_END

NS_BEGIN(Client)

// 모든 이펙트의 부모가 되고싶은 클래스
class CEffectBase abstract : public CBlendObject // 블렌드오브젝트를 상속받지만 무조건 RG_BLEND는 아님
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
		_bool				bTool = { false };
	}DESC;


	// Keyframes
	typedef struct tagEffectKeyFrame
	{
		_float3			vScale = {1.f, 1.f, 1.f};
		_float4			vRotation = { 0.f, 0.f, 0.f, 0.f };
		_float3			vTranslation = {0.f, 0.f, 0.f};
		_float4			vColor = { 1.f, 1.f, 1.f, 1.f };

		_float			fTrackPosition = {};
		INTERPOLATION	eInterpolationType = { INTERPOLATION_LERP };
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

protected:
	/* Keyframe Interpolation */
	void Update_Keyframes();
	_float Interpolate_Ratio(_float fRatio);

protected:
	CShader*		m_pShaderCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };
	CTexture*		m_pMaskTextureCom[2] = { nullptr };

protected:
	const _float4x4*	m_pSocketMatrix = { nullptr };

protected:
	// Basic Effect Preferences
	_float4				m_vColor = { 1.f, 1.f, 1.f, 1.f };
	_float				m_fLifeTime = {};
	_bool				m_bBillboard = { true };
	_bool				m_bAnimation = { true };
	_uint				m_iShaderPass = {};

	// 지금 메쉬만 사용중, 다른 애들도 바꾸고 주석 지우기
	enum TEXUSAGE { TU_DIFFUSE, TU_MASK1, TU_MASK2, TU_END };
	_bool				m_bTextureUsage[TU_END];

	// TrackPositions
	_int				m_iDuration = {10};
	_int				m_iStartTrackPosition = {};
	_int				m_iEndTrackPosition = {20};
	_float				m_fCurrentTrackPosition = {};
	_float				m_fTickPerSecond = {};
	_float				m_fTickAcc = {};

	_uint				m_iNumKeyFrames = {};
	_uint				m_iCurKeyFrameIndex = {};
	vector<EFFKEYFRAME>	m_KeyFrames;

	// UV Grid variables
	_int				m_iTileX{1}, m_iTileY{1};
	_int				m_iTileCnt{1};
	_int				m_iTileIdx{0};
	_float2				m_fTileSize{};
	_float2				m_fOffset{};
	_bool				m_bFlipUV = { false };
	
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
	_bool* Get_Billboard_Ptr() { return &m_bBillboard; }
	_bool* Get_Animation_Ptr() { return &m_bAnimation; }
	_bool* Get_FlipUV_Ptr() { return &m_bFlipUV; }
	void Set_TileXY(_int iX, _int iY) { m_iTileX = iX; m_iTileY = iY; }
	void Set_Billboard(_bool bBillboard) { m_bBillboard = bBillboard; }
	HRESULT Change_Texture(_wstring strTextureName);

#endif

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
	void Reset_TrackPosition() { m_iCurKeyFrameIndex = 0; m_fCurrentTrackPosition = 0.f; m_fTickAcc = 0.f; }

	
	// 키프레임 특성상 항상 인덱스 순서대로 진행되기 때문에 삭제도 무조건 가장 마지막 것 부터 pop_back 합니다
	void Delete_KeyFrame() { m_KeyFrames.pop_back(); }
	void Add_KeyFrame(EFFKEYFRAME tNewKeyframe) { m_KeyFrames.push_back(tNewKeyframe); }

public:
	virtual CGameObject* Clone(void* pArg) PURE;
	virtual void Free() override;

};

NS_END