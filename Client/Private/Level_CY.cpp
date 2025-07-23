#include "Level_CY.h"
#include "GameInstance.h"

CLevel_CY::CLevel_CY(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_CY::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;


	return S_OK;
}

void CLevel_CY::Update(_float fTimeDelta)
{
	int a = 10;
}

HRESULT CLevel_CY::Render()
{
	SetWindowText(g_hWnd, TEXT("채영 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_CY::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	CShadow::SHADOW_DESC		Desc{};
	Desc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc)))
		return E_FAIL;

	return S_OK;
}

CLevel_CY* CLevel_CY::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_CY* pInstance = new CLevel_CY(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_CY");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_CY::Free()
{
	__super::Free();

}
