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

HRESULT CLight_Manager::Add_LevelLightDataReturn(_uint iLevelIndex, const LIGHT_DESC& LightDesc, CLight** ppOut)
{
	CLight* pLight = CLight::Create(LightDesc);
	if (nullptr == pLight)
		return E_FAIL;

	*ppOut = pLight;

	m_LevelLights[iLevelIndex].push_back(pLight);

	return S_OK;
}

HRESULT CLight_Manager::Remove_NoLevelLight()
{
	for (CLight* pLight : m_Lights) {
		Safe_Release(pLight);
	}
	m_Lights.clear();
	return S_OK;
}

HRESULT CLight_Manager::Remove_Light(_uint iLevelIndex, CLight* pLight)
{
	auto& lightList = m_LevelLights[iLevelIndex];
	auto iter = std::find(lightList.begin(), lightList.end(), pLight);
	if (iter != lightList.end())
	{
		Safe_Release(*iter);
		lightList.erase(iter);
	}

	return S_OK;
}

HRESULT CLight_Manager::RemoveAll_Light(_uint iLevelIndex)
{
	auto& lightList = m_LevelLights[iLevelIndex];
	for (CLight* pLight : lightList) {
		Safe_Release(pLight);
	}
	m_LevelLights[iLevelIndex].clear();
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

_uint CLight_Manager::Get_LightCount(_uint TYPE, _uint iLevel)
{
	auto iter = m_LevelLights.find(iLevel);
	_uint Dirrectioncount = 0;
	_uint Pointcount = 0;
	_uint Spotcount = 0;

	if (iter != m_LevelLights.end())
	{
		for (auto& pLight : iter->second)
		{
			if (pLight && pLight->Get_LightDesc()->eType == 0)
				++Dirrectioncount;
			if (pLight && pLight->Get_LightDesc()->eType == 1)
				++Spotcount;
			if (pLight && pLight->Get_LightDesc()->eType == 2)
				++Pointcount;
		}
	}

	if (TYPE == 0)
		return Dirrectioncount;
	if (TYPE == 1)
		return Spotcount;
	if (TYPE == 2)
		return Pointcount;

	return 0;
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
