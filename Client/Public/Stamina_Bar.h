

#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Client)

// 기본적으로 bar는 담아줄 대상이 다르면 다르게 만들었음
// 추가적인 조작이 필요하면 여기에 추가?

class CStamina_Bar : public CDynamic_UI
{


private:
	CStamina_Bar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStamina_Bar(const CStamina_Bar& Prototype);
	virtual ~CStamina_Bar() = default;

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
	_float m_fCurrentStamina = {};
	_float m_fMaxStamina = {};
	_float m_fRatio = { 1.f };

	_float m_fScaleRatio = { 1.f };

public:
	static CStamina_Bar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END