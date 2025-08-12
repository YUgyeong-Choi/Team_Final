#include "Fuoco.h"
#include "GameInstance.h"

CFuoco::CFuoco(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUnit(pDevice, pContext)
{
}

CFuoco::CFuoco(const CFuoco& Prototype)
	: CUnit(Prototype)
{
}
HRESULT CFuoco::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFuoco::Initialize(void* pArg)
{

	UNIT_DESC UnitDesc{};
	UnitDesc.eLevelID = LEVEL::KRAT_CENTERAL_STATION;
	UnitDesc.fRotationPerSec = XMConvertToRadians(90.f);
	UnitDesc.fSpeedPerSec = 20.f;
	lstrcpy(UnitDesc.szName, TEXT("FireEater"));
	UnitDesc.szMeshID = TEXT("FireEater");
	UnitDesc.InitPos = _float3(87.5f, 0.f, -7.5f);
	UnitDesc.InitScale = _float3(0.9f, 0.9f, 0.9f);

	if (FAILED(__super::Initialize(&UnitDesc)))
		return E_FAIL;
	_vector vInitPos = XMVectorSetW(XMLoadFloat3(&UnitDesc.InitPos),1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vInitPos);
	m_pTransformCom->Scaling(UnitDesc.InitScale);

	if (FAILED(LoadFromJson()))
		return E_FAIL;

	return S_OK;
}

void CFuoco::Priority_Update(_float fTimeDelta)
{
}

void CFuoco::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CFuoco::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_SHADOW, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

void CFuoco::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFuoco::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CFuoco::LoadFromJson()
{
	string path = "../Bin/Save/AnimationEvents/" + m_pModelCom->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = m_pModelCom->GetAnimations();

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}
	else
	{
		MSG_BOX("Failed to open animation events file.");
		return E_FAIL;
	}

	path = "../Bin/Save/AnimationStates/" + m_pModelCom->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		m_pAnimator->Deserialize(rootStates);
	}
	else
	{
		MSG_BOX("Failed to open animation states file.");
		return E_FAIL;
	}
	return S_OK;
}

CFuoco* CFuoco::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFuoco* pInstance = new CFuoco(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFuoco");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFuoco::Clone(void* pArg)
{
	CFuoco* pInstance = new CFuoco(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CFuoco");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CFuoco::Free()
{
	__super::Free();
}
