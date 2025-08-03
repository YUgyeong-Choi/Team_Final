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
