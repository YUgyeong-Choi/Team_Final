#include "Bayonet.h"

#include "Unit.h"
#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "SwordTrailEffect.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "Player.h"
#include "Client_Calculation.h"


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
	/* [ 데미지 설정 ] */
	m_fDamage = 100.f;

	BAYONET_DESC* pDesc = static_cast<BAYONET_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	/* [ 바이오닛 위치 셋팅 ] */
	m_pTransformCom->Rotation(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.f));
	m_pTransformCom->Scaling(_float3{0.4f,0.4f,0.4f});

	// 스킬 정보 세팅

	m_eSkillDesc[0].fManaCost = 300.f;
	m_eSkillDesc[0].iSkillType = 0;
	m_eSkillDesc[0].iCountCombo = 3;
	m_eSkillDesc[0].isCombo = true;

	m_eSkillDesc[1].fManaCost = 100.f;
	m_eSkillDesc[1].iSkillType = 1;
	m_eSkillDesc[1].iCountCombo = 0;
	m_eSkillDesc[1].isCombo = false;

	m_fDurability = m_fMaxDurability;



	if (FAILED(Ready_Actor()))
		return E_FAIL;
	if (FAILED(Ready_Effect()))
		return E_FAIL;

	m_pGameInstance->Register_PullCallback(L"Weapon_Status", [this](const _wstring& eventName, void* data) {

		if (L"AddDurablity" == eventName)
		{
			m_fDurability += *static_cast<_float*>(data);

			if (m_fDurability >= m_fMaxDurability)
			{
				m_fDurability = m_fMaxDurability;
				// 빛나는 효과 잠깐 추가...
			}
			
			if (m_fDurability <= 0)
				m_fDurability = 0;
		}

		});

	m_pGameInstance->Notify(L"Weapon_Status", L"Durablity", &m_fDurability);
	m_pGameInstance->Notify(L"Weapon_Status", L"MaxDurablity", &m_fMaxDurability);


	m_iHandleIndex = m_pModelCom->Find_BoneIndex("BN_Handle");

	m_pWeaponEndMatrix = m_pModelCom->Get_CombinedTransformationMatrix(m_pModelCom->Find_BoneIndex("BN_Blade_End"));

	return S_OK;
}

void CBayonet::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);

	if (m_bHitRegActive)
		Update_HitReg(fTimeDelta);
}
void CBayonet::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (KEY_PRESSING(DIK_G))
	{
		CEffectContainer::DESC desc = {};

		auto worldmat = XMLoadFloat4x4(m_pWeaponEndMatrix) * m_pTransformCom->Get_WorldMatrix() * m_pOwner->Get_TransfomCom()->Get_WorldMatrix();
		_vector rot, trans, scale;
		XMMatrixDecompose(&scale, &rot, &trans, worldmat);

		XMStoreFloat4x4(&desc.PresetMatrix, XMMatrixRotationQuaternion(rot) *
			XMMatrixTranslation(trans.m128_f32[0],
				trans.m128_f32[1],
				trans.m128_f32[2]));

		if (nullptr == CEffect_Manager::Get_Instance()->Make_EffectContainer(static_cast<_uint>(m_iLevelID), L"EC_Player_Skill_WeaponParticle_P1", &desc))
			MSG_BOX("이펙트 생성 실패함");
	}
	if (KEY_DOWN(DIK_G))
	{
		CEffectContainer::DESC desc = {};
		
		auto worldmat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iHandleIndex)) * m_pTransformCom->Get_WorldMatrix() * m_pOwner->Get_TransfomCom()->Get_WorldMatrix();

		XMStoreFloat4x4(&desc.PresetMatrix,
			XMMatrixTranslation(worldmat.r[3].m128_f32[0],
				worldmat.r[3].m128_f32[1],
				worldmat.r[3].m128_f32[2]));

		if (nullptr == CEffect_Manager::Get_Instance()->Make_EffectContainer(static_cast<_uint>(m_iLevelID), L"EC_Player_Skill_Blink_P1S2", &desc))
			MSG_BOX("이펙트 생성 실패함");
		static _bool bTEactive = true;
		bTEactive = !bTEactive;
		Set_WeaponTrail_Active(bTEactive, TRAIL_SKILL_BLUE);
	}
}

void CBayonet::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	// 매 프레임 칼 끝 위치 저장
	//m_vEndSocketPrevPos = m_vEndSocketCurPos;
	//auto CurEndWorldMat = XMLoadFloat4x4(m_pWeaponEndMatrix) * m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentWorldMatrix);
	//XMStoreFloat3(&m_vEndSocketCurPos, CurEndWorldMat.r[3]);
	if (m_bHitEffect == true)
	{
		auto CurEndWorldMat = XMLoadFloat4x4(m_pWeaponEndMatrix) * m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentWorldMatrix);
		XMStoreFloat3(&m_vEndSocketCurPos, CurEndWorldMat.r[3]);
		_float fLength = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_vEndSocketCurPos) - XMLoadFloat3(&m_vEndSocketPrevPos)));
		if (fLength < 1e-6f) 
			m_bHitEffect = true;
		else
		{
			m_bHitEffect = false;
			Create_AttackEffect(m_pLastHitObject, m_eLastHitColType);
		}
	}
	Update_Collider();
}

HRESULT CBayonet::Render()
{
	__super::Render();

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider() && m_pPhysXActorCom->Get_ReadyForDebugDraw()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
	}
#endif

	return S_OK;
}

HRESULT CBayonet::Render_Shadow()
{
	if (!m_bIsActive)
		return S_OK;

	/* [ 월드 스페이스 넘기기 ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Light_ViewMatrix(SHADOW::SHADOWA))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Light_ProjMatrix(SHADOW::SHADOWA))))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);
		m_pShaderCom->Begin(3);

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

void CBayonet::Update_Collider()
{
	if (!m_isActive)
		return;

	// 1. 부모 행렬
	_matrix ParentWorld = XMLoadFloat4x4(m_pParentWorldMatrix);

	// 2. Socket 월드 행렬 
	_matrix SocketMat = XMLoadFloat4x4(m_pSocketMatrix);

	for (size_t i = 0; i < 3; i++)
		SocketMat.r[i] = XMVector3Normalize(SocketMat.r[i]);

	// 3. Blade 본 월드 행렬
	_matrix HandleMat = XMLoadFloat4x4(m_pModelCom->Get_CombinedTransformationMatrix(m_iHandleIndex));

	for (size_t i = 0; i < 3; i++)
		HandleMat.r[i] = XMVector3Normalize(HandleMat.r[i]);

	_matrix WeaponWorld = HandleMat * SocketMat * ParentWorld;

	_vector localOffset = XMVectorSet(0.f, -0.5f, 0.f, 1.f);
	_vector worldPos = XMVector4Transform(localOffset, WeaponWorld);
	
	// 6. PhysX 적용
	_vector finalPos = WeaponWorld.r[3];

	_vector finalRot = XMQuaternionRotationMatrix(WeaponWorld);

	PxVec3 physxPos(XMVectorGetX(worldPos), XMVectorGetY(worldPos), XMVectorGetZ(worldPos));
	PxQuat physxRot(XMVectorGetX(finalRot), XMVectorGetY(finalRot), XMVectorGetZ(finalRot), XMVectorGetW(finalRot));

	m_pPhysXActorCom->Set_Transform(PxTransform(physxPos, physxRot));

}

void CBayonet::SetisAttack(_bool isAttack)
{
	if (isAttack)
	{
		m_pPhysXActorCom->Set_SimulationFilterData(m_pPhysXActorCom->Get_FilterData());
		m_pGameInstance->Get_Scene()->resetFiltering(*m_pPhysXActorCom->Get_Actor());
	}
	else
	{
		m_pPhysXActorCom->Init_SimulationFilterData();
	}
}

void CBayonet::Calc_Durability(_float fDelta)
{
	_float fTemp = -1.f * fDelta;

	m_pGameInstance->Notify(L"Weapon_Status", L"AddDurablity", &fTemp);
}

void CBayonet::Reset()
{
	m_fDurability = m_fMaxDurability;
	m_pGameInstance->Notify(L"Weapon_Status", L"Durablity", &m_fDurability);
}

void CBayonet::Set_WeaponTrail_Active(_bool bActive, TRAILTYPE eType)
{
	switch (eType)
	{
	case Client::TRAIL_DEFAULT:
		if (m_pWeaponTrailEffect)
			m_pWeaponTrailEffect->Set_TrailActive(bActive);
		break;
	case Client::TRAIL_SKILL_BLUE:
		if (m_pSkillTrailEffect)
			m_pSkillTrailEffect->Set_TrailActive(bActive);
		break;
	case Client::TRAIL_SKILL_RED:
		break;
	case Client::TRAIL_BLOOD:
		break;
	case Client::TRAIL_END:
		break;
	default:
		break;
	}
}



HRESULT CBayonet::Ready_Components()
{
	/* [ 따로 추가할 컴포넌트가 있습니까? ] */

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CBayonet::Ready_Actor()
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
	
	PxVec3 halfExtents = PxVec3(0.2f, 1.f, 0.2f);
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(false, true, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERWEAPON; 
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY; 
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::PLAYER_WEAPON);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	
	return S_OK;
}

HRESULT CBayonet::Ready_Effect()
{
	_uint iInnerBoneIdx = m_pModelCom->Find_BoneIndex("BN_Blade_B");
	_uint iOuterBoneIdx = m_pModelCom->Find_BoneIndex("BN_Blade_End");

	CSwordTrailEffect::DESC desc = {};
	desc.pInnerSocketMatrix = const_cast<_float4x4*>(m_pModelCom->Get_CombinedTransformationMatrix(iInnerBoneIdx));
	desc.pOuterSocketMatrix = const_cast<_float4x4*>(m_pModelCom->Get_CombinedTransformationMatrix(iOuterBoneIdx));
	desc.pParentCombinedMatrix = &m_CombinedWorldMatrix;
	desc.iLevelID = m_iLevelID;
	m_pWeaponTrailEffect = dynamic_cast<CSwordTrailEffect*>(MAKE_SINGLEEFFECT(ENUM_CLASS(m_iLevelID), TEXT("TE_Test_20_30_3"), TEXT("Layer_Effect"), 0.f, 0.f, 0.f, &desc));
	if (m_pWeaponTrailEffect)
		m_pWeaponTrailEffect->Set_TrailActive(false);
	else
		MSG_BOX("무기 트레일 사망");

	m_pSkillTrailEffect = dynamic_cast<CSwordTrailEffect*>(MAKE_SINGLEEFFECT(ENUM_CLASS(m_iLevelID), TEXT("TE_Skill"), TEXT("Layer_Effect"), 0.f, 0.f, 0.f, &desc));
	if (m_pSkillTrailEffect)
		m_pSkillTrailEffect->Set_TrailActive(false);
	else
		MSG_BOX("무기 스킬 트레일 사망");

	return S_OK;	
}

void CBayonet::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	// 내구도나 이런거 하면 될듯?
	// 가드 때 충돌하고, 퍼펙트 가드가 아니면 감소하도록
	// 이제 트리거로 이사감
}

void CBayonet::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
	//m_isAttack =false;
}

void CBayonet::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CBayonet::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CBayonet::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	//_vector vPlayerPos = XMVectorSetY(m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION), 0.f);
	//_vector vOtherPos = XMVectorSetY(pOther->Get_TransfomCom()->Get_State(STATE::POSITION),0.f);
	//_vector vDir = XMVector3Normalize(vPlayerPos - vOtherPos);

	if (eColliderType == COLLIDERTYPE::MONSTER)
	{
		m_bHitEffect = true;
		m_pLastHitObject = pOther;
		m_eLastHitColType = eColliderType;
		auto CurEndWorldMat = XMLoadFloat4x4(m_pWeaponEndMatrix) * m_pTransformCom->Get_WorldMatrix() * XMLoadFloat4x4(m_pParentWorldMatrix);
		XMStoreFloat3(&m_vEndSocketPrevPos, CurEndWorldMat.r[3]);
		//Create_AttackEffect(pOther, eColliderType);

		CUnit* pUnit = static_cast<CUnit*>(pOther);
		

		
		StartHitReg(0.1f, 0.015f, 0.025f);

		m_pSoundCom->Play_Random("SE_PC_SK_Hit_Skin_Slice_S_", 3);

		static_cast<CPlayer*>(m_pOwner)->Set_HitTarger(pUnit, false);
	}

	// 플레이어의 공격 state에 따라 변경 -> slash / thrust
	// 막타인지도 알 수 있을까? 
}

void CBayonet::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

HRESULT CBayonet::Create_AttackEffect(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	_vector vPlayerPos = XMVectorSetY(m_pOwner->Get_TransfomCom()->Get_State(STATE::POSITION), 0.f);
	_vector vOtherPos = XMVectorSetY(pOther->Get_TransfomCom()->Get_State(STATE::POSITION), 0.f);
	_vector vDir = XMVector3Normalize(vPlayerPos - vOtherPos);

	CUnit* pUnit = static_cast<CUnit*>(pOther);

	auto& vLockonPos = pUnit->Get_LockonPos();
	_float3 vModifiedPos = _float3(vLockonPos.x + vDir.m128_f32[0], vLockonPos.y + vDir.m128_f32[1], vLockonPos.z + vDir.m128_f32[2]);

	_vector vLook = XMVector3Normalize(vDir);           // Look
	_vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f); // 고정 Up
	_vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vLook));
	_vector vUp = XMVector3Cross(vLook, vRight);

	_matrix mAlign = XMMATRIX(vRight, vUp, vLook, XMVectorSet(vModifiedPos.x, vModifiedPos.y, vModifiedPos.z, 1.f));

	_vector vSlashDir = XMVector3Normalize(XMLoadFloat3(&m_vEndSocketCurPos) - XMLoadFloat3(&m_vEndSocketPrevPos));
	float lenSq = XMVectorGetX(XMVector3LengthSq(vSlashDir));

	if (lenSq < 1e-6f) {
		// 너무 짧으면 fallback 방향 (예: +X)
		vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f);
	}
	else {
		vRight = XMVector3Normalize(vSlashDir);
	}

	_float dot = XMVectorGetX(XMVector3Dot(vLook, vSlashDir));
	dot = max(-1.f, min(1.f, dot));
	_float angle = acosf(dot);

	_vector axis = XMVector3Normalize(XMVector3Cross(vLook, vSlashDir));
	_matrix mRot = XMMatrixRotationAxis(axis, angle);

	_matrix mRoll = XMMatrixRotationAxis(vLook, angle);

	CEffectContainer::DESC desc = {};

	XMStoreFloat4x4(&desc.PresetMatrix,
		XMMatrixScaling(2.f, 2.f, 2.f) * mRoll * mAlign);

	CGameObject* pEffect = { nullptr };
	/*rand() % 3 == 1 ? pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_AttackHit_Basic_Spark_1_P2S4"), &desc)
		: rand() % 2 == 1 ? pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_AttackHit_Slash_x-1_P1S2"), &desc)
		:*/ pEffect = MAKE_EFFECT(ENUM_CLASS(m_iLevelID), TEXT("EC_AttackHit_Slash_x-1_P1S2"), &desc);

		if (pEffect == nullptr)
			return E_FAIL;

	return S_OK;
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

	Safe_Release(m_pPhysXActorCom);
}
