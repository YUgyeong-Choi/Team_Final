#include "FlameField.h"
#include "PhysXDynamicActor.h"

CFlameField::CFlameField(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{

}
CFlameField::CFlameField(const CFlameField& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CFlameField::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CFlameField::Initialize(void* pArg)
{
	GAMEOBJECT_DESC Desc{};
	Desc.fRotationPerSec = XMConvertToRadians(140.f);
	Desc.fSpeedPerSec = 0.f; // 불꽃 필드는 움직이지 않음
	lstrcpy(Desc.szName, TEXT("FlameField"));
	if (FAILED(__super::Initialize(&Desc)))
		return E_FAIL;
	if (FAILED(Ready_Components()))
		return E_FAIL;
	if (FAILED(Ready_Actor()))
		return E_FAIL;
    return S_OK;
}

void CFlameField::Priority_Update(_float fTimeDelta)
{
}

void CFlameField::Update(_float fTimeDelta)
{
}

void CFlameField::Late_Update(_float fTimeDelta)
{
}

HRESULT CFlameField::Render()
{
    return S_OK;
}

void CFlameField::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFlameField::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFlameField::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CFlameField::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFlameField::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CFlameField::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CFlameField::Ready_Components()
{
	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;
	return S_OK;
}

HRESULT CFlameField::Ready_Actor()
{
	return S_OK;
}

CFlameField* CFlameField::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CFlameField* pInstance = new CFlameField(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CFlameField");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CFlameField::Clone(void* pArg)
{
	CFlameField* pInstance = new CFlameField(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CFlameField");
		Safe_Release(pInstance);
	}
	return pInstance;
}
void CFlameField::Free()
{
	Safe_Release(m_pPhysXActorCom);
	__super::Free();
}
