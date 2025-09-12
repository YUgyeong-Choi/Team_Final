#include "YGController.h"

#include "GameInstance.h"
#include "Camera_Manager.h"

#include "Level_Loading.h"
CYGController::CYGController(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CGameObject{ pDevice, pContext }
{
}

CYGController::CYGController(const CYGController& Prototype)
    : CGameObject(Prototype)
{
}

HRESULT CYGController::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CYGController::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC _desc{};
	lstrcpy(_desc.szName, TEXT("YGController"));
	_desc.fRotationPerSec = 8.f;
	_desc.fSpeedPerSec = 10.f;


	if (FAILED(__super::Initialize(&_desc))) {
		return E_FAIL;
	}

	if (FAILED(Ready_Components())) {
		return E_FAIL;
	}

	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}


	CCamera_Manager::Get_Instance()->SetPlayer(this);

	return S_OK;
}

void CYGController::Priority_Update(_float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_N))
	{
		CCamera_Manager::Get_Instance()->Play_CutScene(CUTSCENE_TYPE::WAKEUP);
	}

	_vector vMoveDir = XMVectorZero();

	if (m_pGameInstance->Key_Pressing(DIK_W))
		vMoveDir += m_pTransformCom->Get_State(STATE::LOOK); // 앞
	if (m_pGameInstance->Key_Pressing(DIK_S))
		vMoveDir -= m_pTransformCom->Get_State(STATE::LOOK); // 뒤
	if (m_pGameInstance->Key_Pressing(DIK_A))
		vMoveDir -= m_pTransformCom->Get_State(STATE::RIGHT); // 좌
	if (m_pGameInstance->Key_Pressing(DIK_D))
		vMoveDir += m_pTransformCom->Get_State(STATE::RIGHT); // 우

	if (XMVector3LengthSq(vMoveDir).m128_f32[0] > 0.0001f)
	{
		// 정규화해서 방향만 남김
		vMoveDir = XMVector3Normalize(vMoveDir);

		// 속도 * 시간 만큼 스케일
		_float fSpeed = m_pTransformCom->Get_SpeedPerSec();
		_float fDist = fSpeed * fTimeDelta;
		vMoveDir *= fDist;

		// XMVector -> PxVec3 변환
		XMFLOAT3 moveVec;
		XMStoreFloat3(&moveVec, vMoveDir);
		PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);

		// 컨트롤러 이동
		PxControllerFilters filters;
		m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);
	}
	SyncTransformWithController();
}

void CYGController::Update(_float fTimeDelta)
{
}

void CYGController::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
}

HRESULT CYGController::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			continue;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_SPECULAR, 0))) {}

		if (FAILED(m_pShaderCom->Begin(0)))
			continue;

		if (FAILED(m_pModelCom->Render(i)))
			continue;
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pControllerCom);
	}
#endif


	return S_OK;
}




HRESULT CYGController::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_pTransformCom->Get_World4x4())))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

void CYGController::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{

	printf("CYGController 충돌 시작!\n");
}

void CYGController::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CYGController::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	printf("CYGController 충돌 종료!\n");
}

void CYGController::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"CYGController Hit: %s\n", pOther->Get_Name().c_str());
}

void CYGController::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	wprintf(L"CYGController Trigger 시작: %ls\n", pOther->Get_Name().c_str());
}

void CYGController::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{

	wprintf(L"CYGController Trriger 종료: %ls\n", pOther->Get_Name().c_str());
}


HRESULT CYGController::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YG), TEXT("Prototype_Component_Model_Finoa"),TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Controller"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pControllerCom))))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CYGController::Ready_Collider()
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	PxExtendedVec3 pos(positionVec.x, positionVec.y, positionVec.z); 

	m_pControllerCom->Create_Controller(m_pGameInstance->Get_ControllerManager(),m_pGameInstance->GetMaterial(L"Default"),pos);
	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY;
	m_pControllerCom->Set_SimulationFilterData(filterData);
	m_pControllerCom->Set_QueryFilterData(filterData);
	m_pControllerCom->Set_Owner(this);
	m_pControllerCom->Set_ColliderType(COLLIDERTYPE::E);
	return S_OK;
}
void CYGController::SyncTransformWithController()
{
	if (!m_pControllerCom) return;

	PxExtendedVec3 pos = m_pControllerCom->Get_Controller()->getPosition();
	_vector vPos = XMVectorSet((float)pos.x, (float)pos.y, (float)pos.z, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
}

CYGController* CYGController::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CYGController* pInstance = new CYGController(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CYGController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CYGController::Clone(void* pArg)
{
	CYGController* pInstance = new CYGController(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CYGController");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CYGController::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pControllerCom);
}
