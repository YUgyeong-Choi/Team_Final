#include "BossRetryDoor.h"
#include "GameInstance.h"
#include "Player.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"

CBossRetryDoor::CBossRetryDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CDefaultDoor{ pDevice, pContext }
{

}

CBossRetryDoor::CBossRetryDoor(const CBossRetryDoor& Prototype)
	: CDefaultDoor(Prototype)
{

}

HRESULT CBossRetryDoor::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBossRetryDoor::Initialize(void* pArg)
{
	CBossRetryDoor::RETRYDOOR_DESC* pDoorMeshDESC = static_cast<RETRYDOOR_DESC*>(pArg);

	m_eInteractType = pDoorMeshDESC->eInteractType;

	m_fDissolve = 1.f;
	if (FAILED(Add_Component(0, TEXT("Prototype_Component_Texture_NoiseMap2"),
		TEXT("Dissolve2_Com"), reinterpret_cast<CComponent**>(&m_pDissolveMap))))
		return E_FAIL;


	// 아래 모델 이름으로 소환해야함 (해줘)
	//Prototype_Component_Model_FestivalRetryDoor
	//Prototype_Component_Model_PuocoRetryDoor

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	Ready_Trigger(pDoorMeshDESC);
	m_pPlayer = GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex());
	if (FAILED(LoadFromJson()))
		return E_FAIL;
	return S_OK;
}

void CBossRetryDoor::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (m_pPlayer == nullptr)
		return;

	if (m_bCanActive && !m_bFinish)
	{
		if (KEY_DOWN(DIK_E))
		{
			// [ 리트라이 도어에서 해야할 일 ] 
			// 1. 보스 전투바로 (컷씬 x)
			// 2. 플레이어 애니메이션 재생
			// 3. 디졸브 변수 올리기 (m_vDissolveGlowColor, m_fDissolve)
		}
	}

#ifdef _DEBUG
	if (KEY_DOWN(DIK_X))
	{
		m_pAnimator->Get_CurrentAnimController()->SetState("Idle");
		m_bFinish = false;
		m_bCanActive = false;
	}
#endif
}

void CBossRetryDoor::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	Move_Player(fTimeDelta);
}

void CBossRetryDoor::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBossRetryDoor::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pDissolveMap->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;

	_bool vDissolve = true;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_bDissolve", &vDissolve, sizeof(_bool))))
		return E_FAIL;

	if (m_vecDissolveMeshNum.empty())
	{
		if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveGlowColor", &m_vDissolveGlowColor, sizeof(_float3))))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &m_fDissolve, sizeof(_float))))
			return E_FAIL;
	}

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
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


		_float m_fEmissive = 0.f;
		m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float));
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
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

#ifdef _DEBUG

	if (m_pGameInstance->isIn_PhysXAABB(m_pPhysXTriggerCom))
	{
		if (m_pGameInstance->Get_RenderCollider())
		{
			if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom)))
				return E_FAIL;
		}
	}

#endif

	return S_OK;
}

void CBossRetryDoor::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBossRetryDoor::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBossRetryDoor::Play_Sound(_float fTimeDelta)
{
}

void CBossRetryDoor::OpenDoor()
{
	if (m_pAnimator)
	{
		m_pAnimator->SetTrigger("Open");
	}
}

void CBossRetryDoor::Move_Player(_float fTimeDelta)
{
}



HRESULT CBossRetryDoor::Ready_Components(void* pArg)
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxPBRMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;


	/*m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;*/

	return S_OK;
}


HRESULT CBossRetryDoor::LoadFromJson()
{
	string modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelCom)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName, m_pAnimator)))
		return E_FAIL;
	return S_OK;
}


CBossRetryDoor* CBossRetryDoor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBossRetryDoor* pGameInstance = new CBossRetryDoor(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CBossRetryDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CBossRetryDoor::Clone(void* pArg)
{
	CBossRetryDoor* pGameInstance = new CBossRetryDoor(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CBossRetryDoor");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CBossRetryDoor::Free()
{
	__super::Free();
}
