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
	if (m_pGameInstance->Key_Pressing(DIK_E))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * 0.1f);
	}

	if (m_pGameInstance->Key_Pressing(DIK_Q))
	{
		m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), -fTimeDelta * 0.1f);
	}

	_vector vMoveDir = XMVectorZero();

	if (m_pGameInstance->Key_Pressing(DIK_W))
		vMoveDir += m_pTransformCom->Get_State(STATE::LOOK);
	if (m_pGameInstance->Key_Pressing(DIK_S))
		vMoveDir -= m_pTransformCom->Get_State(STATE::LOOK);
	if (m_pGameInstance->Key_Pressing(DIK_A))
		vMoveDir -= m_pTransformCom->Get_State(STATE::RIGHT);
	if (m_pGameInstance->Key_Pressing(DIK_D))
		vMoveDir += m_pTransformCom->Get_State(STATE::RIGHT);


	XMFLOAT3 moveVec = {};
	if (XMVector3LengthSq(vMoveDir).m128_f32[0] > 0.0001f)
	{
		vMoveDir = XMVector3Normalize(vMoveDir);
		_float fSpeed = m_pTransformCom->Get_SpeedPreSec();
		_float fDist = fSpeed * fTimeDelta;
		vMoveDir *= fDist;
		XMStoreFloat3(&moveVec, vMoveDir);
	}

	
	constexpr float fGravity = -9.81f;
	m_vGravityVelocity.y += fGravity * fTimeDelta;
	moveVec.y += m_vGravityVelocity.y * fTimeDelta;


	PxVec3 pxMove(moveVec.x, moveVec.y, moveVec.z);
	PxControllerFilters filters;

	PxControllerCollisionFlags collisionFlags =
		m_pControllerCom->Get_Controller()->move(pxMove, 0.001f, fTimeDelta, filters);

	if (collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN)
		m_vGravityVelocity.y = 0.f;

	SyncTransformWithController();
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


	UpdateShadowCamera();

	SetMoveState(fTimeDelta);

	Ray();
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
			// �÷��̾� ���� ���� ������ �ȵǴ� ��찡 �־ return ��
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
	static _bool bSpaceHeld = false;      // �����̽��� ���� ������
	static _bool bSprinting = false;      // ������Ʈ ���� ��ȯ�ߴ���
	static _float fPressTime = 0.f;       // ���� �ð� ����
	const _float sprintTh = 0.8f;         // �� �ð� �̻� ������ ������Ʈ

	_bool bSpaceDown = m_pGameInstance->Key_Down(DIK_SPACE);
	_bool bSpacePress = m_pGameInstance->Key_Pressing(DIK_SPACE);
	_bool bSpaceUp = m_pGameInstance->Key_Up(DIK_SPACE);

	if (m_pGameInstance->Key_Down(DIK_TAB))
	{
		// ���� ����
		// �ִϸ��̼� ��Ʈ�ѷ� ����
		m_pAnimator->SetTrigger("EquipWepaon");
		if (m_pAnimator->CheckBool("Move"))
		{
		m_pAnimator->SetCurrentAnimController("Player_TwoHand","EquipWeapon_Walk_F");

		}
		else
		{
		m_pAnimator->SetCurrentAnimController("Player_TwoHand","EquipWeapon");
		}
	}

	// �����̽� �ٸ� ������ �� (�� ����)
	if (bSpaceDown)
	{
		if (!bMove)
		{
			// �̵����� ���� ���� ��� Ʈ���� �߻�
			m_pAnimator->SetTrigger("Dash");
		}
		else
		{
			// �̵� ���� ���� Ȧ�� ����
			bSpaceHeld = true;
			bSprinting = false;
			fPressTime = 0.f;
		}
	}

	// �̵� ���̸鼭 �����̽��� ������ ����
	if (bMove && bSpaceHeld && bSpacePress)
	{
		fPressTime += fTimeDelta;

		// ���� �ð� �Ѿ�� ������Ʈ ����
		if (!bSprinting && fPressTime >= sprintTh)
		{
			m_pAnimator->SetBool("Sprint", true);
			bRunToggle = true;
			bSprinting = true;
		}
	}

	// �����̽� �ٸ� �� �� (�� ����)
	if (bSpaceHeld && bSpaceUp)
	{
		if (bMove)
		{
			if (!bSprinting)
			{
				// ª�� �������� ��� Ʈ����
				m_pAnimator->SetTrigger("Dash");
			}
			else
			{
				// ������Ʈ ����
				m_pAnimator->SetBool("Sprint", false);
			}
		}

		// ���� �ʱ�ȭ
		bSpaceHeld = false;
		bSprinting = false;
		fPressTime = 0.f;
	}

	// �̵��� �ߴܵǸ� ������Ʈ�� �ߴ�
	if (!bMove && bSprinting)
	{
		m_pAnimator->SetBool("Sprint", false);
		bSprinting = false;
		bSpaceHeld = false;
		fPressTime = 0.f;
	}

	// �ִϸ����� ���� ����
	if (m_pAnimator)
	{
		m_pAnimator->SetBool("Move", bMove);

		// ZŰ�� �� ���
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

	// 1. Ÿ�� ��ġ ��� (�÷��̾� ���� ������)
	_vector vPlayerPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vTargetEye = vPlayerPos + XMVectorSet(-50.f, 70.f, -50.f, 0.f);
	_vector vTargetAt = vPlayerPos;

	m_vShadowCam_Eye = vTargetEye;
	m_vShadowCam_At = vTargetAt;

	// 3. �ε��� ī�޶� ����
	XMStoreFloat4(&Desc.vEye, m_vShadowCam_Eye);
	XMStoreFloat4(&Desc.vAt, m_vShadowCam_At);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;

	Desc.fFovy = XMConvertToRadians(40.0f);
	if (FAILED(m_pGameInstance->Ready_Light_For_Shadow(Desc,SHADOW::SHADOWA)))
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
	// 1. ī�޶� ���� ���� ����
	_vector vCamLook = m_pCamera_Orbital->Get_TransfomCom()->Get_State(STATE::LOOK);
	_vector vCamRight = m_pCamera_Orbital->Get_TransfomCom()->Get_State(STATE::RIGHT);

	// 2. ���� ���⸸ ����
	vCamLook = XMVectorSetY(vCamLook, 0.f);
	vCamRight = XMVectorSetY(vCamRight, 0.f);
	vCamLook = XMVector3Normalize(vCamLook);
	vCamRight = XMVector3Normalize(vCamRight);

	// 3. �Է� ó��
	_vector vInputDir = XMVectorZero();
	if (m_pGameInstance->Key_Pressing(DIK_W)) vInputDir += vCamLook;
	if (m_pGameInstance->Key_Pressing(DIK_S)) vInputDir -= vCamLook;
	if (m_pGameInstance->Key_Pressing(DIK_D)) vInputDir += vCamRight;
	if (m_pGameInstance->Key_Pressing(DIK_A)) vInputDir -= vCamRight;

	// 4. �Է� ������ ����
	if (XMVector3Equal(vInputDir, XMVectorZero()))
		return;

	// 5. ���� ����ȭ
	vInputDir = XMVector3Normalize(vInputDir);

	// 6. ȸ��: ���� ����� �Է� ������ ���� ���
	_vector vPlayerLook = m_pTransformCom->Get_State(STATE::LOOK);
	vPlayerLook = XMVectorSetY(vPlayerLook, 0.f);
	vPlayerLook = XMVector3Normalize(vPlayerLook);

	_float fDot = XMVectorGetX(XMVector3Dot(vPlayerLook, vInputDir));
	fDot = max(-1.f, min(1.f, fDot)); // Clamp
	_float fAngle = acosf(fDot);

	// ȸ�� ���� Ȯ�� (�ð� or �ݽð�)
	_vector vCross = XMVector3Cross(vPlayerLook, vInputDir);
	if (XMVectorGetY(vCross) < 0.f)
		fAngle = -fAngle;

	// �ε巯�� ȸ�� ���� (���� ȸ�� �ӵ�)
	const _float fTurnSpeed = XMConvertToRadians(720.f); // �ʴ� 720�� ȸ��
	_float fClampedAngle = max(-fTurnSpeed * fTimeDelta, min(fTurnSpeed * fTimeDelta, fAngle));
	m_pTransformCom->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fClampedAngle);

	// 7. �̵� (�Է� �������� ���� ���� �̵�)
	m_pTransformCom->Go_Dir(vInputDir, fTimeDelta);
}

void CTestAnimObject::Ray()
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

			//  �ڱ� �ڽ��̸� ����
			if (hitActor == m_pControllerCom->Get_Actor())
			{
				printf(" Ray hit myself  skipping\n");
				return;
			}
			PxVec3 hitPos = hit.block.position;
			PxVec3 hitNormal = hit.block.normal;

			CPhysXActor* pHitActor = static_cast<CPhysXActor*>(hitActor->userData);
			pHitActor->Get_Owner()->On_Hit(this, m_pControllerCom->Get_ColliderType());

			printf("Ray�浹 �ߴ�!\n");
			printf("RayHitPos X: %f, Y: %f, Z: %f\n", hitPos.x, hitPos.y, hitPos.z);
			printf("RayHitNormal X: %f, Y: %f, Z: %f\n", hitNormal.x, hitNormal.y, hitNormal.z);
			m_bRayHit = true;
			m_vRayHitPos = hitPos;
			// ���� hit.block.���⿡ �� faceIndex, U, V �پ��ϰ� ������ �ñ��ϸ� ������.. 
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
