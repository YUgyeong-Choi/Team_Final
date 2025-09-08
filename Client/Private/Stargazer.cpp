#include "Stargazer.h"
#include "GameInstance.h"
#include "Player.h"
#include "Level.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"
#include "UI_Script_StarGazer.h"
#include "UI_Button_Script.h"
#include "UI_SelectLocation.h"

CStargazer::CStargazer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
	, m_pShaderCom(nullptr)
{

}

CStargazer::CStargazer(const CStargazer& Prototype)
	:CGameObject(Prototype)
	, m_pShaderCom(Prototype.m_pShaderCom)
{

	Safe_AddRef(m_pShaderCom);
}

HRESULT CStargazer::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CStargazer::Initialize(void* pArg)
{
	STARGAZER_DESC* pDesc = static_cast<STARGAZER_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components(pArg)))
		return E_FAIL;

	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
	{
		LoadAnimDataFromJson(m_pModelCom[i], m_pAnimator[i]);
	}

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);

	m_eStargazerTag = pDesc->eStargazerTag;

	m_eState = STARGAZER_STATE::DESTROYED;

	Register_Events();

	LoadScriptData();

	if (FAILED(Ready_Collider()))
		return E_FAIL;
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

void CStargazer::Priority_Update(_float fTimeDelta)
{
	if(m_pPlayer == nullptr)
		Find_Player();

	//�μ�����
	//AS_Stargazer_broken_idle
	//AS_Stargazer_restore 
	
	//�μ����� (Restore)������
	//�����Ѱ� (Open) ����

	//�����Ѱ�
	//AS_Close_Idle 
	//AS_Open
	//AS_Open_Idle

	if (Check_Player_Close())
	{
		if (m_eState == STARGAZER_STATE::DESTROYED)
		{

			CUI_Manager::Get_Instance()->Activate_Popup(true);
			CUI_Manager::Get_Instance()->Set_Popup_Caption(3);
		}
		else if (STARGAZER_STATE::FUNCTIONAL == m_eState)
		{
			if (!m_bTalkActive && (nullptr == m_pScript))
			{
				CUI_Manager::Get_Instance()->Activate_Popup(true);
				CUI_Manager::Get_Instance()->Set_Popup_Caption(4);
			}
			
		}

		if (m_pGameInstance->Key_Down(DIK_E))
		{
			if (m_eState == STARGAZER_STATE::DESTROYED)
			{
				// ���߿� Ʈ���ŷ� �ٲٱ�
				m_pAnimator[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]->SetTrigger("Restore");

				//m_eState = STARGAZER_STATE::FUNCTIONAL;
				CUI_Manager::Get_Instance()->Activate_Popup(false);
				return;
			}
			else if (STARGAZER_STATE::FUNCTIONAL == m_eState)
			{
				if (m_eScriptDatas.empty())
				{
					// �ٷ� ���ٶ��� ��ũ��Ʈ�� ����, ������ �� �ִ� ��ư�� ����

					if (nullptr == m_pScript)
					{
						
						Ready_Script();
						

						CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(true, this, true, 1.7f);
						CCamera_Manager::Get_Instance()->SetbMoveable(false);
						CUI_Manager::Get_Instance()->Off_Panel();
						CUI_Manager::Get_Instance()->Activate_Popup(false);
						

						return;
					}

				}
				else
				{
					// ��ȭ�� ��ũ��Ʈ�� ����, ��ȭ�� ����Ǹ� clear �ع�����
					if (!m_bTalkActive)
					{
						m_bTalkActive = true;
						CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(true, this, true, 1.7f);
						CCamera_Manager::Get_Instance()->SetbMoveable(false);

						//wprintf(L"Wego: %s\n", m_NpcTalkData[m_curTalkType][m_curTalkIndex].c_str());


						CUI_Manager::Get_Instance()->Off_Panel();
						CUI_Manager::Get_Instance()->Activate_Popup(false);
						CUI_Manager::Get_Instance()->Activate_TalkScript(true);
						CUI_Manager::Get_Instance()->Update_TalkScript(m_eScriptDatas[m_iScriptIndex].strSpeaker, (m_eScriptDatas[m_iScriptIndex].strSoundText), false);

					
					}
				
				}
			}

		}
		else if (m_pGameInstance->Key_Down(DIK_SPACE))
		{
			if (m_bTalkActive)
			{
				++m_iScriptIndex;

				// ��ȭ �� �� ���.
				if (m_iScriptIndex >= m_eScriptDatas.size())
				{

					m_iScriptIndex = 0;
					m_bTalkActive = false;
					m_eScriptDatas.clear();
					CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
					CUI_Manager::Get_Instance()->Activate_TalkScript(false);

					Ready_Script();
					return;
				}


				CUI_Manager::Get_Instance()->Update_TalkScript(m_eScriptDatas[m_iScriptIndex].strSpeaker, (m_eScriptDatas[m_iScriptIndex].strSoundText), false);
			}

			// ��ũ��Ʈ�� ������ ���� ��ư�� ������Ʈ �� �� �ְ�
			if (nullptr != m_pScript)
			{
				Button_Interaction();

			}

		}
		else if (m_pGameInstance->Key_Down(DIK_ESCAPE))
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
			CCamera_Manager::Get_Instance()->SetbMoveable(false);
			CUI_Manager::Get_Instance()->On_Panel();
			CUI_Manager::Get_Instance()->Activate_Popup(false);
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);

			Delete_Script();
			return;
		}


	}
	

	if (nullptr != m_pScript)
	{
		m_pScript->Priority_Update(fTimeDelta);
	}

	Update_Button();


	//���º���
	//if (m_pGameInstance->Key_Down(DIK_C))
	//{
	//	if (m_eState == STARGAZER_STATE::DESTROYED)
	//		m_eState = STARGAZER_STATE::FUNCTIONAL;
	//	else
	//		m_eState = STARGAZER_STATE::DESTROYED;
	//}

	//if (m_pGameInstance->Key_Down(DIK_R))
	//{
	//	m_pAnimator[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]->SetTrigger("Restore");
	//}

	//if (m_pGameInstance->Key_Down(DIK_O))
	//{
	//	m_pAnimator[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]->SetTrigger("Open");
	//}

	//�÷��̾�� ������ E�� ������ �ٸ� ���ٶ��� �̵�(�׽�Ʈ)
	//�÷��̾������� �ڵ� �ű�� �۾� �ʿ�������
	if (m_pGameInstance->Key_Down(DIK_E))
	{
		if (Check_Player_Close())
		{
			//����
			m_pGameInstance->Get_CurrentLevel()->Reset();
			m_pPlayer->Reset();
		}
	}

	if (m_pGameInstance->Key_Down(DIK_0))
	{
		if (Check_Player_Close()) 
		{
			Teleport_Stargazer(STARGAZER_TAG::OUTER);
		}
	}

	if (m_pGameInstance->Key_Down(DIK_9))
	{
		if (Check_Player_Close())
		{
			Teleport_Stargazer(STARGAZER_TAG::FIRE_EATER);
		}
	}

	if (m_pGameInstance->Key_Down(DIK_MINUS))
	{
		if (Check_Player_Close())
		{
			Teleport_Stargazer(STARGAZER_TAG::FESTIVAL_LEADER);
		}
	}
	 
}

void CStargazer::Update(_float fTimeDelta)
{
	/* [ �ִϸ��̼� ������Ʈ ] */
	if (m_pAnimator[ENUM_CLASS(m_eState)])
		m_pAnimator[ENUM_CLASS(m_eState)]->Update(fTimeDelta);

	if (m_pModelCom[ENUM_CLASS(m_eState)])
		m_pModelCom[ENUM_CLASS(m_eState)]->Update_Bones();

	if (nullptr != m_pScript)
	{
		m_pScript->Update(fTimeDelta);

		for (auto& pButton : m_pSelectButtons)
			if (nullptr != pButton)
				pButton->Update(fTimeDelta);

	}

	

}

void CStargazer::Late_Update(_float fTimeDelta)
{
	if (nullptr != m_pScript)
	{
		m_pScript->Late_Update(fTimeDelta);

		for (auto& pButton : m_pSelectButtons)
			if (nullptr != pButton)
				pButton->Late_Update(fTimeDelta);
	}




	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CStargazer::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom[ENUM_CLASS(m_eState)]->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);


		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eState)]->Render(i)))
			return E_FAIL;
	}

	if (m_pGameInstance->Get_RenderMapCollider())
	{
		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;
	}

	return S_OK;
}

void CStargazer::Script_Activate()
{
	if (m_pSelectButtons.empty())
		return;

	m_pScript->Active_Update(true);
	for (auto& pButton : m_pSelectButtons)
		if (nullptr != pButton)
			pButton->Active_Update(true);
}

void CStargazer::Delete_Script()
{
	if (m_pSelectButtons.empty())
		return;

	Safe_Release(m_pScript);

	for (auto& pButton : m_pSelectButtons)
		Safe_Release(pButton);

	m_pSelectButtons.clear();
}

void CStargazer::LoadAnimDataFromJson(CModel* pModel, CAnimator* pAnimator)
{
	string path = "../Bin/Save/AnimationEvents/" + pModel->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = pModel->GetAnimations();

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

	path = "../Bin/Save/AnimationStates/" + pModel->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		pAnimator->Deserialize(rootStates);
	}
}

void CStargazer::Find_Player()
{
	m_pPlayer = static_cast<CPlayer*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player")));
	Safe_AddRef(m_pPlayer);
}

void CStargazer::Register_Events()
{
	m_pAnimator[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]->RegisterEventListener("ChangeModel", [this]()
		{
			if (m_eState == STARGAZER_STATE::DESTROYED)
				m_eState = STARGAZER_STATE::FUNCTIONAL;
			m_pAnimator[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]->SetTrigger("Open");
		});
}

void CStargazer::Teleport_Stargazer(STARGAZER_TAG eTag)
{
	//���ٶ�� ����Ʈ���� ���� �±׸� ã�� �װ����� �÷��̾� �̵����Ѷ�

	list<CGameObject*>& ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Stargazer"));

	for (CGameObject* pObj : ObjList)
	{
		if (static_cast<CStargazer*>(pObj)->m_eStargazerTag == eTag)
		{
			_float3 vPos;
			XMStoreFloat3(&vPos, pObj->Get_TransfomCom()->Get_State(STATE::POSITION));

			//��¦ ����
			vPos.y += 1.f;
			//��¦ �ڷ�
			vPos.x -= 2.f;

			PxVec3 pxPos(vPos.x, vPos.y, vPos.z);

			PxTransform posTrans = PxTransform(pxPos);

			m_pPlayer->Get_Controller()->Set_Transform(posTrans);

			break;
		}
	}

	

	CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
	CCamera_Manager::Get_Instance()->SetbMoveable(true);
	CUI_Manager::Get_Instance()->On_Panel();
	CUI_Manager::Get_Instance()->Activate_Popup(false);
	CUI_Manager::Get_Instance()->Activate_TalkScript(false);

	Delete_Script();
}

_bool CStargazer::Check_Player_Close()
{
	//�÷��̾ ������� üũ
	_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
	_vector vPos = m_pTransformCom->Get_State(STATE::POSITION);
	_vector vDiff = vPos - vPlayerPos;
	_float fDist = XMVectorGetX(XMVector3Length(vDiff));

	if (fDist < 2.f)
		return true;
	else 
		return false;
}

void CStargazer::LoadScriptData()
{
	// �� Ÿ�Կ� �°� ������ ������
	string filename = "../Bin/Save/Stargazer/Stargazer_" + to_string(ENUM_CLASS(m_eStargazerTag)) + ".json";

	ifstream ifs(filename);
	// ������ ���� - ��ũ��Ʈ �����Ͱ� ����
	if (!ifs.is_open())
		return;

	// ��ũ��Ʈ �����Ͱ� �����ϸ�, �־��ش�
	json j;

	ifs >> j;

	for (auto& entry : j["ScriptDatas"])
	{
		TALKDATA data;
		data.strSoundTag = entry["SoundName"].get<std::string>();
		data.strSpeaker = entry["Speaker"].get<std::string>();
		data.strSoundText = entry["Text"].get<std::string>();
		m_eScriptDatas.push_back(data);
	}

	
}

void CStargazer::Update_Button()
{
	if (m_pSelectButtons.empty())
		return;

	if (m_pGameInstance->Key_Down(DIK_W))
	{
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(false);
		--m_iSelectButtonIndex;
		if (m_iSelectButtonIndex < 0)
			m_iSelectButtonIndex = 0;

		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(true);
	}
	else if (m_pGameInstance->Key_Down(DIK_S))
	{
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(false);
		++m_iSelectButtonIndex;
		if (m_iSelectButtonIndex >= m_pSelectButtons.size())
			m_iSelectButtonIndex = static_cast<_int>(m_pSelectButtons.size()) - 1;
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(true);
	}
}

void CStargazer::Button_Interaction()
{
	if (m_pSelectButtons.empty())
		return;

	// �� �̺�Ʈ �̸��� �´� ui�� ����
	// add object�� �����, target�� �־��༭ ����� �Ѵ�.

	if ("SelectLocation" == m_ButtonEvents[m_iSelectButtonIndex])
	{
		// ��� ���� ui �����

		CUI_SelectLocation::UI_SELECT_LOCATION_DESC eDesc = {};
		eDesc.pTarget = this;

		m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_SelectLocation"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_UI_SelectLocation"), &eDesc);

		m_pScript->Active_Update(false);
		for (auto& pButton : m_pSelectButtons)
			if (nullptr != pButton)
				pButton->Active_Update(false);
	}
	else if ("LevelUp" == m_ButtonEvents[m_iSelectButtonIndex])
	{
		// ������ ui �����
	}

	

}

HRESULT CStargazer::Ready_Script()
{
	m_pScript = static_cast<CUI_Script_StarGazer*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT,
		ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Script_Stargazer"), nullptr));

	// ���� ã�Ƽ�
	string strFilePath = ("../Bin/Save/Stargazer/Stargazer_Select.json");
	json j;

	ifstream file(strFilePath);

	file >> j;

	list<CGameObject*>& ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Stargazer"));



	size_t iSize = {};

	for (CGameObject* pObj : ObjList)
	{
		if (static_cast<CStargazer*>(pObj)->m_eState == STARGAZER_STATE::FUNCTIONAL)
		{
			++iSize;
		}
	}

	for (auto& button : j["Buttons"])
	{
		if (button["RequiredSize"].get<_int>() > iSize)
			continue;

		auto pObj = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button_Script"));

		m_pSelectButtons.push_back(static_cast<CUI_Button_Script*>(pObj));

		string strCaption = button["Label"].get<string>();

		m_pSelectButtons.back()->Update_Script((strCaption));

		m_ButtonEvents.push_back(button["Event"].get<string>());

	}

	// ��ġ ����ֱ�

	_float2 vSize = m_pSelectButtons.back()->Get_ButtonPos();

	size_t iNum = m_pSelectButtons.size();

	for (size_t i = 0; i < iNum; ++i)
	{
		m_pSelectButtons[i]->Update_Position(vSize.x, vSize.y - g_iWinSizeY * 0.07f * (iNum - 1 - i));
	}

	m_pSelectButtons[0]->Set_isSelect(true);


	return S_OK;
}

HRESULT CStargazer::Bind_ShaderResources()
{
	/* [ ���� �����̽� �ѱ�� ] */
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	/* [ �� , ���� �����̽� �ѱ�� ] */
	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	return S_OK;
}

HRESULT CStargazer::Ready_Collider()
{
	// 3. Transform���� S, R, T �и�
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	// 3-1. ������, ȸ��, ��ġ ��ȯ
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

	_float3 vHalf = _float3(0.5f, 0.5f, 0.5f);

	halfExtents = VectorToPxVec3(XMLoadFloat3(&vHalf));
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::A);
	//m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

HRESULT CStargazer::Ready_Components(void* pArg)
{
	/* Com_Model */
	if (FAILED(__super::Add_Component(m_iLevelID, _wstring(TEXT("Prototype_Component_Model_Stargazer_Destroyed")),
		TEXT("Com_Model_Destroyed"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(m_iLevelID, _wstring(TEXT("Prototype_Component_Model_Stargazer")),
		TEXT("Com_Model_Functional"), reinterpret_cast<CComponent**>(&m_pModelCom[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
	{
		m_pAnimator[i] = CAnimator::Create(m_pDevice, m_pContext);
		if (nullptr == m_pAnimator)
			return E_FAIL;

		if (FAILED(m_pAnimator[i]->Initialize(m_pModelCom[i])))
			return E_FAIL;
	}

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;


	return S_OK;
}

CStargazer* CStargazer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CStargazer* pGameInstance = new CStargazer(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CStaticMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

CGameObject* CStargazer::Clone(void* pArg)
{
	CStargazer* pInstance = new CStargazer(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CStargazer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CStargazer::Free()
{
	__super::Free();

	for (_uint i = 0; i < ENUM_CLASS(STARGAZER_STATE::END); ++i)
	{
		Safe_Release(m_pModelCom[i]);
		Safe_Release(m_pAnimator[i]);
	}
	
	Safe_Release(m_pPlayer);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pPhysXActorCom);

	Safe_Release(m_pScript);
	for (auto& pButton : m_pSelectButtons)
		Safe_Release(pButton);
}
