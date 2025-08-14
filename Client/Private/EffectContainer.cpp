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
	if (pArg == nullptr)
	{
		MSG_BOX("EffectContainer must have Arguments");
		return E_FAIL;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	DESC* pDesc = static_cast<DESC*>(pArg);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&pDesc->vPresetPosition), 1.f));
	Load_JsonFiles(pDesc->j);
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
	if (m_iCurFrame > m_iMaxFrame)
	{
		if (true == m_bLoop)
		{
			m_fCurFrame = 0.f;
			m_iCurFrame = 0;
			for (auto& pEffect : m_Effects)
			{
				pEffect->Reset_TrackPosition();
			}
		}
		else
		{
			/*
			*  �̺κ� ���� ��� �� ����Ʈ���� Loop���¸� ������ �� �ð��� ������ �׵��� ������ �� 
			*/
			m_bDead = true;
			//for (auto& pEffect : m_Effects)
			//{
			//	// �ٵ� ��ƼŬ�� �������ָ� �Ǳ� �� 
			//}
		}
	}

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

    return S_OK;
}

HRESULT CEffectContainer::Load_JsonFiles(const json& j)
{
	if (!j.contains("EffectObject") || !j["EffectObject"].is_array())
	{
		return E_FAIL;
	}

	if (j.contains("MaxFrame"))
		m_iMaxFrame = j["MaxFrame"].get<_int>();
	else
		m_iMaxFrame = 200;
	if (j.contains("Loop"))
		m_bLoop = j["Loop"].get<_bool>();
	else
		m_bLoop = true;

	for (const auto& jItem : j["EffectObject"])
	{
		// �̸�
		//if (jItem.contains("Name"))
		//	m_strSeqItemName = jItem["Name"].get<string>();

		// Ÿ��
		EFFECT_TYPE eEffectType = {};
		if (jItem.contains("EffectType"))
			eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<_int>());

		// Effect ��ü ���� �� ������ȭ
		CEffectBase* pInstance = { nullptr };

		switch (eEffectType)
		{
		case Client::EFF_SPRITE:
		{
			CSpriteEffect::DESC desc = {};
			desc.fRotationPerSec = XMConvertToRadians(90.f);
			desc.fSpeedPerSec = 5.f;
			desc.bTool = false;
			desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SpriteEffect"), &desc));
		}
		break;
		case Client::EFF_PARTICLE:
		{
			CParticleEffect::DESC desc = {};
			desc.fRotationPerSec = XMConvertToRadians(90.f);
			desc.fSpeedPerSec = 5.f;
			desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			desc.bTool = false;
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ParticleEffect"), &desc));
		}
		break;
		case Client::EFF_MESH:
		{
			CMeshEffect::DESC desc = {};
			desc.fRotationPerSec = XMConvertToRadians(90.f);
			desc.fSpeedPerSec = 5.f;
			desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			desc.bTool = false;
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MeshEffect"), &desc));
		}
		break;
		case Client::EFF_TRAIL:
		{

			// �ҵ�Ʈ���ϸ�??? Ʈ������ ���������� ó���� ��? 
			//CTrailEffect::DESC desc = {};
			//desc.fRotationPerSec = XMConvertToRadians(90.f);
			//desc.fSpeedPerSec = 5.f;
			//desc.pSocketMatrix = m_pTransformCom->Get_WorldMatrix_Ptr();
			//desc.bTool = false;
			//pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
			//	PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_TrailEffect"), &desc));
		}
		break;
		}


		if (pInstance != nullptr)
		{
			if (FAILED(pInstance->Ready_Effect_Deserialize(jItem)))
				return E_FAIL;
			m_Effects.push_back(pInstance);
		}
		else
			return E_FAIL;
	}
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
