#pragma once

#include "Client_Defines.h"
#include "Dynamic_UI.h"

NS_BEGIN(Engine)
class CTexture;
NS_END


NS_BEGIN(Client)

// 비율 대로 늘어나게 수정할거

class CUI_Bar_Loading : public CDynamic_UI
{


private:
	CUI_Bar_Loading(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Bar_Loading(const CUI_Bar_Loading& Prototype);
	virtual ~CUI_Bar_Loading() = default;

public:
	_float Get_Ratio() { return m_fRatio; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Update_From_Feature_Ratio(atomic<_float>& fRatio);


private:
	_int m_iRange = { 120 };
	_float m_fRatio = { };

public:
	static CUI_Bar_Loading* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END