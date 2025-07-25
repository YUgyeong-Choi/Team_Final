#include "Light_Manager.h"
#include "Light.h"

CLight_Manager::CLight_Manager()
{

}

const LIGHT_DESC* CLight_Manager::Get_Light(_uint iIndex)
{
	auto	iter = m_Lights.begin();

	for (size_t i = 0; i < iIndex; i++)
		++iter;
	
	return (*iter)->Get_LightDesc();
}

HRESULT CLight_Manager::Add_LevelLightData(_uint iLevelIndex, const LIGHT_DESC& LightDesc)
{
	CLight* pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_LevelLights[iLevelIndex].push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Add_Light(const LIGHT_DESC& LightDesc)
{
	CLight* pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	m_Lights.push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Render_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	for (auto& pLight : m_Lights)
		pLight->Render(pShader, pVIBuffer);

	return S_OK;
}

HRESULT CLight_Manager::Render_PBR_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer, _uint Level)
{
	for (auto& pLight : m_LevelLights[Level])
		pLight->PBRRender(pShader, pVIBuffer);

	return S_OK;
}


CLight_Manager* CLight_Manager::Create()
{
	return new CLight_Manager;
}

void CLight_Manager::Free()
{
	__super::Free();

	for (auto& pLight : m_Lights)	
		Safe_Release(pLight);
	m_Lights.clear();

	for (auto& pair : m_LevelLights)
	{
		for (auto& pLight : pair.second)
			Safe_Release(pLight);
	}
	m_LevelLights.clear();
}
