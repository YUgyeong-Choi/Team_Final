#include "Level_JW.h"
#include "GameInstance.h"

CLevel_JW::CLevel_JW(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_JW::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;


	return S_OK;
}

void CLevel_JW::Update(_float fTimeDelta)
{
	int a = 10;
}

HRESULT CLevel_JW::Render()
{
	SetWindowText(g_hWnd, TEXT("장원 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_JW::Ready_Lights()
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

CLevel_JW* CLevel_JW::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_JW* pInstance = new CLevel_JW(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_JW");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_JW::Free()
{
	__super::Free();

}
