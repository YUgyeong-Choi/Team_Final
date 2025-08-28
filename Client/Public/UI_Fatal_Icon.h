#pragma once

#include "Client_Defines.h"
#include "UIObject.h"
#include "Player.h"


NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;

NS_END

NS_BEGIN(Client)

class CUI_Fatal_Icon final : public CUIObject
{
private:
	CUI_Fatal_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Fatal_Icon(const CUI_Fatal_Icon& Prototype);
	virtual ~CUI_Fatal_Icon() = default;

public:
	void Set_isRender(_bool isRender) { m_isRender = isRender; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Find_Player();

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	CPlayer* m_pPlayer = {nullptr};

	CUnit*	m_pTarget = { nullptr };

	_bool	m_isRender = { false };

public:
	static CUI_Fatal_Icon* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END