#include "Bayonet.h"

#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "PhysX_IgnoreSelfCallback.h"


CBayonet::CBayonet(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CWeapon(pDevice, pContext)
{
}
CBayonet::CBayonet(const CBayonet& Prototype)
	: CWeapon(Prototype)
{
}
HRESULT CBayonet::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CBayonet::Initialize(void* pArg)
{
	BAYONET_DESC* pDesc = static_cast<BAYONET_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/* [ 바이오닛 위치 셋팅 ] */
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.f));
	m_pTransformCom->Scaling(_float3{0.4f,0.4f,0.4f});

	return S_OK;
}

void CBayonet::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
}
void CBayonet::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);
}

void CBayonet::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CBayonet::Render()
{
	__super::Render();

	return S_OK;
}

HRESULT CBayonet::Ready_Components()
{
	/* [ 따로 추가할 컴포넌트가 있습니까? ] */

	return S_OK;
}

void CBayonet::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

CBayonet* CBayonet::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CBayonet* pInstance = new CBayonet(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CBayonet");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CBayonet::Clone(void* pArg)
{
	CBayonet* pInstance = new CBayonet(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CBayonet");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CBayonet::Free()
{
	__super::Free();
}
