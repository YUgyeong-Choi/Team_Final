#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "TestAnimObject.h"

CTestAnimObject::CTestAnimObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pAnimator(nullptr)
	, m_pShaderCom(nullptr)
{
}
CTestAnimObject::CTestAnimObject(const CTestAnimObject& Prototype)
	: CGameObject(Prototype)
	, m_pAnimator(Prototype.m_pAnimator)
	, m_pShaderCom(Prototype.m_pShaderCom)
{
	Safe_AddRef(m_pAnimator);
	Safe_AddRef(m_pShaderCom);
}
HRESULT CTestAnimObject::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CTestAnimObject::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC GameObjectDesc = {};
	GameObjectDesc.fSpeedPerSec = 30.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.f);

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

    return S_OK;
}

void CTestAnimObject::Update(_float fTimeDelta)
{
	if (m_pAnimator)
	{
		m_pAnimator->Update(fTimeDelta);
	}
	if (m_pModelCom)
	{
		m_pModelCom->Play_Animation();
	}
	Input_Test(fTimeDelta);
}

void CTestAnimObject::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CTestAnimObject::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;
    return S_OK;
}

HRESULT CTestAnimObject::Bind_Shader()
{
	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{
			// 플레이어 모델이 눈이 연결이 안되는 경우가 있어서 return 뺌
		}
		//	return E_FAIL;

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

    return S_OK;
}

HRESULT CTestAnimObject::Ready_Components()
{

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_TestAnimObject"),TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;

	return S_OK;
}

void CTestAnimObject::Input_Test(_float fTimeDelta)
{
}

CTestAnimObject* CTestAnimObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTestAnimObject* pInstance = new CTestAnimObject(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTestAnimObject");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTestAnimObject::Clone(void* pArg)
{
	CTestAnimObject* pInstance = new CTestAnimObject(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTestAnimObject");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTestAnimObject::Free()
{
	__super::Free();
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
}
