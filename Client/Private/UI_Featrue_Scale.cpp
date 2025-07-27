#include "UI_Featrue_Scale.h"
#include "Shader.h"

CUI_Featrue_Scale::CUI_Featrue_Scale(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)	
	:CUI_Feature{pDevice, pContext}
{
}

CUI_Featrue_Scale::CUI_Featrue_Scale(const CUI_Featrue_Scale& Prototype)
	:CUI_Feature{Prototype}
{
}

HRESULT CUI_Featrue_Scale::Initialize_Prototype()
{
	return E_NOTIMPL;
}

HRESULT CUI_Featrue_Scale::Initialize(void* pArg)
{
	return E_NOTIMPL;
}

void CUI_Featrue_Scale::Update(_int& iCurrentFrame, CDynamic_UI* pUI)
{
}

HRESULT CUI_Featrue_Scale::Bind_ShaderResources(CShader* pShader)
{
	return E_NOTIMPL;
}

UI_FEATURE_TOOL_DESC CUI_Featrue_Scale::Get_Desc_From_Tool()
{
	return UI_FEATURE_TOOL_DESC();
}

UI_FEATRE_DESC& CUI_Featrue_Scale::Get_Desc()
{
	// TODO: 여기에 return 문을 삽입합니다.
	return m_eDesc;
}

CUI_Featrue_Scale* CUI_Featrue_Scale::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	return nullptr;
}

CComponent* CUI_Featrue_Scale::Clone(void* pArg)
{
	return nullptr;
}

void CUI_Featrue_Scale::Free()
{
}
