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
	m_iCurFrame = static_cast<_int>(m_fCurFrame); // 캐스팅을 너무 많이 하는 것 같아서 그냥 별도로 저장
	m_fLifeTimeAcc += fTimeDelta;
	/*
	* 루프일 때는 전체 프레임 재생이 끝나면 0으로 다시 돌아감
	* 아닐 땐 계속 증가하시다가 언젠가 죽겠지.. 
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
			*  이부분 삭제 대신 각 이펙트들의 Loop상태를 해제한 후 시간이 지나면 죽도록 변경할 것 
			*/
			m_bDead = true;
			//for (auto& pEffect : m_Effects)
			//{
			//	// 근데 파티클만 해제해주면 되긴 함 
			//}
		}
	}

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
		// 이름
		//if (jItem.contains("Name"))
		//	m_strSeqItemName = jItem["Name"].get<string>();

		// 타입
		EFFECT_TYPE eEffectType = {};
		if (jItem.contains("EffectType"))
			eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<_int>());

		// Effect 객체 생성 및 역직렬화
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

			// 소드트레일만??? 트레일은 개별적으로 처리할 듯? 
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
