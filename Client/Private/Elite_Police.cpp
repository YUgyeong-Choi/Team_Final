#include "Elite_Police.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"
#include "LockOn_Manager.h"
#include "Client_Calculation.h"

CElite_Police::CElite_Police(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonster_Base{pDevice, pContext}
{
}

CElite_Police::CElite_Police(const CElite_Police& Prototype)
	:CMonster_Base{Prototype}
{
}

HRESULT CElite_Police::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CElite_Police::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Weapon()))
		return E_FAIL;

	m_fDetectDist = 10.f;

	m_iHP = 300;

	m_pHPBar->Set_MaxHp(m_iHP);

	m_iLockonBoneIndex = m_pModelCom->Find_BoneIndex("Bip001-Spine2");

	return S_OK;
}

void CElite_Police::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_strStateName.find("Dead") != m_strStateName.npos)
	{
		if (m_pAnimator->IsFinished())
		{
			m_pWeapon->Set_bDead();
		}
	}

}

void CElite_Police::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CElite_Police::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_State();
}

HRESULT CElite_Police::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;


	return S_OK;
}

void CElite_Police::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CElite_Police::Update_State()
{
	Check_Detect();

	m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;

	if (!m_isDetect || m_iHP <= 0)
	{
		m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;
		return;
	}
}

_int CElite_Police::Update_AttackType()
{

	// °Å¸®·Î?
	_vector vDist = {};
	vDist = m_pTransformCom->Get_State(STATE::POSITION) - m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

	if (XMVectorGetX(vDist) > 8.f)
		return ATTACK_END;

	else
		return GetRandomInt(ATTACK_A, ATTACK_D);
}

HRESULT CElite_Police::Ready_Weapon()
{
	CWeapon_Monster::MONSTER_WEAPON_DESC Desc{};
	Desc.eLevelID = LEVEL::STATIC;
	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.InitPos = { 0.125f, 0.f, 0.f };
	Desc.InitScale = { 1.f, 0.6f, 1.f };
	Desc.iRender = 0;

	Desc.szMeshID = TEXT("Elite_Police_Weapon");
	lstrcpy(Desc.szName, TEXT("Elite_Police_Weapon"));
	Desc.vAxis = { 0.f,1.f,0.f,0.f };
	Desc.fRotationDegree = { 90.f };

	Desc.pSocketMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("Bip001-R-Hand"));
	Desc.pParentWorldMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Monster_Weapon"),
		ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Monster_Weapon"), &pGameObject, &Desc)))
		return E_FAIL;

	m_pWeapon = dynamic_cast<CWeapon_Monster*>(pGameObject);

	Safe_AddRef(m_pWeapon);


	return S_OK;
}

CElite_Police* CElite_Police::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CElite_Police* pInstance = new CElite_Police(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CElite_Police");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CElite_Police::Clone(void* pArg)
{
	CElite_Police* pInstance = new CElite_Police(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CElite_Police");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CElite_Police::Free()
{
	__super::Free();

	Safe_Release(m_pWeapon);

}
