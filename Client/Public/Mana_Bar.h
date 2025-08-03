#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"

NS_BEGIN(Engine)
class CTexture;
NS_END


NS_BEGIN(Client)

class CMana_Bar : public CDynamic_UI
{


private:
	CMana_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMana_Bar(const CMana_Bar& Prototype);
	virtual ~CMana_Bar() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Component(const wstring& strTextureTag);

private:
	CTexture* m_pBackTextureCom = { nullptr };
	CTexture* m_pGradationCom = { nullptr };

private:
	_int m_iCurrentMana = {};
	_int m_iMaxMana = {300};
	

public:
	static CMana_Bar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END