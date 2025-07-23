#include "Level_GL.h"
#include "GameInstance.h"

CLevel_GL::CLevel_GL(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
{

}

HRESULT CLevel_GL::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	return S_OK;
}

void CLevel_GL::Update(_float fTimeDelta)
{
	int a = 10;
}

HRESULT CLevel_GL::Render()
{
	SetWindowText(g_hWnd, TEXT("경래 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_GL::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	return S_OK;
}

CLevel_GL* CLevel_GL::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_GL* pInstance = new CLevel_GL(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_GL");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_GL::Free()
{
	__super::Free();

}
