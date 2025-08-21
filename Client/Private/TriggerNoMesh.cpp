#include "TriggerNoMesh.h"
#include "GameInstance.h"

CTriggerNoMesh::CTriggerNoMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CTriggerNoMesh::CTriggerNoMesh(const CTriggerNoMesh& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CTriggerNoMesh::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CTriggerNoMesh::Initialize(void* pArg)
{
	CTriggerNoMesh::TRIGGERNOMESH_DESC* TriggerNoMeshDESC = static_cast<TRIGGERNOMESH_DESC*>(pArg);


	if (FAILED(__super::Initialize(TriggerNoMeshDESC)))
		return E_FAIL;


	return S_OK;
}

void CTriggerNoMesh::Priority_Update(_float fTimeDelta)
{
	if (m_bDoOnce)
	{
		_bool bIsPlaying = false;
		bIsPlaying = m_pSoundCom->IsPlaying(m_vecSoundData[m_iSoundIndex].strSoundTag);

		if (!bIsPlaying)
		{
			m_iSoundIndex++;
			if (m_iSoundIndex >= m_vecSoundData.size())
			{
				m_bDead = true; 
				m_pPhysXTriggerCom->RemovePhysX();
			}
			else
				m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
		}
	}
}

void CTriggerNoMesh::Update(_float fTimeDelta)
{
}

void CTriggerNoMesh::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CTriggerNoMesh::Render()
{
	__super::Render();
	return S_OK;
}

void CTriggerNoMesh::On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
	if (!m_bDoOnce)
	{
		m_bDoOnce = true;
		m_iSoundIndex = 0;
		m_pSoundCom->Play(m_vecSoundData[m_iSoundIndex].strSoundTag);
	}
		
}

void CTriggerNoMesh::On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType)
{
}

void CTriggerNoMesh::Play_Sound()
{
}



CTriggerNoMesh* CTriggerNoMesh::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTriggerNoMesh* pGameInstance = new CTriggerNoMesh(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CTriggerNoMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CTriggerNoMesh::Clone(void* pArg)
{
	CTriggerNoMesh* pGameInstance = new CTriggerNoMesh(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CTriggerNoMesh");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CTriggerNoMesh::Free()
{
	__super::Free();
}
