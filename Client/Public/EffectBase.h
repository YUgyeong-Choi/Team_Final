#pragma once

#include "Client_Defines.h"
#include "BlendObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
NS_END

NS_BEGIN(Client)

// ��� ����Ʈ�� �θ� �ǰ���� Ŭ����
class CEffectBase abstract : public CBlendObject // ���������Ʈ�� ��ӹ����� ������ RG_BLEND�� �ƴ�
{
public:
	typedef struct tagEffectBaseDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _float4x4*	pSocketMatrix = { nullptr };
	}DESC;
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

public:
	const vector<KEYFRAME>& Get_KeyFrames() { return m_KeyFrames; }
	_int* Get_Duration_Ptr() { return &m_iDuration; }
	_int* Get_StartTrackPosition_Ptr() { return &m_iStartTrackPosition; }

	void Set_KeyFrames(KEYFRAME tNewKeyframe);

protected:
	CShader*		m_pShaderCom = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };

protected:
	_float4x4*		m_pSocketMatrix = { nullptr };

protected:
	// TrackPosition, Keyframes
	_int				m_iDuration = {10};
	_int				m_iStartTrackPosition = {};
	_float				m_fTickPerSecond = {};
	_float				m_fCurrentTrackPosition = {};
	_uint				m_iNumKeyFrames = {};
	vector<KEYFRAME>	m_KeyFrames;

public:
	virtual CGameObject* Clone(void* pArg) PURE;
	virtual void Free() override;

};

NS_END