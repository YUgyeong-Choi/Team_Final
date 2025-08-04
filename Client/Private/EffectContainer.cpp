#include "GameInstance.h"
#include "EffectContainer.h"
#include "ParticleEffect.h"
#include "SpriteEffect.h"
#include "MeshEffect.h"

CEffectContainer::CEffectContainer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CEffectContainer::CEffectContainer(const CEffectContainer& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CEffectContainer::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CEffectContainer::Initialize(void* pArg)
{

    return S_OK;
}

void CEffectContainer::Priority_Update(_float fTimeDelta)
{
	m_fCurFrame += m_fTickPerSecond * fTimeDelta;
	m_iCurFrame = static_cast<_int>(m_fCurFrame); // 캐스팅을 너무 많이 하는 것 같아서 그냥 별도로 저장
	m_fLifeTimeAcc += fTimeDelta;
	/*
	* 루프일 때는 전체 프레임 재생이 끝나면 0으로 다시 돌아감
	* 아닐 땐 계속 증가하시다가 언젠가 죽겠지.. 
	*/
	if (m_iCurFrame > m_iMaxFrame && true == m_bLoop)
	{
		m_fCurFrame = 0.f;
		m_iCurFrame = 0;
		for (auto& pEffect : m_Effects)
		{
			pEffect->Reset_TrackPosition();
		}
	}
	if (m_fLifeTimeAcc >= m_fLifeTime)
		m_bDead = true;

	// 가진 이펙트들을 업데이트
	for (auto& pEffect : m_Effects)
	{
		/*
		* 시퀀스와 최대한 비슷한 구조로 저장 
		* 컨테이너의 (전체적인) 프레임을 돌리고
		* 보유한 이펙트들의 재생 시간(시작~종료 프레임 내)에 컨테이너의 프레임이 존재할 경우 재생
		* 업데이트만 시켜주면 이펙트들은 알아서 내부적으로 업데이트 및 재생함
		*/
		if (m_iCurFrame >= pEffect->Get_StartTrackPosition() &&
			m_iCurFrame <= pEffect->Get_EndTrackPosition())
		{
			pEffect->Priority_Update(fTimeDelta);
		}
	}
}

void CEffectContainer::Update(_float fTimeDelta)
{
	// 가진 이펙트들을 업데이트
	for (auto& pEffect : m_Effects)
	{
		if (m_iCurFrame >= pEffect->Get_StartTrackPosition() &&
			m_iCurFrame <= pEffect->Get_EndTrackPosition())
		{
			pEffect->Update(fTimeDelta);
		}
	}
}

void CEffectContainer::Late_Update(_float fTimeDelta)
{
	// 가진 이펙트들을 업데이트
	for (auto& pEffect : m_Effects)
	{
		if (m_iCurFrame >= pEffect->Get_StartTrackPosition() &&
			m_iCurFrame <= pEffect->Get_EndTrackPosition())
		{
			pEffect->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CEffectContainer::Render()
{
#ifdef _DEBUG


#endif
    return S_OK;
}

HRESULT CEffectContainer::Load_JsonFiles(const _wstring strJsonFilePath)
{
	//json j;
	//ifstream ifs(strJsonFilePath);

	/************************************************/
	IGFD::FileDialogConfig config;
	config.path = R"(../Bin/Save/Effect/EffectContainer/)";

	json jLoad;
	IFILEDIALOG->OpenDialog("LoadEffectsetDialog", "Choose File to Load", ".json", config);

	if (IFILEDIALOG->Display("LoadEffectsetDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path loadPath = IFILEDIALOG->GetFilePathName();
			string filename = IFILEDIALOG->GetCurrentFileName();
			string prefix = filename.substr(0, 2);

			// put extension '.json'
			if (loadPath.extension().string() != ".json")
				loadPath += ".json";

			// Compare Prefix
			if (prefix != "EC")
			{
				MSG_BOX("Filename should start with EC (EffectContainer)");
				IFILEDIALOG->Close();
				return S_OK;
			}

			ifstream ifs(loadPath);
			if (!ifs.is_open())
			{
				MSG_BOX("File open Failed");
				IFILEDIALOG->Close();
				return E_FAIL;
			}

			ifs >> jLoad;
			ifs.close();

			if (!jLoad.contains("EffectObject") || !jLoad["EffectObject"].is_array())
			{
				IFILEDIALOG->Close();
				return E_FAIL;
			}

			for (const auto& jItem : jLoad["EffectObject"])
			{
				// 이름
				//if (jItem.contains("Name"))
				//	m_strSeqItemName = jItem["Name"].get<string>();

				// 타입
				EFFECT_TYPE eEffectType = {};
				if (jItem.contains("EffectType"))
					EFFECT_TYPE eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<int>());

				// Effect 객체 생성 및 역직렬화
				if (jItem.contains("EffectPreferences") && jItem["EffectPreferences"].is_array() && !jItem["EffectPreferences"].empty())
				{
					json jData;

					if (jItem["EffectPreferences"].is_array())
					{
						if (!jItem["EffectPreferences"].empty())
							jData = jItem["EffectPreferences"][0];
					}
					else if (jItem["EffectPreferences"].is_object())
					{
						jData = jItem["EffectPreferences"];
					}

					CEffectBase* pInstance = { nullptr };

					switch (eEffectType)
					{
					case Client::EFF_SPRITE:
					{
						CSpriteEffect::DESC desc = {};
						desc.bTool = false;
						pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
							PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ToolSprite"), &desc));
						if (pInstance != nullptr)
						{
							pInstance->Deserialize(jData);
							pInstance->Ready_Textures_Prototype_Tool();
						}
					}
					break;
					case Client::EFF_PARTICLE:
					{
						CParticleEffect::DESC desc = {};
						desc.fRotationPerSec = XMConvertToRadians(90.f);
						desc.fSpeedPerSec = 5.f;
						desc.bTool = false;
						pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
							PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"), &desc));
						if (pInstance != nullptr)
						{
							pInstance->Deserialize(jData);
							pInstance->Ready_Textures_Prototype_Tool();
							static_cast<CToolParticle*>(pInstance)->Change_InstanceBuffer(nullptr);
							m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
						}
					}
					break;
					case Client::EFF_MESH:
					{
						CMeshEffect::DESC desc = {};
						desc.fRotationPerSec = XMConvertToRadians(90.f);
						desc.fSpeedPerSec = 5.f;
						desc.bTool = false;
						pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
							PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"), &desc));
						if (pInstance != nullptr)
						{
							pInstance->Deserialize(jData);
							pInstance->Ready_Textures_Prototype_Tool();
							m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
						}
					}
					break;
					case Client::EFF_TRAIL:
					{
					}
					break;
					}

					if (pInstance == nullptr)
					{
						MSG_BOX("Failed to make Effect");
						m_bOpenLoadEffectContainer = false;
						IFILEDIALOG->Close();
						return E_FAIL;
					}
				}
				m_bOpenLoadEffectContainer = false;
				IFILEDIALOG->Close();
			}
		}
	}
	return S_OK;















	return S_OK;
}

HRESULT CEffectContainer::Ready_Components()
{

    return S_OK;
}

HRESULT CEffectContainer::Bind_ShaderResources()
{
    return S_OK;
}

CEffectContainer* CEffectContainer::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CEffectContainer* pInstance = new CEffectContainer(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CEffectContainer::Clone(void* pArg)
{
	CEffectContainer* pInstance = new CEffectContainer(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEffectContainer");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CEffectContainer::Free()
{
	__super::Free();
	for (auto& pEffect : m_Effects)
	{
		Safe_Release(pEffect);
	}
}
