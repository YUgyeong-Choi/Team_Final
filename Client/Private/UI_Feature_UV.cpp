#include "UI_Feature_UV.h"
#include "Shader.h"

CUI_Feature_UV::CUI_Feature_UV(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUI_Feature{ pDevice, pContext }
{
}

CUI_Feature_UV::CUI_Feature_UV(const CUI_Feature_UV& Prototype)
	:CUI_Feature{Prototype}
{
}

HRESULT CUI_Feature_UV::Initialize_Prototype()
{
	
	return S_OK;
}

HRESULT CUI_Feature_UV::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return S_OK;

	UI_FEATURE_UV_DESC* pDesc = static_cast<UI_FEATURE_UV_DESC*>(pArg);

	m_fUV = pDesc->fStartUV;

	m_fOffsetUV = pDesc->fOffsetUV;

	m_iWidth = static_cast<_int>(1 / m_fOffsetUV.x);
	m_iHeight = static_cast<_int>(1 / m_fOffsetUV.y);

	m_isLoop = pDesc->isLoop;
	
	m_iStartFrame = pDesc->iStartFrame;
	m_iEndFrame = pDesc->iEndFrame;
	m_strProtoTag = TEXT("Prototype_Component_UI_Feature_UV");

	return S_OK;
}

void CUI_Feature_UV::Update(_int& iCurrentFrame, CDynamic_UI* pUI)
{
	if (iCurrentFrame < m_iStartFrame)
		return;

	if (m_iWidth == 0 || m_iHeight == 0)
		return;

	if (m_isLoop)
	{
		
		m_iCurrentFrame = iCurrentFrame % (m_iWidth * m_iHeight);
	}
	else
	{
		if (m_iCurrentFrame >= m_iEndFrame)
			return;

		m_iCurrentFrame = iCurrentFrame;

	}


	int iRow = m_iCurrentFrame / m_iWidth;
	int iCol = m_iCurrentFrame % m_iWidth;

	m_fUV.x = iCol * m_fOffsetUV.x;
	m_fUV.y = iRow * m_fOffsetUV.y;

	

	
}

HRESULT CUI_Feature_UV::Bind_ShaderResources(CShader* pShader)
{
	if (FAILED(pShader->Bind_RawValue("g_fTexcoord", &m_fUV, sizeof(_float2))))
		return E_FAIL;

	if (FAILED(pShader->Bind_RawValue("g_fTileSize", &m_fOffsetUV, sizeof(_float2))))
		return E_FAIL;


	return E_NOTIMPL;
}

UI_FEATURE_TOOL_DESC CUI_Feature_UV::Get_Desc_From_Tool()
{
	UI_FEATURE_TOOL_DESC eDesc = {};

	eDesc.isLoop = m_isLoop;
	eDesc.iType = 1;
	eDesc.iStartFrame = m_iStartFrame;
	eDesc.iEndFrame = m_iEndFrame;
	eDesc.fStartUV = {};
	eDesc.fOffsetUV = m_fOffsetUV;
	eDesc.strTypeTag = "UV";

	return eDesc;
}

UI_FEATRE_DESC& CUI_Feature_UV::Get_Desc()
{
	

	m_eDesc.isLoop = m_isLoop;
	m_eDesc.iStartFrame = m_iStartFrame;
	m_eDesc.iEndFrame = m_iEndFrame;
	m_eDesc.fStartUV = {0.f, 0.f};
	m_eDesc.fOffsetUV = m_fOffsetUV;
	m_eDesc.strProtoTag = "Prototype_Component_UI_Feature_UV";

	return m_eDesc;
}

json CUI_Feature_UV::Serialize()
{
	json j = __super::Serialize();


	j["StartUV"]["U"] = m_fUV.x;
	j["StartUV"]["V"] = m_fUV.y;

	j["OffsetUV"]["U"] = m_fOffsetUV.x;
	j["OffsetUV"]["V"] = m_fOffsetUV.y;

	j["FeatureProtoTag"] = "Prototype_Component_UI_Feature_UV";


	return j;
}

void CUI_Feature_UV::Deserialize(const json& j)
{
	__super::Deserialize(j);

	m_fUV = { j["StartUV"]["U"] , j["StartUV"]["V"] };
	m_fOffsetUV = { j["OffsetUV"]["U"] , j["OffsetUV"]["V"] };

	string strPrototag = j["FeatureProtoTag"];
	m_strProtoTag = StringToWStringU8(strPrototag);

	m_iWidth = static_cast<_int>(1 / m_fOffsetUV.x);
	m_iHeight = static_cast<_int>(1 / m_fOffsetUV.y);

}

CUI_Feature_UV* CUI_Feature_UV::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Feature_UV* pInstance = new CUI_Feature_UV(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_Feature_UV");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CUI_Feature_UV::Clone(void* pArg)
{
	CUI_Feature_UV* pInstance = new CUI_Feature_UV(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Feature_UV");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Feature_UV::Free()
{
	__super::Free();

}
