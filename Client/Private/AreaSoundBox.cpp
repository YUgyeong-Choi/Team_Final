#include "AreaSoundBox.h"
#include "GameInstance.h"

CAreaSoundBox::CAreaSoundBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CTriggerBox{ pDevice, pContext }
{

}

CAreaSoundBox::CAreaSoundBox(const CAreaSoundBox& Prototype)
	: CTriggerBox(Prototype)
{

}

HRESULT CAreaSoundBox::Initialize_Prototype()
{
	/* 외부 데이터베이스를 통해서 값을 채운다. */

	return S_OK;
}

HRESULT CAreaSoundBox::Initialize(void* pArg)
{
	CAreaSoundBox::AREASOUNDBOX_DESC* AreaSoundBoxDESC = static_cast<AREASOUNDBOX_DESC*>(pArg);
	m_eTriggerSoundType = AreaSoundBoxDESC->eTriggerBoxType;
	m_szSoundID = AreaSoundBoxDESC->szSoundID;
	m_strSoundName = AreaSoundBoxDESC->strSoundName;

	if (FAILED(CGameObject::Initialize(AreaSoundBoxDESC)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	_float3 vPosition = { AreaSoundBoxDESC->vPosition.x, AreaSoundBoxDESC->vPosition.y, AreaSoundBoxDESC->vPosition.z };
	m_pTransformCom->Set_State(STATE::POSITION,	XMVectorSet(vPosition.x, vPosition.y, vPosition.z,1.f));

	m_pSoundCom->Set3DState(AreaSoundBoxDESC->fMinMax.x, AreaSoundBoxDESC->fMinMax.y);
	m_pSoundCom->Update3DPosition(vPosition);
	m_pSoundCom->SetVolume(AreaSoundBoxDESC->strSoundName, AreaSoundBoxDESC->fVolume);
	m_pSoundCom->Play(m_strSoundName);


	return S_OK;
}

void CAreaSoundBox::Priority_Update(_float fTimeDelta)
{
}

void CAreaSoundBox::Update(_float fTimeDelta)
{
	if (!m_pSoundCom->IsPlaying(m_strSoundName))
		m_pSoundCom->Play(m_strSoundName);
}

void CAreaSoundBox::Late_Update(_float fTimeDelta)
{
	__super::Late_Update(fTimeDelta);
}

HRESULT CAreaSoundBox::Render()
{
	__super::Render();
	return S_OK;
}

void CAreaSoundBox::SoundPlay(const string szSoundID, _float fVolume)
{
	m_pSoundCom->SetVolume(szSoundID, fVolume);
	m_pSoundCom->Play(szSoundID);
}

void CAreaSoundBox::SoundVolume(const string szSoundID, _float fVolume)
{
	m_pSoundCom->SetVolume(szSoundID, fVolume);
}

void CAreaSoundBox::SoundPosition(_float3 vPosition, _float fMin, _float fMax)
{
	m_pSoundCom->Set3DState(fMin, fMax);
	m_pSoundCom->Update3DPosition(vPosition);
}

void CAreaSoundBox::SoundStop()
{
	m_pSoundCom->StopAll();
}

HRESULT CAreaSoundBox::Ready_Components()
{
	/* For.Com_Sound */
	if (FAILED(Add_Component(static_cast<int>(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Sound_")) + m_szSoundID,
		TEXT("Com_AreaSound"), reinterpret_cast<CComponent**>(&m_pSoundCom))))
		return E_FAIL;

	return S_OK;
}


CAreaSoundBox* CAreaSoundBox::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAreaSoundBox* pGameInstance = new CAreaSoundBox(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CAreaSoundBox");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}


CGameObject* CAreaSoundBox::Clone(void* pArg)
{
	CAreaSoundBox* pGameInstance = new CAreaSoundBox(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CAreaSoundBox");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CAreaSoundBox::Free()
{
	__super::Free();

	Safe_Release(m_pSoundCom);
}
