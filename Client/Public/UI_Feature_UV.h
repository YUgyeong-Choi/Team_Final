#pragma once

#include "Client_Defines.h"

#include "UI_Feature.h"

NS_BEGIN(Client)

class CUI_Feature_UV final : public CUI_Feature
{
private:
	CUI_Feature_UV(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Feature_UV(const CUI_Feature_UV& Prototype);
	virtual ~CUI_Feature_UV() = default;



public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void Update(_int& iCurrentFrame, CDynamic_UI* pUI);
	virtual HRESULT Bind_ShaderResources(class CShader* pShader);

	virtual UI_FEATURE_TOOL_DESC Get_Desc_From_Tool() override;
	virtual UI_FEATRE_DESC& Get_Desc() override;

private:
	// uv
	_float2 m_fUV = {};
	_float2 m_fOffsetUV = {};
	// 가로 세로 몇개 있는지
	_int    m_iWidth = {};
	_int    m_iHeight = {};

	UI_FEATURE_UV_DESC m_eDesc = {};
	
public:

	static CUI_Feature_UV* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


NS_END