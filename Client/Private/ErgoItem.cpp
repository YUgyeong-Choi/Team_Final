#include "ErgoItem.h"
#include "GameInstance.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "Player.h"
#include "UI_Manager.h"
#include "UI_Guide.h"
#include "UI_Letter.h"

CErgoItem::CErgoItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CItem{ pDevice, pContext }
{
}

CErgoItem::CErgoItem(const CErgoItem& Prototype)
	:CItem(Prototype)
{
}

HRESULT CErgoItem::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CErgoItem::Initialize(void* pArg)
{
	ERGOITEM_DESC* pDesc = static_cast<ERGOITEM_DESC*>(pArg);

	if (pDesc->eItemTag == ITEM_TAG::PULSE_CELL)
	{
		m_bPulseCell = true;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Effect(pArg)))
		return E_FAIL;



	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	m_eItemTag = pDesc->eItemTag;

	if (FAILED(Ready_Collider()))
		return E_FAIL;

	//XMStoreFloat3(&m_vOriginPos, m_pTransformCom->Get_State(STATE::POSITION));

	return S_OK;
}

void CErgoItem::Priority_Update(_float fTimeDelta)
{
	if (nullptr == m_pTarget)
	{
		m_pTarget = GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex());
	}
	
	if (m_bDoOnce)
	{
		// 키 입력 받아서 ...

		if (m_pGameInstance->Key_Down(DIK_E))
		{
			//if (m_bDead != true)
				static_cast<CPlayer*>(m_pTarget)->NotifyCanGetItem(true);
			Set_bDead();

			m_pPhysXActorCom->RemovePhysX();
			
			m_bDoOnce = false;
			CUI_Manager::Get_Instance()->Activate_Popup(false);
				
			if (m_eItemTag == ITEM_TAG::END)
			{

			}
			else if (m_eItemTag == ITEM_TAG::PULSE_CELL)
			{
				// 펄스는 관련 guide ui 띄우기
				CUI_Guide::UI_GUIDE_DESC eDesc{};

				eDesc.partPaths = { TEXT("../Bin/Save/UI/Guide/Guide_Pulse.json")};

				m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"),
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Guide"), &eDesc);

				_int iCount = 3;
				m_pGameInstance->Notify(TEXT("Slot_Belts"), TEXT("MaxCount"), &iCount);

				static_cast<CPlayer*>(m_pTarget)->Callback_UpBelt();
				static_cast<CPlayer*>(m_pTarget)->Callback_DownBelt();

			}
			else if (m_eItemTag == ITEM_TAG::PASSENGER_NOTE)
			{
				CUI_Letter::UI_LETTER_DESC eDesc{};

				eDesc.partPaths = { TEXT("../Bin/Save/UI/Letter/Letter_Page_0.json"), TEXT("../Bin/Save/UI/Letter/Letter_Page_1.json") };

				m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Letter"),
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Guide"), &eDesc);
			}
			else if (m_eItemTag == ITEM_TAG::KRAT_TIMES)
			{
				CUI_Letter::UI_LETTER_DESC eDesc{};

				eDesc.partPaths = { TEXT("../Bin/Save/UI/Letter/Letter_NewsPaper_0.json"), TEXT("../Bin/Save/UI/Letter/Letter_NewsPaper_1.json") };

				m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Letter"),
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Guide"), &eDesc);
			}
			else if (m_eItemTag == ITEM_TAG::LOST_ERGO)
			{
				CUI_Container::UI_CONTAINER_DESC eDesc{};
				eDesc.fLifeTime = 6.f;
				eDesc.useLifeTime = true;
				eDesc.strFilePath = TEXT("../Bin/Save/UI/ErgoRecovery.json");

				m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Container"), &eDesc);

				CUI_Container* pContainer = static_cast<CUI_Container*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player_UI_Container")));

				static_cast<CDynamic_UI*>(pContainer->Get_PartUI().back())->Set_isUVmove(true);

				static_cast<CPlayer*>(m_pTarget)->Recovery_Ergo();
				
				CUI_Manager::Get_Instance()->Sound_Play("SE_UI_AlertRecovery");
			}
			else
			{
				CUI_Manager::Get_Instance()->Activate_UI(TEXT("Pickup_Item"), false);
				CUI_Manager::Get_Instance()->Update_PickUpItem(ENUM_CLASS(m_eItemTag));
				CUI_Manager::Get_Instance()->Activate_UI(TEXT("Pickup_Item"), true);
			}



			// 이펙트 삭제 로직 필요
			if (m_pEffect)
				m_pEffect->End_Effect();
			// 없어지는 이펙트 추가할 것 - 채영

			
		}
	}
	


}

void CErgoItem::Update(_float fTimeDelta)
{
	//if (m_eItemTag == ITEM_TAG::PULSE_CELL)
	//{
	//	// 시간 누적
	//	m_fAccTime += fTimeDelta;

	//	_float fAmplitude = 0.1f;   // 위아래 진폭
	//	_float fSpeed = 1.f;    // 움직이는 속도

	//	// 기준 위치를 중심으로 sin 파 적용
	//	_float3 vPos = m_vOriginPos;
	//	vPos.y += sinf(m_fAccTime * fSpeed) * fAmplitude;

	//	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&vPos), 1.f));
	//}
}


void CErgoItem::Late_Update(_float fTimeDelta)
{
	if (m_bPulseCell)
	{
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	}
}

HRESULT CErgoItem::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0)))
			return E_FAIL;
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{

		}

		_bool bIsEmissive = true;

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
			bIsEmissive = false;

		/* [ 이미시브 맵이 있다면 사용하라 ] */
		if (bIsEmissive)
		{
			_float fEmissive = 100.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pShaderCom->Begin(0);

		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderMapCollider())
	{
		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;
	}
#endif

	return S_OK;
}

_bool CErgoItem::Check_Player_Close()
{
	if (nullptr == m_pTarget)
		return false;

	//
	//플레이어가 가까운지 체크
	_vector vPlayerPos = m_pTarget->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDiff = vPos - vPlayerPos;
	_float fDist = XMVectorGetX(XMVector3Length(vDiff));

	if (fDist < 1.5f)
		return true;
	else
		return false;
	
}

void CErgoItem::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	// 팝업 키기
	if (!m_bDoOnce)
	{
		CUI_Manager::Get_Instance()->Activate_Popup(true);

		if (m_eItemTag != ITEM_TAG::LOST_ERGO)
			CUI_Manager::Get_Instance()->Set_Popup_Caption(5);
		else
			CUI_Manager::Get_Instance()->Set_Popup_Caption(6);

		m_bDoOnce = true;
	}
}

void CErgoItem::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CErgoItem::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (m_bDoOnce)
	{
		// 팝업 끄기
		CUI_Manager::Get_Instance()->Activate_Popup(false);
		m_bDoOnce = false;
	}
}

HRESULT CErgoItem::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

HRESULT CErgoItem::Ready_Components(void* pArg)
{
	if (m_bPulseCell)
	{
		/* Com_Shader */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
			TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
			return E_FAIL;

		//ERGOITEM_DESC* pDesc = static_cast<ERGOITEM_DESC*>(pArg);
		//if (pDesc->eItemTag == ITEM_TAG::PULSE_CELL)

		/* For.Com_Model */ 	//아이템 테스트용 모델
		if (FAILED(__super::Add_Component(m_iLevelID, TEXT("Prototype_Component_Model_PulseCell"),
			TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
			return E_FAIL;
	}

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CErgoItem::Ready_Effect(void* pArg)
{
	CEffectContainer::DESC desc = {};
	desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
	XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixIdentity());


	//ERGOITEM_DESC* pDesc = static_cast<ERGOITEM_DESC*>(pArg);

	wstring wsEffectName = {};

	if (m_bPulseCell)
		wsEffectName = TEXT("EC_YW_PulseCell_Effect_Edit");
	else
		wsEffectName = TEXT("EC_ErgoItem_M3P1_WB");

	m_pEffect = static_cast<CEffectContainer*>(MAKE_EFFECT(ENUM_CLASS(m_iLevelID), wsEffectName, &desc));
	if (m_pEffect == nullptr)
		return E_FAIL;

	return S_OK;
}

HRESULT CErgoItem::Ready_Collider()
{
	// 3. Transform에서 S, R, T 분리
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. 스케일, 회전, 위치 변환
	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = {};

	/*if (pArg != nullptr)
	{
		halfExtents = VectorToPxVec3(XMLoadFloat3(&pDesc->vExtent));
	}
	else
	{
		halfExtents = VectorToPxVec3(XMLoadFloat3(&m_vHalfExtents));
	}*/

	_float3 vHalf = _float3(0.5f, 0.75f, 0.5f);

	halfExtents = VectorToPxVec3(XMLoadFloat3(&vHalf));
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(false, true, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_INTERACT;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);

	PxActor* pActor = m_pPhysXActorCom->Get_Actor();
	if (!pActor->getScene()) // nullptr이면 씬에 없음
	{
		m_pGameInstance->Get_Scene()->addActor(*pActor);
	}


	return S_OK;
}

CErgoItem* CErgoItem::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CErgoItem* pInstance = new CErgoItem(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CErgoItem");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CErgoItem::Clone(void* pArg)
{
	CErgoItem* pInstance = new CErgoItem(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CErgoItem");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CErgoItem::Free()
{
	__super::Free();
	
	if (m_pEffect)
		m_pEffect->Set_bDead();

	if (m_bPulseCell)
	{
		Safe_Release(m_pModelCom);
		Safe_Release(m_pShaderCom);
	}

	Safe_Release(m_pPhysXActorCom);
}
