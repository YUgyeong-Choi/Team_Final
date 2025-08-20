#pragma once

#include "Client_Defines.h"
#include "UIObject.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;

NS_END

NS_BEGIN(Client)

class CUI_MonsterHP_Bar final : public CUIObject
{
public:
	typedef struct tagHpDesc : public  CUIObject::UIOBJECT_DESC
	{
		_float fHeight;
		_float* pHP;
		_bool* pIsGroggy;
		const _float4x4* pParentMatrix;

	}HPBAR_DESC;
private:
	CUI_MonsterHP_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_MonsterHP_Bar(const CUI_MonsterHP_Bar& Prototype);
	virtual ~CUI_MonsterHP_Bar() = default;

public:
	void Set_RenderTime(_float fTime) { m_fRenderTime = fTime; }
	void Set_MaxHp(_float iHp) { m_fMaxHP = iHp; }

	void Add_Damage(_float fDamage) { m_fDamage += fDamage; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CTexture* m_pBlurTextureCom = { nullptr };
	CTexture* m_pGradationCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	_float* m_pHP = { nullptr };
	_bool* m_isGroggy = {nullptr};
	_float  m_fMaxHP = {};
	_float  m_fDamage = {};

	_float m_fRenderTime = { };

	const _float4x4* m_pParentMatrix = { nullptr };
	_float4x4				m_CombinedWorldMatrix{};



public:
	static CUI_MonsterHP_Bar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END