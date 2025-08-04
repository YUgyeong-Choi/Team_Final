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
	m_iCurFrame = static_cast<_int>(m_fCurFrame); // ĳ������ �ʹ� ���� �ϴ� �� ���Ƽ� �׳� ������ ����
	m_fLifeTimeAcc += fTimeDelta;
	/*
	* ������ ���� ��ü ������ ����� ������ 0���� �ٽ� ���ư�
	* �ƴ� �� ��� �����Ͻôٰ� ������ �װ���.. 
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

	// ���� ����Ʈ���� ������Ʈ
	for (auto& pEffect : m_Effects)
	{
		/*
		* �������� �ִ��� ����� ������ ���� 
		* �����̳��� (��ü����) �������� ������
		* ������ ����Ʈ���� ��� �ð�(����~���� ������ ��)�� �����̳��� �������� ������ ��� ���
		* ������Ʈ�� �����ָ� ����Ʈ���� �˾Ƽ� ���������� ������Ʈ �� �����
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
	// ���� ����Ʈ���� ������Ʈ
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
	// ���� ����Ʈ���� ������Ʈ
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
				// �̸�
				//if (jItem.contains("Name"))
				//	m_strSeqItemName = jItem["Name"].get<string>();

				// Ÿ��
				EFFECT_TYPE eEffectType = {};
				if (jItem.contains("EffectType"))
					EFFECT_TYPE eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<int>());

				// Effect ��ü ���� �� ������ȭ
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
