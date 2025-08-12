#include "Buttler_Train.h"
#include "GameInstance.h"
#include "Weapon_Monster.h"

CButtler_Train::CButtler_Train(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CMonster_Base{pDevice, pContext}
{
}

CButtler_Train::CButtler_Train(const CButtler_Train& Prototype)
	:CMonster_Base{Prototype}
{
}

HRESULT CButtler_Train::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CButtler_Train::Initialize(void* pArg)
{
	if(FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Weapon()))
		return E_FAIL;


	m_fDetectDist = 10.f;
	
	m_iHP = 300;

	return S_OK;
}

void CButtler_Train::Priority_Update(_float fTimeDelta)
{

	__super::Priority_Update(fTimeDelta);
}

void CButtler_Train::Update(_float fTimeDelta)
{
	Calc_Pos(fTimeDelta);

	__super::Update(fTimeDelta);
}

void CButtler_Train::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	Update_State();


	
}

HRESULT CButtler_Train::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	

	return S_OK;
}

void CButtler_Train::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	// 
	if (eColliderType == COLLIDERTYPE::PALYER)
	{
		m_pAnimator->SetTrigger("Hit");
		m_pAnimator->SetInt("Dir", ENUM_CLASS(Calc_HitDir(pOther->Get_TransfomCom()->Get_State(STATE::POSITION))));
	}

	m_pHPBar->Set_RenderTime(2.f);

}

void CButtler_Train::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CButtler_Train::Update_State()
{
	 Check_Detect();

	if (!m_isDetect)
		return;

	_vector vDist = {};
	vDist = m_pTransformCom->Get_State(STATE::POSITION) - m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);

	
	m_pAnimator->SetFloat("Distance", XMVectorGetX(XMVector3Length(vDist)));

    m_strStateName = m_pAnimator->Get_CurrentAnimController()->GetCurrentState()->stateName;

	

	if (m_strStateName.find("Run") != m_strStateName.npos || m_strStateName.find("Walk") != m_strStateName.npos)
		m_isLookAt = true;
	else
		m_isLookAt = false;

	if (m_strStateName.find("Attack") != m_strStateName.npos)
	{
		if (m_strStateName.find("Light") != m_strStateName.npos)
		{
			m_pAnimator->SetBool("UseLightAttack", false);
		}
		else
		{
			m_pAnimator->SetBool("UseLightAttack", true);
		}
	}

	
	if (m_iHP <= 0)
	{
		
	}

}

void CButtler_Train::Calc_Pos(_float fTimeDelta)
{
	if (m_strStateName.find("Run") != m_strStateName.npos || m_strStateName.find("Walk") != m_strStateName.npos)
	{
		m_pTransformCom->Go_Dir(m_pTransformCom->Get_State(STATE::LOOK), fTimeDelta, nullptr, m_pNaviCom);
	}

	else if (m_strStateName.find("Attack") != m_strStateName.npos )
	{
		RootMotionActive(fTimeDelta);
	}

}

HRESULT CButtler_Train::Ready_Weapon()
{
	return S_OK;
}

CButtler_Train* CButtler_Train::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CButtler_Train* pInstance = new CButtler_Train(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CButtler_Train");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CButtler_Train::Clone(void* pArg)
{
	CButtler_Train* pInstance = new CButtler_Train(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CButtler_Train");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CButtler_Train::Free()
{
	__super::Free();

	Safe_Release(m_pWeapon);

}