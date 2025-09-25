#include "FinalDoor.h"
#include "GameInstance.h"
#include "Player.h"

CFinalDoor::CFinalDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CFinalDoor::CFinalDoor(const CFinalDoor& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CFinalDoor::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CFinalDoor::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	_float3 vPosition = _float3(-0.41f, 0.29f, -177.08f);
	_matrix matWorld = XMMatrixTranslation(vPosition.x, vPosition.y, vPosition.z);
	_float4x4 matWorldFloat;
	XMStoreFloat4x4(&matWorldFloat, matWorld);
	m_pTransformCom->Set_WorldMatrix(matWorldFloat);
	m_pTransformCom->Scaling(_float3(1.7f, 1.7f, 1.7f));
	return S_OK;
}

void CFinalDoor::Priority_Update(_float fTimeDelta)
{
	if (!m_pPlayer)
		m_pPlayer = dynamic_cast<CPlayer*>(GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex()));

	__super::Priority_Update(fTimeDelta);

	if (m_bOpen)
	{
		_vector pos = m_pTransformCom->Get_State(STATE::POSITION);

		float y = XMVectorGetY(pos);

		y += 0.015f;

		if (y >= 5.0f)
		{
			y = 5.0f;
			m_bOpen = false;
		}

		// pos의 Y값 갱신
		pos = XMVectorSetY(pos, y);
		m_pTransformCom->Set_State(STATE::POSITION, pos);
	}

}

void CFinalDoor::Update(_float fTimeDelta)
{

	__super::Update(fTimeDelta);
}

void CFinalDoor::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	__super::Late_Update(fTimeDelta);
}

HRESULT CFinalDoor::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	_float fDissolve = 0.f;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &fDissolve, sizeof(_float))))
		return E_FAIL;

	for (_uint i = 0; i < iNumMesh; i++)
	{
		_float Emissive = 0.f;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &Emissive, sizeof(_float))))
			return E_FAIL;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{
			return E_FAIL;
		}

		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;
				m_bDoOnce = true;
			}

			if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
				return E_FAIL;
		}


		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXActorCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
				return E_FAIL;
		}
	}
#endif

	return S_OK;
}
HRESULT CFinalDoor::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CFinalDoor::Ready_Components(void* pArg)
{
	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(m_pGameInstance->GetCurrentLevelIndex() ,TEXT("Prototype_Component_Model_SM_Station_TrainDoor") ,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;
}

void CFinalDoor::DoorOpen()
{
	m_bOpen = true;
}

CFinalDoor* CFinalDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFinalDoor* pGameInstance = new CFinalDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CFinalDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CFinalDoor::Clone(void* pArg)
{
	CFinalDoor* pGameInstance = new CFinalDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CFinalDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CFinalDoor::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pModelCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pEmissiveCom);
}
