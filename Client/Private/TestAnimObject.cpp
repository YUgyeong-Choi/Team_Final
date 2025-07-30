#include "Animator.h"
#include "Animation.h"
#include "GameInstance.h"
#include "AnimController.h"
#include "TestAnimObject.h"
#include "Camera_Manager.h"
#include "PhysX_IgnoreSelfCallback.h"
#include "PhysXController.h"


CTestAnimObject::CTestAnimObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
	, m_pAnimator(nullptr)
	, m_pShaderCom(nullptr)
{
}
CTestAnimObject::CTestAnimObject(const CTestAnimObject& Prototype)
	: CGameObject(Prototype)
	, m_pAnimator(Prototype.m_pAnimator)
	, m_pShaderCom(Prototype.m_pShaderCom)
{
	Safe_AddRef(m_pAnimator);
	Safe_AddRef(m_pShaderCom);
}
HRESULT CTestAnimObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CTestAnimObject::Initialize(void* pArg)
{
	CGameObject::GAMEOBJECT_DESC GameObjectDesc = {};
	GameObjectDesc.fSpeedPerSec = 10.f;
	GameObjectDesc.fRotationPerSec = XMConvertToRadians(90.f);

	m_pCamera_Orbital = CCamera_Manager::Get_Instance()->GetOrbitalCam();

	if (FAILED(__super::Initialize(&GameObjectDesc)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

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

	path = "../Bin/Save/AnimationStates/" + m_pModelCom->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		m_pAnimator->Deserialize(rootStates);
	}

	m_pAnimator->Get_CurrentAnimController()->SetState("Idle");


	_fvector vPos{ 0.0f, 5.f, 0.0f, 1.0f };
	m_pTransformCom->Set_State(STATE::POSITION, vPos);

	if (FAILED(Ready_Collider())) {
		return E_FAIL;
	}

	CCamera_Manager::Get_Instance()->SetPlayer(this);

	return S_OK;
}

void CTestAnimObject::Priority_Update(_float fTimeDelta)
{
	/* [ 캐스케이드 전용 업데이트 함수 ] */
	UpdateShadowCamera();
	/* [ 움직임 전용 함수 ] */
	SetMoveState(fTimeDelta);
	/* [ 룩 벡터 레이케스트 ] */
	RayCast();
}
void CTestAnimObject::Update(_float fTimeDelta)
{
	if (m_pAnimator)
	{
		m_pAnimator->Update(fTimeDelta);
	}
	if (m_pModelCom)
	{
		m_pModelCom->Update_Bones();
	}
	//Input_Test(fTimeDelta);
}

void CTestAnimObject::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CTestAnimObject::Render()
{
	if (FAILED(Bind_Shader()))
		return E_FAIL;

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pControllerCom);
	}
#endif
	return S_OK;
}

HRESULT CTestAnimObject::Bind_Shader()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
		{
			// 플레이어 모델이 눈이 연결이 안되는 경우가 있어서 return 뺌
		}
		//	return E_FAIL;

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CTestAnimObject::Ready_Components()
{

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Prototype_Component_Model_TestAnimObject"), TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Controller"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pControllerCom))))
		return E_FAIL;

	return S_OK;
}

void CTestAnimObject::Input_Test(_float fTimeDelta)
{
	_bool bUp = GetAsyncKeyState(VK_UP) & 0x8000;
	_bool bDown = GetAsyncKeyState(VK_DOWN) & 0x8000;
	_bool bLeft = GetAsyncKeyState(VK_LEFT) & 0x8000;
	_bool bRight = GetAsyncKeyState(VK_RIGHT) & 0x8000;
	_bool bMove = (bUp || bDown || bLeft || bRight);

	static _bool bRunToggle = false;
	static _bool bSpaceHeld = false;      // 스페이스가 눌린 중인지
	static _bool bSprinting = false;      // 스프린트 모드로 전환했는지
	static _float fPressTime = 0.f;       // 누른 시간 누적
	const _float sprintTh = 0.8f;         // 이 시간 이상 누르면 스프린트

	_bool bSpaceDown = m_pGameInstance->Key_Down(DIK_SPACE);
	_bool bSpacePress = m_pGameInstance->Key_Pressing(DIK_SPACE);
	_bool bSpaceUp = m_pGameInstance->Key_Up(DIK_SPACE);

	if (m_pGameInstance->Key_Down(DIK_TAB))
	{
		// 무기 장착
		// 애니메이션 컨트롤러 변경
		m_pAnimator->SetTrigger("EquipWepaon");
		m_pAnimator->SetCurrentAnimController("Player_TwoHand","EquipWeapon");
	}

	// 스페이스 바를 눌렀을 때 (한 번만)
	if (bSpaceDown)
	{
		if (!bMove)
		{
			// 이동하지 않을 때는 즉시 트리거 발생
			m_pAnimator->SetTrigger("Dash");
		}
		else
		{
			// 이동 중일 때는 홀드 시작
			bSpaceHeld = true;
			bSprinting = false;
			fPressTime = 0.f;
		}
	}

	// 이동 중이면서 스페이스를 누르는 동안
	if (bMove && bSpaceHeld && bSpacePress)
	{
		fPressTime += fTimeDelta;

		// 일정 시간 넘어가면 스프린트 시작
		if (!bSprinting && fPressTime >= sprintTh)
		{
			m_pAnimator->SetBool("Sprint", true);
			bRunToggle = true;
			bSprinting = true;
		}
	}

	// 스페이스 바를 뗄 때 (한 번만)
	if (bSpaceHeld && bSpaceUp)
	{
		if (bMove)
		{
			if (!bSprinting)
			{
				// 짧게 눌렀으면 대시 트리거
				m_pAnimator->SetTrigger("Dash");
			}
			else
			{
				// 스프린트 종료
				m_pAnimator->SetBool("Sprint", false);
			}
		}

		// 상태 초기화
		bSpaceHeld = false;
		bSprinting = false;
		fPressTime = 0.f;
	}

	// 이동이 중단되면 스프린트도 중단
	if (!bMove && bSprinting)
	{
		m_pAnimator->SetBool("Sprint", false);
		bSprinting = false;
		bSpaceHeld = false;
		fPressTime = 0.f;
	}

	// 애니메이터 상태 설정
	if (m_pAnimator)
	{
		m_pAnimator->SetBool("Move", bMove);

		// Z키로 런 토글
		if (m_pGameInstance->Key_Down(DIK_Z))
		{
			bRunToggle = !bRunToggle;
		}
		m_pAnimator->SetBool("Run", bRunToggle);
	}
}

HRESULT CTestAnimObject::Ready_Collider()
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxExtendedVec3 pos(positionVec.x, positionVec.y, positionVec.z);
	m_pControllerCom->Create_Controller(m_pGameInstance->Get_ControllerManager(), m_pGameInstance->GetMaterial(L"Default"), pos, 0.4f, 1.0f);
	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	filterData.word1 = WORLDFILTER::FILTER_MONSTERBODY;
	m_pControllerCom->Set_SimulationFilterData(filterData);
	m_pControllerCom->Set_QueryFilterData(filterData);
	m_pControllerCom->Set_Owner(this);
	m_pControllerCom->Set_ColliderType(COLLIDERTYPE::E);
	return S_OK;
}

void CTestAnimObject::SyncTransformWithController()
{
	if (!m_pControllerCom) return;

	PxExtendedVec3 pos = m_pControllerCom->Get_Controller()->getPosition();
	_vector vPos = XMVectorSet((float)pos.x, (float)pos.y - 0.8f, (float)pos.z, 1.f);
	m_pTransformCom->Set_State(STATE::POSITION, vPos);
}


HRESULT CTestAnimObject::UpdateShadowCamera()
{
	CShadow::SHADOW_DESC Desc{};

	// 1. 카메라 고정 위치 (예: 공중에 떠있는 위치)
	_vector vFixedEye = XMVectorSet(76.f, 57.f, -21.f, 1.f);

	// 2. 플레이어 현재 위치를 타겟으로 설정
	_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetAt = vPlayerPos;

	// 3. 적용
	m_vShadowCam_Eye = vFixedEye;
	m_vShadowCam_At = vTargetAt;

	XMStoreFloat4(&Desc.vEye, m_vShadowCam_Eye);
	XMStoreFloat4(&Desc.vAt, m_vShadowCam_At);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;

	Desc.fFovy = XMConvertToRadians(40.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWA)))
		return E_FAIL;
	Desc.fFovy = XMConvertToRadians(80.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWB)))
		return E_FAIL;
	Desc.fFovy = XMConvertToRadians(120.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc, SHADOW::SHADOWC)))
		return E_FAIL;

	return S_OK;
}

void CTestAnimObject::SetMoveState(_float fTimeDelta)
{
	// 1. 카메라 기준 방향 추출
	_vector vCamLook = m_pCamera_Orbital->Get_TransfomCom()->Get_State(STATE::LOOK);
	_vector vCamRight = m_pCamera_Orbital->Get_TransfomCom()->Get_State(STATE::RIGHT);

	// 2. 수평 방향만 유지
	vCamLook = XMVectorSetY(vCamLook, 0.f);
	vCamRight = XMVectorSetY(vCamRight, 0.f);
	vCamLook = XMVector3Normalize(vCamLook);
	vCamRight = XMVector3Normalize(vCamRight);

	// 3. 입력 처리
	_vector vInputDir = XMVectorZero();
	if (m_pGameInstance->Key_Pressing(DIK_W)) vInputDir += vCamLook;
	if (m_pGameInstance->Key_Pressing(DIK_S)) vInputDir -= vCamLook;
	if (m_pGameInstance->Key_Pressing(DIK_D)) vInputDir += vCamRight;
	if (m_pGameInstance->Key_Pressing(DIK_A)) vInputDir -= vCamRight;

	// 4. 입력 없으면 리턴
	if (XMVector3Equal(vInputDir, XMVectorZero()))
	{
		_float3 moveVec = {};

		_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
		_float fDist = fSpeed * fTimeDelta;
		vInputDir *= fDist;
		XMStoreFloat3(&moveVec, vInputDir);

		// 중력 적용
		constexpr float fGravity = -9.81f;
		m_vGravityVelocity.y += fGravity * fTimeDelta;
		moveVec.y += m_vGravityVelocity.y * fTimeDelta;

		PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);
		PxControllerFilters filters;

		PxControllerCollisionFlags collisionFlags =
			m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

		// 4. 지면에 닿았으면 중력 속도 초기화
		if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
			m_vGravityVelocity.y = 0.f;

		SyncTransformWithController();
		return;
	}

	// 5. 방향 정규화
	vInputDir = XMVector3Normalize(vInputDir);

	// 6. 회전: 현재 방향과 입력 방향의 각도 계산
	_vector vPlayerLook = m_pTransformCom->Get_State(STATE::LOOK);
	vPlayerLook = XMVectorSetY(vPlayerLook, 0.f);
	vPlayerLook = XMVector3Normalize(vPlayerLook);

	_float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vInputDir));
	fDot = max(-1.f, min(1.f, fDot)); // Clamp
	_float fAngle = acosf(fDot);

	// 회전 방향 확인 (시계 or 반시계)
	_vector vCross = XMVector3Cross(vPlayerLook, vInputDir);
	if (XMVectorGetY(vCross) < 0.f)
		fAngle = -fAngle;

	// 부드러운 회전 적용 (고정 회전 속도)
	const _float fTurnSpeed = XMConvertToRadians(720.f); // 초당 720도 회전
	_float fClampedAngle = max(-fTurnSpeed * fTimeDelta, min(fTurnSpeed * fTimeDelta, fAngle));
	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);

	// 7. 이동
	_float3 moveVec = {};

	_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
	_float fDist = fSpeed * fTimeDelta;
	vInputDir *= fDist;
	//어느방향으로 몇만큼 이동한 벡터를 구함
	XMStoreFloat3(&moveVec, vInputDir);

	// 중력 적용
	constexpr float fGravity = -9.81f;
	m_vGravityVelocity.y += fGravity * fTimeDelta;
	moveVec.y += m_vGravityVelocity.y * fTimeDelta;

	PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);
	PxControllerFilters filters;

	PxControllerCollisionFlags collisionFlags =
		m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

	// 4. 지면에 닿았으면 중력 속도 초기화
	if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
		m_vGravityVelocity.y = 0.f;

	SyncTransformWithController();
}

void CTestAnimObject::RayCast()
{
	PxVec3 origin = m_pControllerCom->Get_Actor()->getGlobalPose().p;
	XMFLOAT3 fLook;
	XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
	PxVec3 direction = PxVec3(fLook.x, fLook.y, fLook.z);
	direction.normalize();
	_float fRayLength = 10.f;

	PxHitFlags hitFlags = PxHitFlag::eDEFAULT;
	PxRaycastBuffer hit;
	PxQueryFilterData filterData;
	filterData.flags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER;
	
	CIgnoreSelfCallback callback(m_pControllerCom->Get_Actor());

	if (m_pGameInstance->Get_Scene()->raycast(origin, direction, fRayLength, hit, hitFlags, filterData, &callback))
	{
		if (hit.hasBlock)
		{
			PxRigidActor* hitActor = hit.block.actor;

			//  자기 자신이면 무시
			if (hitActor == m_pControllerCom->Get_Actor())
			{
				printf(" Ray hit myself  skipping\n");
				return;
			}
			PxVec3 hitPos = hit.block.position;
			PxVec3 hitNormal = hit.block.normal;

			CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);
			pHitActor->Get_Owner()->On_Hit(this, m_pControllerCom->Get_ColliderType());

			printf("Ray충돌 했다!\n");
			printf("RayHitPos X: %f, Y: %f, Z: %f\n", hitPos.x, hitPos.y, hitPos.z);
			printf("RayHitNormal X: %f, Y: %f, Z: %f\n", hitNormal.x, hitNormal.y, hitNormal.z);
			m_bRayHit = true;
			m_vRayHitPos = hitPos;
			// 저기 hit.block.여기에 뭐 faceIndex, U, V 다양하게 있으니 궁금하면 보세여.. 
		}
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		DEBUGRAY_DATA _data{};
		_data.vStartPos = m_pControllerCom->Get_Actor()->getGlobalPose().p;
		XMFLOAT3 fLook;
		XMStoreFloat3(&fLook, m_pTransformCom->Get_State(STATE::LOOK));
		_data.vDirection = PxVec3(fLook.x, fLook.y, fLook.z);
		_data.fRayLength = 10.f;
		_data.bIsHit = m_bRayHit;
		_data.vHitPos = m_vRayHitPos;
		m_pControllerCom->Add_RenderRay(_data);

		m_bRayHit = false;
		m_vRayHitPos = {};
	}
#endif

}

CTestAnimObject* CTestAnimObject::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTestAnimObject* pInstance = new CTestAnimObject(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CTestAnimObject");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CTestAnimObject::Clone(void* pArg)
{
	CTestAnimObject* pInstance = new CTestAnimObject(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTestAnimObject");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CTestAnimObject::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pControllerCom);
}
