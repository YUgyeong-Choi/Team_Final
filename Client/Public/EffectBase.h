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
	}DESC;

	enum INTERPOLATION {
		INTERPOLATION_LERP,
		INTERPOLATION_EASEOUTBACK,
		INTERPOLATION_EASEOUTCUBIC,
		INTERPOLATION_EASEINQUAD,
		INTERPOLATION_EASEOUTQUAD,
		INTERPOLATION_END
	};

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

protected:
	const _float4x4*		m_pSocketMatrix = { nullptr };

protected:
	// TrackPositions
	_int				m_iDuration = {10};
	_int				m_iStartTrackPosition = {};
	_int				m_iEndTrackPosition = {20};
	_float				m_fCurrentTrackPosition = {};
	_float				m_fTickPerSecond = {};
	_float				m_fTickAcc = {};

	// Keyframes
	typedef struct tagEffectKeyFrame
	{
		_float3			vScale;
		_float4			vRotation;
		_float3			vTranslation;

		_float			fTrackPosition;
		INTERPOLATION	eInterpolationType = { INTERPOLATION_LERP };
	}EFFKEYFRAME;

	_uint				m_iNumKeyFrames = {};
	_uint				m_iCurKeyFrameIndex = {};
	vector<EFFKEYFRAME>	m_KeyFrames;

	// UV Grid variables
	_int				m_iTileX{1}, m_iTileY{1};
	_int				m_iTileCnt{1};
	_int				m_iTileIdx{0};
	_float2				m_fTileSize{};
	_float2				m_fOffset{};

public:
	vector<EFFKEYFRAME>& Get_KeyFrames() { return m_KeyFrames; }

	_int* Get_Duration_Ptr() { m_iDuration = m_iEndTrackPosition - m_iStartTrackPosition; return &m_iDuration; }
	_int* Get_EndTrackPosition_Ptr() { return &m_iEndTrackPosition; }
	_int* Get_StartTrackPosition_Ptr() { return &m_iStartTrackPosition; }

	void Set_KeyFrames(EFFKEYFRAME tNewKeyframe);
	HRESULT Set_InterpolationType(_uint iKeyFrameIndex, INTERPOLATION eType) { 
		if (iKeyFrameIndex >= m_KeyFrames.size())
			return E_FAIL;
		m_KeyFrames[iKeyFrameIndex].eInterpolationType = eType; return S_OK;
	}
	void Reset_TrackPosition() { m_iCurKeyFrameIndex = 0; m_fCurrentTrackPosition = 0.f; m_fTickAcc = 0.f; }

public:
	virtual CGameObject* Clone(void* pArg) PURE;
	virtual void Free() override;

};

NS_END