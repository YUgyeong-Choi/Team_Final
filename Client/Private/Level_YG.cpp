#include "Level_YG.h"
#include "GameInstance.h"
#include "YGTool.h"
#include "Camera_Manager.h"
#include "Level_Loading.h"
CLevel_YG::CLevel_YG(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
		: CLevel { pDevice, pContext }
	, m_pCamera_Manager{ CCamera_Manager::Get_Instance() }
{

}

HRESULT CLevel_YG::Initialize()
{
	if (FAILED(Ready_Lights()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Sky(TEXT("Layer_Sky"))))
		return E_FAIL;

	if (FAILED(Ready_Camera()))
		return E_FAIL;

	if (FAILED(Ready_Layer_Object(TEXT("Layer_YG"))))
		return E_FAIL;

	return S_OK;
}

void CLevel_YG::Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_F1))
	{
		m_pCamera_Manager->SetPlayer(nullptr);
		if (SUCCEEDED(m_pGameInstance->Change_Level(static_cast<_uint>(LEVEL::LOADING), CLevel_Loading::Create(m_pDevice, m_pContext, LEVEL::LOGO))))
		{
			return;
		}
			
	}

	m_pCamera_Manager->Update(fTimeDelta);

	__super::Update(fTimeDelta);
}

HRESULT CLevel_YG::Render()
{
	SetWindowText(g_hWnd, TEXT("유경 레벨입니다."));

	_wstring text = L"제일 왼쪽: Capsule";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 30.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"왼쪽 : TriangleMesh";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 60.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"가운곳 : ConvexMesh <- 지 맘대로 콜라이더 만들어서 간단하게 생긴 오브젝트한테만 해야함";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 90.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"오른쪽 : Box";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 120.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));

	text = L"제일 오른쪽 : Shere";
	m_pGameInstance->Draw_Font(TEXT("Font_151"), text.c_str(), _float2(0.f, 150.f), XMVectorSet(0.f, 0.f, 0.f, 1.f));
	
	return S_OK;
}

HRESULT CLevel_YG::Ready_Camera()
{
	m_pCamera_Manager->Initialize(LEVEL::STATIC);
	m_pCamera_Manager->SetFreeCam();

	return S_OK;
}

HRESULT CLevel_YG::Ready_Lights()
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

HRESULT CLevel_YG::Ready_Layer_Object(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGObject"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGCapsule"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTriangleMesh"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGConvexMesh"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGBox"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGShpere"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;

	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_GameObject_YGTrrigerWithoutModel"),
		ENUM_CLASS(LEVEL::YG), strLayerTag)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CLevel_YG::Ready_Layer_Sky(const _wstring strLayerTag)
{
	if (FAILED(m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Sky"),
		ENUM_CLASS(LEVEL::STATIC), strLayerTag)))
		return E_FAIL;

	return S_OK;
}


CLevel_YG* CLevel_YG::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLevel_YG* pInstance = new CLevel_YG(pDevice, pContext);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CLevel_YG");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CLevel_YG::Free()
{
	__super::Free();
}
