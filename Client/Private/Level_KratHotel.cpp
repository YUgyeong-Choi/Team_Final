#include "Level_KratHotel.h"
#include "GameInstance.h"
#include "Camera_Manager.h"

#include "Level_Loading.h"

CLevel_KratHotel::CLevel_KratHotel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_KratHotel::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;
	if (FAILED(Ready_Camera()))
		return E_FAIL;
	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	/* [ 사운드 ] */
	m_pBGM = m_pGameInstance->Get_Single_Sound("LiesOfP");
	m_pBGM->Set_Volume(1.f);
	m_pBGM->Play();

	return S_OK;
}

void CLevel_KratHotel::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
			return;
	}

	m_pCamera_Manager->Update(fTimeDelta);
	__super::Update(fTimeDelta);
}

HRESULT CLevel_KratHotel::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이 레벨입니다."));

	return S_OK;
}

HRESULT CLevel_KratHotel::Ready_Lights()
{
	LIGHT_DESC			LightDesc{};

	LightDesc.eType = LIGHT_DESC::TYPE_DIRECTIONAL;
	LightDesc.vDirection = _float4(1.f, -1.f, 1.f, 0.f);	
	LightDesc.vDiffuse = _float4(0.6f, 0.6f, 0.6f, 1.f);
	LightDesc.fAmbient = 0.2f;
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	
	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;


	/*LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(1.f, 0.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;

	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(15.f, 5.0f, 10.f, 1.f);
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(0.f, 1.f, 0.f, 1.f);
	LightDesc.fAmbient = 0.4f;
	LightDesc.vSpecular = _float4(0.f, 1.f, 0.f, 1.f);

	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
		return E_FAIL;*/


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

HRESULT CLevel_KratHotel::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}


HRESULT CLevel_KratHotel::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::KRAT_HOTEL), strLayerTag)))
		return E_FAIL;

	return S_OK;
}

CLevel_KratHotel* CLevel_KratHotel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_KratHotel* pInstance = new CLevel_KratHotel(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_KratHotel");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_KratHotel::Free()
{
	__super::Free();

	if (m_pBGM)
	{
		m_pBGM->Stop();
		Safe_Release(m_pBGM);
	}
}
