
#pragma once

#include "Client_Defines.h"

#include "UI_Feature.h"

NS_BEGIN(Client)

class CUI_Feature_Color final : public CUI_Feature
{
private:
	CUI_Feature_Color(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Feature_Color(const CUI_Feature_Color& Prototype);
	virtual ~CUI_Feature_Color() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void Update(_int& iCurrentFrame, CDynamic_UI* pUI, _bool isReverse = false);
	virtual HRESULT Bind_ShaderResources(class CShader* pShader);

	virtual UI_FEATURE_TOOL_DESC Get_Desc_From_Tool() override;
	virtual UI_FEATRE_DESC& Get_Desc() override;

	virtual json Serialize();
	virtual void Deserialize(const json& j);

private:

	
	_float4			m_fCurrentColor = {};
	_float4			m_fStartColor = {};
	_float4			m_fEndColor = {};

	UI_FEATURE_COLOR_DESC m_eDesc = {};

public:

	static CUI_Feature_Color* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


NS_END