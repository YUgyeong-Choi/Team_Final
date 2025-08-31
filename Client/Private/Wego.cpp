#include "Wego.h"

#include "GameInstance.h"
#include "Camera_Manager.h"
#include "UIObject.h"
#include "UI_Manager.h"
CWego::CWego(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUnit(pDevice, pContext)
{
}

CWego::CWego(const CWego& Prototype)
	: CUnit(Prototype)
{
}

HRESULT CWego::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CWego::Initialize(void* pArg)
{
	WEGO_DESC* pDesc = static_cast<WEGO_DESC*>(pArg);

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	/* [ 초기화 위치값 ] */
	m_pTransformCom->Set_State(STATE::POSITION, _vector{ m_InitPos.x, m_InitPos.y, m_InitPos.z, 1.f });
	m_pTransformCom->SetUp_Scale(pDesc->InitScale.x, pDesc->InitScale.y, pDesc->InitScale.z);

	Ready_Collider();
	Ready_Trigger();

	// NPC 대화 데이터 
	LoadNpcTalkData("../Bin/Save/Npc/Wego_Temp.json");

	m_strNpcName = u8"초보 탐험가 휴고";

	LoadAnimDataFromJson();

	m_pAnimator->SetPlaying(true);

	return S_OK;
}

void CWego::Priority_Update(_float fTimeDelta)
{
	__super::Priority_Update(fTimeDelta);
	if (m_bFinish)
	{
		CCamera_Manager::Get_Instance()->SetbMoveable(true);
		m_bFinish = false;
	}

	for (auto& pButton : m_pSelectButtons)
		pButton->Priority_Update(fTimeDelta);
}

void CWego::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	// Talk 진행
	if (m_bTalkActive)
	{
		
		if (m_pGameInstance->Key_Down(DIK_SPACE))
		{
			++m_curTalkIndex;

			if(m_curTalkIndex < m_NpcTalkData[m_curTalkType].size())
			{
				// 버튼 로직을 생각하자
				if (m_curTalkIndex == m_iButtonActiveIndex + 1)
				{

					m_curTalkIndex = m_NextIndex[m_iSelectButtonIndex];
					m_CheckButtonSelct[m_iSelectButtonIndex] = true;
				}
				else if (m_curTalkIndex > m_iButtonActiveIndex + 1)
				{
					for (auto bCheck : m_CheckButtonSelct)
					{
						if (!bCheck)
						{
							m_curTalkIndex = m_iButtonActiveIndex;

			

							break;
						}
						m_curTalkIndex = _int(m_NpcTalkData[m_curTalkType].size() - 1);
					}

				}
			}
			else if (m_curTalkIndex >= m_NpcTalkData[m_curTalkType].size())
			{
			
				if (m_curTalkType == WEGOTALKTYPE::ONE)
					m_curTalkType = WEGOTALKTYPE::TWO;

				m_curTalkIndex = 0;
				m_bTalkActive = false;
				m_bFinish = true;
				CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);

				CUI_Manager::Get_Instance()->Activate_TalkScript(false);
				CUI_Manager::Get_Instance()->On_Panel();
				m_pAnimator->SetBool("TalkStart", false);
				return;
			}
			

			//wprintf(L"Wego: %s\n", m_NpcTalkData[m_curTalkType][m_curTalkIndex].c_str());
			CUI_Manager::Get_Instance()->Update_TalkScript(m_strNpcName, (m_NpcTalkData[m_curTalkType][m_curTalkIndex]), m_bAutoTalk);

			//
		}
		Update_Button();
	}

	// Talk 활성화
	if (m_bInTrigger)
	{
		if (m_pGameInstance->Key_Down(DIK_E) && !m_bTalkActive)
		{
			m_bTalkActive = true;
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(true, this, true, 1.7f);
			CCamera_Manager::Get_Instance()->SetbMoveable(false);

			//wprintf(L"Wego: %s\n", m_NpcTalkData[m_curTalkType][m_curTalkIndex].c_str());


			CUI_Manager::Get_Instance()->Off_Panel();
			CUI_Manager::Get_Instance()->Activate_Popup(false);
			CUI_Manager::Get_Instance()->Activate_TalkScript(true);
			CUI_Manager::Get_Instance()->Update_TalkScript(m_strNpcName, (m_NpcTalkData[m_curTalkType][m_curTalkIndex]), m_bAutoTalk);

			m_pAnimator->SetBool("TalkStart",true);
		}
	}

	// Talk 비활성화
	if (m_bTalkActive)
	{
		if (m_pGameInstance->Key_Down(DIK_ESCAPE))
		{
			m_curTalkIndex = 0;
			m_bTalkActive = false;
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
			m_bFinish = true;



			CUI_Manager::Get_Instance()->On_Panel();
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);
			m_pAnimator->SetBool("TalkStart", false);
		}
	}

	for (auto& pButton : m_pSelectButtons)
		pButton->Update(fTimeDelta);
}

void CWego::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);

	for (auto& pButton : m_pSelectButtons)
		pButton->Late_Update(fTimeDelta);
}

HRESULT CWego::Render()
{


	__super::Render();
#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderCollider()) {
		m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom);
		m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom);
	}
#endif
	
	

	return S_OK;
}


void CWego::On_CollisionEnter(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWego::On_CollisionStay(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWego::On_CollisionExit(CGameObject* pOther, COLLIDERTYPE eColliderType, _vector HitPos, _vector HitNormal)
{
}

void CWego::On_Hit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CWego::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	m_bInTrigger = true;

	CUI_Manager::Get_Instance()->Activate_Popup(true);
	CUI_Manager::Get_Instance()->Set_Popup_Caption(1);
}

void CWego::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	m_bInTrigger = false;
	CUI_Manager::Get_Instance()->Activate_Popup(false);

}

HRESULT CWego::Ready_Components()
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"),
		TEXT("Com_PhysX"), reinterpret_cast<CComponent**>(&m_pPhysXActorCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_PhysX_Dynamic"),
		TEXT("Com_PhysX2"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CWego::Ready_Collider()
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));
	positionVec.y += 0.5f;

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = { 0.2f,1.f,0.2f };
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXActorCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXActorCom->Set_ShapeFlag(true, false, true);

	PxFilterData filterData{};
	filterData.word0 = 0;
	filterData.word1 = 0; // 일단 보류
	m_pPhysXActorCom->Set_SimulationFilterData(filterData);
	m_pPhysXActorCom->Set_QueryFilterData(filterData);
	m_pPhysXActorCom->Set_Owner(this);
	m_pPhysXActorCom->Set_ColliderType(COLLIDERTYPE::NPC);
	m_pPhysXActorCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXActorCom->Get_Actor());

	return S_OK;
}

HRESULT CWego::Ready_Trigger()
{
	XMVECTOR S, R, T;
	XMMatrixDecompose(&S, &R, &T, m_pTransformCom->Get_WorldMatrix());

	PxVec3 scaleVec = PxVec3(XMVectorGetX(S), XMVectorGetY(S), XMVectorGetZ(S));
	PxQuat rotationQuat = PxQuat(XMVectorGetX(R), XMVectorGetY(R), XMVectorGetZ(R), XMVectorGetW(R));
	PxVec3 positionVec = PxVec3(XMVectorGetX(T), XMVectorGetY(T), XMVectorGetZ(T));

	PxTransform pose(positionVec, rotationQuat);
	PxMeshScale meshScale(scaleVec);

	PxVec3 halfExtents = { 1.f,0.2f,1.f };
	PxBoxGeometry geom = m_pGameInstance->CookBoxGeometry(halfExtents);
	m_pPhysXTriggerCom->Create_Collision(m_pGameInstance->GetPhysics(), geom, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXTriggerCom->Set_ShapeFlag(false, true, false);

	PxFilterData filterData{};
	filterData.word0 = WORLDFILTER::FILTER_NPC;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY; // 일단 보류
	m_pPhysXTriggerCom->Set_SimulationFilterData(filterData);
	m_pPhysXTriggerCom->Set_QueryFilterData(filterData);
	m_pPhysXTriggerCom->Set_Owner(this);
	m_pPhysXTriggerCom->Set_ColliderType(COLLIDERTYPE::NPC);
	m_pPhysXTriggerCom->Set_Kinematic(true);
	m_pGameInstance->Get_Scene()->addActor(*m_pPhysXTriggerCom->Get_Actor());

	return S_OK;
}

void CWego::LoadNpcTalkData(string filePath)
{
	ifstream inFile(filePath);
	if (inFile.is_open())
	{
		json j;
		inFile >> j;
		inFile.close();

		// j가 배열 형태일 것으로 가정
		for (const auto& item : j)
		{
			int type = item["Type"];
			WEGOTALKTYPE talkType = (type == 0) ? WEGOTALKTYPE::ONE : WEGOTALKTYPE::TWO;

			// Words 배열 읽기
			vector<string> words;
			for (const auto& word : item["Words"])
			{
				// string -> wstring 변환
				string s = word.get<std::string>();
			
				words.push_back(s);
			}

			// 맵에 저장
			m_NpcTalkData[talkType] = std::move(words);


			if (item.contains("ButtonActiveIndex"))
			{
				m_iButtonActiveIndex = item["ButtonActiveIndex"].get<_int>();



				for (auto& button : item["Buttons"])
				{
					auto pObj = m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Button_Script"));

					m_pSelectButtons.push_back(static_cast<CUI_Button_Script*>(pObj));

					string strCaption = button["Label"].get<string>();

					m_pSelectButtons.back()->Update_Script((strCaption));
					m_pSelectButtons.back()->Active_Update(false);

					m_NextIndex.push_back(button["NextIndex"].get<_int>());
					m_CheckButtonSelct.push_back(false);
				}

				// 위치 잡아주기

				_float2 vSize = m_pSelectButtons.back()->Get_ButtonPos();

				size_t iNum = m_pSelectButtons.size();

				for (size_t i =0; i< iNum; ++i)
				{
					m_pSelectButtons[i]->Update_Position(vSize.x, vSize.y - g_iWinSizeY * 0.07f * (iNum - 1 - i));
				}

				m_pSelectButtons[0]->Set_isSelect(true);

			}




		}
	}
}

void CWego::LoadAnimDataFromJson()
{
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


}

void CWego::Update_Button()
{
	if (m_curTalkIndex == m_iButtonActiveIndex)
	{
		for (auto& button : m_pSelectButtons)
			button->Active_Update(true);

		if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
		{
			for (size_t i = 0; i < m_pSelectButtons.size(); ++i)
			{
				m_pSelectButtons[i]->Check_Select();
				if (m_pSelectButtons[i]->Get_isSelect())
				{
					m_iSelectButtonIndex = _int(i);
				}
			}



		}

	
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
				m_iSelectButtonIndex = _int(m_pSelectButtons.size() - 1);
			m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(true);
		}

		

	}
	else
	{
		for (auto& button : m_pSelectButtons)
			button->Active_Update(false);
	}
	
}

CWego* CWego::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CWego* pInstance = new CWego(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CWego");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CWego::Clone(void* pArg)
{
	CWego* pInstance = new CWego(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CWego");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CWego::Free()
{
	__super::Free();
	Safe_Release(m_pModelCom);
	Safe_Release(m_pAnimator);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pPhysXActorCom);
	Safe_Release(m_pPhysXTriggerCom);

	for (auto& pButton : m_pSelectButtons)
		Safe_Release(pButton);
}
