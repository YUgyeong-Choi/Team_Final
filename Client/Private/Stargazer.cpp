#include "Stargazer.h"
#include "GameInstance.h"
#include "Player.h"
#include "Level.h"
#include "UI_Manager.h"
#include "Camera_Manager.h"
#include "UI_Script_StarGazer.h"
#include "UI_Button_Script.h"
#include "UI_SelectLocation.h"
#include "UI_Levelup.h"
#include "StargazerEffect.h"
#include "UI_Guide.h"
#include "PhysXStaticActor.h"


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

	if (FAILED(Ready_EffectSet()))
		return E_FAIL;

	return S_OK;
}

void CStargazer::Priority_Update(_float fTimeDelta)
{
	if(m_pPlayer == nullptr)
		Find_Player();

	
	if (!m_bUseOtherUI)
	{

		if (!m_isCollison)
			return;

		Update_Button();

		if (m_pGameInstance->Key_Down(DIK_F))
		{
			if (m_bTalkActive)
			{
				m_bAutoTalk = !m_bAutoTalk;

				CUI_Manager::Get_Instance()->Update_TalkScript(m_eScriptDatas[m_iScriptIndex].strSpeaker, (m_eScriptDatas[m_iScriptIndex].strSoundText), m_bAutoTalk);
			}
		}

		if (m_bAutoTalk && m_bTalkActive)
		{
			_bool bIsPlaying = false;
			bIsPlaying = m_pSoundCom->IsPlaying(m_eScriptDatas[m_iScriptIndex].strSoundTag);

			if (!bIsPlaying)
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

				CUI_Manager::Get_Instance()->Update_TalkScript(m_eScriptDatas[m_iScriptIndex].strSpeaker, (m_eScriptDatas[m_iScriptIndex].strSoundText), m_bAutoTalk);

				m_pSoundCom->SetVolume(m_eScriptDatas[m_iScriptIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
				m_pSoundCom->Play(m_eScriptDatas[m_iScriptIndex].strSoundTag);
			}

		}


		if (m_pGameInstance->Key_Down(DIK_SPACE))
		{

			// ��ũ��Ʈ�� ������ ���� ��ư�� ������Ʈ �� �� �ְ�
			if (nullptr != m_pScript)
			{
				Button_Interaction();

					
			}

			if (m_bTalkActive)
			{
				m_pSoundCom->StopAll();
				if (nullptr != m_pGuide)
				{
					m_bUseOtherUI = true;
				}
					
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

					

				CUI_Manager::Get_Instance()->Update_TalkScript(m_eScriptDatas[m_iScriptIndex].strSpeaker, (m_eScriptDatas[m_iScriptIndex].strSoundText), m_bAutoTalk);

				m_pSoundCom->SetVolume(m_eScriptDatas[m_iScriptIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
				m_pSoundCom->Play(m_eScriptDatas[m_iScriptIndex].strSoundTag);

			}

		}
		else if (m_pGameInstance->Key_Down(DIK_E))
		{
			if (m_eState == STARGAZER_STATE::DESTROYED)
			{
				if (m_bIsRotatingToStargazer == false)
					m_bIsRotatingToStargazer = true;
				m_pPlayer->OnTriggerEvent(CPlayer::eTriggerEvent::STARGAZER_RESTORE_START);
				m_pAnimator[ENUM_CLASS(STARGAZER_STATE::DESTROYED)]->SetTrigger("Restore");
				CUI_Manager::Get_Instance()->Activate_Popup(false);
				m_pEffectSet->Activate_Stargazer_Reassemble();

				m_pSoundCom->SetVolume("AMB_OJ_PR_Stargazer_Open_01", g_fInteractSoundVolume);
				m_pSoundCom->Play("AMB_OJ_PR_Stargazer_Open_01");

				m_pSoundCom->SetVolume("AMB_OJ_PR_Stargazer_Restore_Activated", g_fInteractSoundVolume);
				m_pSoundCom->Play("AMB_OJ_PR_Stargazer_Restore_Activated");

				if (m_eStargazerTag == STARGAZER_TAG::FESTIVAL_LEADER_IN)
				{
					list<CGameObject*>& ObjList = m_pGameInstance->Get_ObjectList(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Stargazer"));

					for (CGameObject* pObj : ObjList)
					{
						if (static_cast<CStargazer*>(pObj)->m_eStargazerTag == STARGAZER_TAG::FIRE_EATER)
						{
							static_cast<CStargazer*>(pObj)->Restore();

							break;
						}
					}
				}

				return;
			}
			else if (STARGAZER_STATE::FUNCTIONAL == m_eState)
			{
				if (m_bIsRotatingToStargazer == false)
					m_bIsRotatingToStargazer = true;
				m_pPlayer->OnTriggerEvent(CPlayer::eTriggerEvent::STARGAZER_ACTIVATE_START);
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

						m_pPlayer->Reset();
						m_pGameInstance->Get_CurrentLevel()->Reset();

						// 
						_float3 vPos;

						XMStoreFloat3(&vPos, Get_TransfomCom()->Get_State(STATE::POSITION));

						// �÷��̾� ��ġ ����
						vPos.y += 1.f;
						vPos.x -= 2.5f;
						m_pPlayer->SetTeleportPos(vPos);

						return;
					}
					else
					{
						Script_Activate();
						m_pPlayer->Reset();
						m_pGameInstance->Get_CurrentLevel()->Reset();
						return;
					}

				}
				else
				{
					// ��ȭ�� ��ũ��Ʈ�� ����, ��ȭ�� ����Ǹ� clear �ع�����
					if (!m_bTalkActive)
					{
						if (!m_isMakeGuide)
						{
							CUI_Guide::UI_GUIDE_DESC eGuideDesc{};
							eGuideDesc.partPaths = { TEXT("../Bin/Save/UI/Guide/Guide_Stargazer.json") };

							m_pGuide = static_cast<CUI_Guide*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, 
																ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Guide"), &eGuideDesc));
					


							m_isMakeGuide = true;
							m_bUseOtherUI = true;


							m_pPlayer->Reset();
							m_pGameInstance->Get_CurrentLevel()->Reset();

							_float3 vPos;

							XMStoreFloat3(&vPos, Get_TransfomCom()->Get_State(STATE::POSITION));

							// �÷��̾� ��ġ ����
							vPos.y += 1.f;
							vPos.x -= 2.5f;
							m_pPlayer->SetTeleportPos(vPos);


							return;
						}

					}

				}
			}

		}

		else if (m_pGameInstance->Key_Down(DIK_ESCAPE))
		{
			CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
			CCamera_Manager::Get_Instance()->SetbMoveable(true);
			CUI_Manager::Get_Instance()->On_Panel();
			CUI_Manager::Get_Instance()->Activate_Popup(false);
			CUI_Manager::Get_Instance()->Activate_TalkScript(false);
			m_pPlayer->OnTriggerEvent(CPlayer::eTriggerEvent::STARGAZER_ACTIVATE_END);
			m_bTalkActive = false;
			m_bUseScript = false;
			m_bUseOtherUI = false;

			Delete_Script();
			m_pSoundCom->StopAll();
			return;
		}

	}
	
	

	if (nullptr != m_pScript)
	{
		m_pScript->Priority_Update(fTimeDelta);
	}
	if (nullptr != m_pGuide)
	{
		m_pGuide->Priority_Update(fTimeDelta);
		if (m_pGuide->Get_bDead())
		{
			Safe_Release(m_pGuide);
			m_pGuide = nullptr;
			m_bUseOtherUI = false;
		}
	}

	if (m_bUseTeleport)
	{
		m_bUseTeleport = false;
		m_bUseOtherUI = false;

		CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(false, nullptr, true, 0.f);
		CUI_Manager::Get_Instance()->Activate_Popup(false);
		CUI_Manager::Get_Instance()->Activate_TalkScript(false);

	}


	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		//��ȭ ������ ���� ��ȭ�� �̵�
		if (nullptr != m_pGuide)
		{

			m_bUseOtherUI = false;

			if (!m_eScriptDatas.empty())
			{
				m_bTalkActive = true;
				CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_ActiveTalk(true, this, true, 1.7f);
				CCamera_Manager::Get_Instance()->SetbMoveable(false);

				CUI_Manager::Get_Instance()->Off_Panel();
				CUI_Manager::Get_Instance()->Activate_Popup(false);
				CUI_Manager::Get_Instance()->Activate_TalkScript(true);
				CUI_Manager::Get_Instance()->Update_TalkScript(m_eScriptDatas[m_iScriptIndex].strSpeaker, (m_eScriptDatas[m_iScriptIndex].strSoundText), m_bAutoTalk);

				m_pSoundCom->SetVolume(m_eScriptDatas[m_iScriptIndex].strSoundTag, 0.5f * g_fInteractSoundVolume);
				m_pSoundCom->Play(m_eScriptDatas[m_iScriptIndex].strSoundTag);
			}

		

		}
	}

	if (m_bIsRotatingToStargazer)
	{
		// ���ٶ�� �ٶ󺸰� �ϱ�
		_vector vPlayerPos = m_pPlayer->Get_TransfomCom()->Get_State(STATE::POSITION);
		_vector vStargazerPos = this->Get_TransfomCom()->Get_State(STATE::POSITION);

		vStargazerPos = XMVectorSetY(vStargazerPos, XMVectorGetY(vPlayerPos));

		_vector vDir = XMVector3Normalize(vStargazerPos - vPlayerPos);

		if (m_pPlayer->Get_TransfomCom()->RotateToDirectionSmoothly(vDir, fTimeDelta))
		{
			m_bIsRotatingToStargazer = false;
		}
	}

}

void CStargazer::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

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

	if (nullptr != m_pGuide)
		m_pGuide->Update(fTimeDelta);

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


	if (nullptr != m_pGuide)
		m_pGuide->Late_Update(fTimeDelta);


	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CStargazer::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;


	if (m_bIsDissolve)
	{
		if (FAILED(m_pDissolveMap->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
			return E_FAIL;

		_bool vDissolve = true;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bDissolve", &vDissolve, sizeof(_bool))))
			return E_FAIL;

		if (m_vecDissolveMeshNum.empty())
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveGlowColor", &m_vDissolveGlowColor, sizeof(_float3))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &m_fDissolve, sizeof(_float))))
				return E_FAIL;
		}
	}
	else
	{
		_bool vDissolve = false;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_bDissolve", &vDissolve, sizeof(_bool))))
			return E_FAIL; 
	}

	_uint		iNumMesh = m_pModelCom[ENUM_CLASS(m_eState)]->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (m_bIsDissolve && m_vecDissolveMeshNum.size() > 0)
		{
			auto iter = find(m_vecDissolveMeshNum.begin(), m_vecDissolveMeshNum.end(), i);
			if (iter != m_vecDissolveMeshNum.end())
			{
				if (FAILED(m_pShaderCom->Bind_RawValue("g_vDissolveGlowColor", &m_vDissolveGlowColor, sizeof(_float3))))
					return E_FAIL;
				if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &m_fDissolve, sizeof(_float))))
					return E_FAIL;
			}
			else
			{
				_float fDissolve = 1.f;
				if (FAILED(m_pShaderCom->Bind_RawValue("g_fDissolveAmount", &fDissolve, sizeof(_float))))
					return E_FAIL;
			}
		}

		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);
		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0);


		m_pModelCom[ENUM_CLASS(m_eState)]->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

		if (FAILED(m_pShaderCom->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pModelCom[ENUM_CLASS(m_eState)]->Render(i)))
			return E_FAIL;
	}

#ifdef _DEBUG
	if (m_pGameInstance->Get_RenderMapCollider())
	{
		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXActorCom)))
			return E_FAIL;

		if (FAILED(m_pGameInstance->Add_DebugComponent(m_pPhysXTriggerCom)))
			return E_FAIL;
	}
#endif

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

	m_iSelectButtonIndex = -1;

	m_bUseScript = true;
	m_bUseOtherUI = false;
}

void CStargazer::Delete_Script()
{
	if (m_pSelectButtons.empty())
		return;

	Safe_Release(m_pScript);
	m_pScript = nullptr;

	for (auto& pButton : m_pSelectButtons)
		Safe_Release(pButton);

	m_pSelectButtons.clear();
	m_ButtonEvents.clear();

	m_iSelectButtonIndex = -1;
	m_iScriptIndex = 0;
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
			m_pPlayer->OnTriggerEvent(CPlayer::eTriggerEvent::STARGAZER_RESTORE_END);
			m_pEffectSet->Activate_Stargazer_Spread();
			
			if (m_eStargazerTag != STARGAZER_TAG::FIRE_EATER)
			{
				CUI_Container::UI_CONTAINER_DESC eDesc{};
				eDesc.fLifeTime = 8.f;
				eDesc.useLifeTime = true;
				eDesc.strFilePath = TEXT("../Bin/Save/UI/StargazerActivated.json");

				m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"),
					m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_UI_Activated"), &eDesc);

				CUI_Container* pObj = static_cast<CUI_Container*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_UI_Activated")));

				static_cast<CDynamic_UI*>(pObj->Get_PartUI().back())->Set_isUVmove(true);
			}

		

			


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
			// �÷��̾ �ڷ���Ʈ �ڼ��� ��´�
			m_pPlayer->Start_Teleport();

			// ���õ� ���ٶ���� ��ġ�� �����´�.
			_float3 vPos;
			XMStoreFloat3(&vPos, pObj->Get_TransfomCom()->Get_State(STATE::POSITION));

			// �÷��̾� ��ġ ����
			vPos.y += 1.f;
			vPos.x -= 2.5f;
			m_pPlayer->SetTeleportPos(vPos);

			break;
		}
	}

	


	Delete_Script();
	m_bUseTeleport = true;
	m_bUseOtherUI = false;
	m_iSelectButtonIndex = -1;
}

void CStargazer::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_isCollison)
	{
		CUI_Manager::Get_Instance()->Activate_Popup(true);

		if (m_eState == STARGAZER_STATE::DESTROYED)
			CUI_Manager::Get_Instance()->Set_Popup_Caption(3);
		else if(m_eState == STARGAZER_STATE::FUNCTIONAL)
			CUI_Manager::Get_Instance()->Set_Popup_Caption(4);

		m_isCollison = true;
	}
}

void CStargazer::On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (m_isCollison)
	{
		if (m_eState == STARGAZER_STATE::FUNCTIONAL)
		{
			CUI_Manager::Get_Instance()->Set_Popup_Caption(4);
			if(m_bUseScript || m_bTalkActive || m_bUseOtherUI || m_bUseTeleport || m_pPlayer->Get_IsTeleport())
				CUI_Manager::Get_Instance()->Activate_Popup(false);
			else
				CUI_Manager::Get_Instance()->Activate_Popup(true);
		}
			
	}
}

void CStargazer::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	m_isCollison = false;	
	CUI_Manager::Get_Instance()->Activate_Popup(false);

	m_bTalkActive = false;
	m_bUseScript = false;
	m_bUseOtherUI = false;
	
}

void CStargazer::Restore()
{
	if (m_eState == STARGAZER_STATE::DESTROYED)
	{
		m_eState = STARGAZER_STATE::FUNCTIONAL;
		m_pAnimator[ENUM_CLASS(STARGAZER_STATE::FUNCTIONAL)]->SetTrigger("Open");

		
	}
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

	if (!m_bUseScript || m_bUseOtherUI)
		return;


	if (m_iSelectButtonIndex < 0)
	{
		m_iSelectButtonIndex = 0;
		return;
	}
	else if (m_iSelectButtonIndex >= static_cast<_int>(m_pSelectButtons.size()))
	{
		m_iSelectButtonIndex = static_cast<_int>(m_pSelectButtons.size()) - 1;
		return;
	}
		

	if (m_pGameInstance->Key_Down(DIK_W))
	{
		// ���� ��ư ����
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(false);

		// �ε��� ���� + ����
		--m_iSelectButtonIndex;
		if (m_iSelectButtonIndex < 0)
			m_iSelectButtonIndex = 0;

		// ���ο� ��ư ����
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(true);

		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
	}
	else if (m_pGameInstance->Key_Down(DIK_S))
	{
		// ���� ��ư ����
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(false);

		// �ε��� ���� + ����
		++m_iSelectButtonIndex;
		if (m_iSelectButtonIndex >= static_cast<_int>(m_pSelectButtons.size()))
			m_iSelectButtonIndex = static_cast<_int>(m_pSelectButtons.size()) - 1;

		// ���ο� ��ư ����
		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(true);

		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Hovered_Default_02");
	}

	
}

void CStargazer::Button_Interaction()
{
	if (m_pSelectButtons.empty())
		return;

	if (!m_bUseScript)
		return;

	// �� �̺�Ʈ �̸��� �´� ui�� ����
	// add object�� �����, target�� �־��༭ ����� �Ѵ�.

	if (m_iSelectButtonIndex < 0 || m_iSelectButtonIndex >= m_pSelectButtons.size())
		return;

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

		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(false);

		m_iSelectButtonIndex = -1;

		m_bUseScript = false;

		m_bUseOtherUI = true;

		// ��ư �Է� �Ҹ� �߰�

		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");


	}
	else if ("LevelUp" == m_ButtonEvents[m_iSelectButtonIndex])
	{
		// ������ ui �����
		CUI_Levelup::UI_LEVELUP_DESC eDesc = {};

		eDesc.pTarget = this;

		m_pGameInstance->Add_GameObject(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Levelup"), m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_UI_Levelup"), &eDesc);

		m_pScript->Active_Update(false);
		for (auto& pButton : m_pSelectButtons)
			if (nullptr != pButton)
				pButton->Active_Update(false);

		m_pSelectButtons[m_iSelectButtonIndex]->Set_isSelect(false);

		m_iSelectButtonIndex = -1;

		m_bUseScript = false;


		m_bUseOtherUI = true;

		// ��ư �Է� �Ҹ� �߰�
		CUI_Manager::Get_Instance()->Sound_Play("SE_UI_Btn_Selected_Default_03");

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

	m_iSelectButtonIndex = 0;

	m_bUseScript = true;
	m_bUseOtherUI = false;
	return S_OK;
}

HRESULT CStargazer::Ready_EffectSet()
{
	CGameObject* pInstance = nullptr;
	CStargazerEffect::DESC desc = {};
	desc.pOwner = this;
	desc.iLevelID = m_iLevelID;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(m_iLevelID, TEXT("Prototype_GameObject_StargazerEffect"), m_iLevelID, TEXT("Layer_EffectSet"), &pInstance,&desc)))
		return E_FAIL;
	m_pEffectSet = static_cast<CStargazerEffect*>(pInstance);
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

	PxActor* pActor = m_pPhysXActorCom->Get_Actor();
	if (!pActor->getScene()) // nullptr�̸� ���� ����
	{
		m_pGameInstance->Get_Scene()->addActor(*pActor);
	}


	_float fRadius = 1.2f;
	PxSphereGeometry geomTrigger = m_pGameInstance->CookSphereGeometry(fRadius);
	m_pPhysXTriggerCom->Create_Collision(m_pGameInstance->GetPhysics(), geomTrigger, pose, m_pGameInstance->GetMaterial(L"Default"));
	m_pPhysXTriggerCom->Set_ShapeFlag(false, true, true);

	filterData.word0 = WORLDFILTER::FILTER_INTERACT;
	filterData.word1 = WORLDFILTER::FILTER_PLAYERBODY;
	m_pPhysXTriggerCom->Set_SimulationFilterData(filterData);
	m_pPhysXTriggerCom->Set_QueryFilterData(filterData);
	m_pPhysXTriggerCom->Set_Owner(this);
	m_pPhysXTriggerCom->Set_ColliderType(COLLIDERTYPE::TRIGGER);

	PxActor* pActor1 = m_pPhysXTriggerCom->Get_Actor();
	if (!pActor1->getScene()) // nullptr�̸� ���� ����
	{
		m_pGameInstance->Get_Scene()->addActor(*pActor1);
	}

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

	/* For.Com_PhysX */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC),
		TEXT("Prototype_Component_PhysX_Static"), TEXT("Com_PhysX_Trigger"), reinterpret_cast<CComponent**>(&m_pPhysXTriggerCom))))
		return E_FAIL;

	/* For.Com_Sound */
	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_Sound_Stargazer"), TEXT("Com_Sound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
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
	Safe_Release(m_pPhysXTriggerCom);

	Safe_Release(m_pScript);
	for (auto& pButton : m_pSelectButtons)
		Safe_Release(pButton);

	Safe_Release(m_pSoundCom);
}
