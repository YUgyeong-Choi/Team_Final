#include "PreviewObject.h"

#include "GameInstance.h"

CPreviewObject::CPreviewObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CPreviewObject::CPreviewObject(const CPreviewObject& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CPreviewObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CPreviewObject::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	//미리보기용 렌더타겟
	if (FAILED(m_pGameInstance->Add_RenderTarget(TEXT("Target_Preview"), static_cast<_uint>(g_iWinSizeX), static_cast<_uint>(g_iWinSizeY), DXGI_FORMAT_B8G8R8A8_UNORM, _float4(0.0f, 0.f, 0.f, 0.f))))
		return E_FAIL;
	if (FAILED(m_pGameInstance->Add_MRT(TEXT("MRT_Preview"), TEXT("Target_Preview"))))
		return E_FAIL;

	m_pCameraTransformCom->Set_RotationPreSec(1.f);
	m_pCameraTransformCom->Set_SpeedPreSec(1.f);

	_vector vEye = XMVectorSet(0.f, 10.f, -10.f, 1.f);
	_vector vAt = XMVectorSet(0.f, 0.f, 0.f, 1.f); // 고정 타겟
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_matrix matView = XMMatrixLookAtLH(vEye, vAt, vUp);
	m_pCameraTransformCom->Set_WorldMatrix(matView);

	return S_OK;
}

void CPreviewObject::Priority_Update(_float fTimeDelta)
{

}

void CPreviewObject::Update(_float fTimeDelta)
{


}

void CPreviewObject::Late_Update(_float fTimeDelta)
{
	if (m_pModelCom == nullptr)
		return;

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CPreviewObject::Render()
{
	//중간에 프리뷰용 타겟 변경

	m_pGameInstance->End_MRT();

	m_pGameInstance->Begin_MRT(TEXT("MRT_Preview"));

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);

		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		m_pShaderCom->Begin(3);

		m_pModelCom->Render(i);
	}

	m_pGameInstance->End_MRT();

	//타겟 복원
	m_pGameInstance->Begin_MRT(TEXT("MRT_GameObjects"), nullptr, false);

	return S_OK;
}

HRESULT CPreviewObject::Change_Model(wstring ModelPrototypeTag)
{
	//현재 들고 있는 모델을 바꾼다.
	m_ModelPrototypeTag = ModelPrototypeTag;

	Safe_Release(m_pModelCom);

	//컴포넌트 리무브
	Remove_Component(TEXT("Com_Model"));

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), m_ModelPrototypeTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CPreviewObject::Ready_Components(void* pArg)
{

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	///* For.Com_Model */
	//if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Model_SM_Station_Light_01"),
	//	TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
	//	return E_FAIL;

	/* For.Com_CameraTransformCom */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Transform"),
		TEXT("Com_CameraTransformCom"), reinterpret_cast<CComponent**>(&m_pCameraTransformCom))))
		return E_FAIL;
	

	return S_OK;
}

HRESULT CPreviewObject::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pCameraTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

CPreviewObject* CPreviewObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CPreviewObject* pInstance = new CPreviewObject(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPreviewObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CPreviewObject::Clone(void* pArg)
{
	CPreviewObject* pInstance = new CPreviewObject(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPreviewObject");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CPreviewObject::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pCameraTransformCom);

	if (m_bCloned)
	{
		m_pGameInstance->Delete_RenderTarget(TEXT("Target_Preview"));
		m_pGameInstance->Delete_MRT(TEXT("MRT_Preview"));
	}
}
