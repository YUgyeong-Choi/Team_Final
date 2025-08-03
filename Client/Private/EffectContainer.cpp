#include "EffectContainer.h"
#include "EffectBase.h"

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
