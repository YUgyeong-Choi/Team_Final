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
		/* 빛정보를 쉐이더에 던진다. */
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
	/* [ PBR 전용 렌더 ] */
	_uint iPassIndex = {};

	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		/* 빛정보를 쉐이더에 던진다. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 7;
	}
	else if (LIGHT_DESC::TYPE_SPOT == m_LightDesc.eType)
	{
		/* 빛정보를 쉐이더에 던진다. */
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
	return S_OK;
	/* [ 볼륨메트릭 전용 렌더 ] */
	if(!m_LightDesc.bIsVolumetric)
		return S_OK; // 볼륨메트릭이 아닌 라이트는 렌더링하지 않는다.

	_uint iPassIndex = {};

	if (LIGHT_DESC::TYPE_DIRECTIONAL == m_LightDesc.eType)
	{
		/* 빛정보를 쉐이더에 던진다. */
		if (FAILED(pShader->Bind_RawValue("g_vLightDir", &m_LightDesc.vDirection, sizeof(_float4))))
			return E_FAIL;

		iPassIndex = 10;
	}
	else if (LIGHT_DESC::TYPE_SPOT == m_LightDesc.eType)
	{
		if (m_LightDesc.bIsPlayerFar)
			return S_OK; // 볼륨 메트리는 맞지만 플레이어가 멀리 있지 않으면 렌더링하지 않는다.

		_vector vLightPos = XMLoadFloat4(&m_LightDesc.vPosition);
		_vector vLightDir = XMVector3Normalize(XMLoadFloat4(&m_LightDesc.vDirection));
		_float  fRange = 2000.f;

		_vector vFrustumTestPos = vLightPos + vLightDir * (fRange * 0.5f);
		_float  fFrustumRadius = fRange * 0.5f;

		if (!m_pGameInstance->isIn_Frustum_WorldSpace(vFrustumTestPos, fFrustumRadius))
			return S_OK;

		/* 빛정보를 쉐이더에 던진다. */
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

		iPassIndex = 12;
	}
	else
	{
		if (m_LightDesc.bIsPlayerFar)
			return S_OK; // 볼륨 메트리는 맞지만 플레이어가 멀리 있지 않으면 렌더링하지 않는다.

		_vector vPosition = XMLoadFloat4(&m_LightDesc.vPosition);
		if (!m_pGameInstance->isIn_Frustum_WorldSpace(vPosition, 1.f))
			return S_OK;

		if (FAILED(pShader->Bind_RawValue("g_vLightPos", &m_LightDesc.vPosition, sizeof(_float4))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fLightRange", &m_LightDesc.fRange, sizeof(_float))))
			return E_FAIL;
		if (FAILED(pShader->Bind_RawValue("g_fFogCutoff", &m_LightDesc.fFogCutoff, sizeof(_float))))
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

	if (FAILED(pShader->Bind_RawValue("g_fFogPower", &m_LightDesc.fFogDensity, sizeof(_float))))
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
