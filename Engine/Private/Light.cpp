#include "Light.h"

#include "GameInstance.h"


CLight::CLight()
{
}

HRESULT CLight::Initialize(const LIGHT_DESC& LightDesc)
{
	m_pGameInstance = CGameInstance::Get_Instance();
	m_LightDesc = LightDesc;
	
    return S_OK;
}

HRESULT CLight::Render(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	_uint			iPassIndex = {};

	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		/* ºûÁ¤º¸¸¦ ½¦ÀÌ´õ¿¡ ´øÁø´Ù. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 1;
	}
	else
	{
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = 2;
	}


	if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fLightAmbient", &m_LightDesc.fAmbient, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Bind_Buffers();
	pVIBuffer->Render();

	return S_OK;
}
HRESULT CLight::PBRRender(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	/* [ PBR Àü¿ë ·»´õ ] */
	_uint iPassIndex = {};

	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		/* ºûÁ¤º¸¸¦ ½¦ÀÌ´õ¿¡ ´øÁø´Ù. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 7;
	}
	else if (LIGHT_DESC::TYPE_SPOT == m_LightDesc.eType)
	{
		if (!m_LightDesc.bIsUse)
			return S_OK;

		/* ºûÁ¤º¸¸¦ ½¦ÀÌ´õ¿¡ ´øÁø´Ù. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fInnerCosAngle", &m_LightDesc.fInnerCosAngle, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fOuterCosAngle", &m_LightDesc.fOuterCosAngle, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFalloff", &m_LightDesc.fFalloff, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 11;
	}
	else
	{
		if (!m_LightDesc.bIsUse)
			return S_OK;

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;

		iPassIndex = 6;
	}


	if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fLightAmbient", &m_LightDesc.fAmbient, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fLightIntencity", &m_LightDesc.fIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Bind_Buffers();
	pVIBuffer->Render();

	return S_OK;
}

HRESULT CLight::VolumetricRender(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	/* [ º¼·ý¸ÞÆ®¸¯ Àü¿ë ·»´õ ] */
	if(!m_LightDesc.bIsVolumetric)
		return S_OK; // º¼·ý¸ÞÆ®¸¯ÀÌ ²¨Áö¸é ·»´õ¸µÇÏÁö ¾Ê´Â´Ù.

	_uint iPassIndex = {};
	AREAMGR eArea = m_pGameInstance->GetCurrentAreaMgr();

	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		if (eArea == AREAMGR::FUOCO || eArea == AREAMGR::OUTER || eArea == AREAMGR::FESTIVAL)
			return S_OK;

		AREAMGR eAreaMgr = m_pGameInstance->GetCurrentAreaMgr();
		if (eAreaMgr == AREAMGR::HOTEL)
		{
			_float fFogDensity = 0.2f;
			if (FAILED(pShader->Bind_RawValue("g_fFogPower", &fFogDensity, sizeof(_float))))
				return E_FAIL;
		}
		else if (eAreaMgr == AREAMGR::STATION)
		{
			_float fFogDensity = 0.4f;
			if (FAILED(pShader->Bind_RawValue("g_fFogPower", &fFogDensity, sizeof(_float))))
				return E_FAIL;
		}
		else if (eAreaMgr == AREAMGR::OUTER || eAreaMgr == AREAMGR::FESTIVAL)
		{
			_float fFogDensity = 0.7f;
			if (FAILED(pShader->Bind_RawValue("g_fFogPower", &fFogDensity, sizeof(_float))))
				return E_FAIL;
		}
			

		/* ºûÁ¤º¸¸¦ ½¦ÀÌ´õ¿¡ ´øÁø´Ù. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 10;
	}
	else if (LIGHT_DESC::TYPE_SPOT == m_LightDesc.eType)
	{
		if (!m_LightDesc.bIsUse)
			return S_OK;

		if (m_LightDesc.bIsPlayerFar && eArea != AREAMGR::OUTER && eArea != AREAMGR::FESTIVAL)
			return S_OK;

		/* ºûÁ¤º¸¸¦ ½¦ÀÌ´õ¿¡ ´øÁø´Ù. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;		
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fInnerCosAngle", &m_LightDesc.fInnerCosAngle, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fOuterCosAngle", &m_LightDesc.fOuterCosAngle, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFalloff", &m_LightDesc.fFalloff, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFogCutoff", &m_LightDesc.fFogCutoff, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFogPower", &m_LightDesc.fFogDensity, sizeof(_float))))
			return E_FAIL;

		iPassIndex = 12;
	}
	else
	{
		if (!m_LightDesc.bIsUse)
			return S_OK;
		
		if (m_LightDesc.bIsPlayerFar && eArea != AREAMGR::OUTER && eArea != AREAMGR::FESTIVAL)
			return S_OK;

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFogCutoff", &m_LightDesc.fFogCutoff, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFogPower", &m_LightDesc.fFogDensity, sizeof(_float))))
			return E_FAIL;

		iPassIndex = 9;
	}


	if (FAILED(pShader->Bind_RawValue("g_vLightDiffuse", &m_LightDesc.vDiffuse, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fLightAmbient", &m_LightDesc.fAmbient, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_fLightIntencity", &m_LightDesc.fIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(pShader->Bind_RawValue("g_vLightSpecular", &m_LightDesc.vSpecular, sizeof(_float4))))
		return E_FAIL;

	pShader->Begin(iPassIndex);

	pVIBuffer->Bind_Buffers();
	pVIBuffer->Render();

	return S_OK;
}

CLight* CLight::Create(const LIGHT_DESC& LightDesc)
{
	CLight* pInstance = new CLight();

	if (FAILED(pInstance->Initialize(LightDesc)))
	{
		MSG_BOX("Failed to Created : CLight");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLight::Free()
{
    __super::Free();

}
