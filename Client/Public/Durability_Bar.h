#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"

NS_BEGIN(Engine)
class CTexture;
NS_END


NS_BEGIN(Client)

// 기본적으로 bar는 담아줄 대상이 다르면 다르게 만들었음
// 추가적인 조작이 필요하면 여기에 추가?

class CDurability_Bar : public CDynamic_UI
{


private:
	CDurability_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDurability_Bar(const CDurability_Bar& Prototype);
	virtual ~CDurability_Bar() = default;


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Component(const wstring& strTextureTag);


private:
	CTexture* m_pBackTextureCom = { nullptr };
	CTexture* m_pGradationCom = { nullptr };

private:
	_int m_iCurrentHP = {};
	_int m_iMaxHP = {};
	_float m_fRatio = { 1.f };

public:
	static CDurability_Bar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END