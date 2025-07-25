#include "EffectBase.h"

#include "GameInstance.h"

CEffectBase::CEffectBase(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CBlendObject{ pDevice, pContext }
{

}

CEffectBase::CEffectBase(const CEffectBase& Prototype)
	: CBlendObject( Prototype )
	, m_KeyFrames(Prototype.m_KeyFrames)
{

}

HRESULT CEffectBase::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CEffectBase::Initialize(void* pArg)
{
	if (pArg == nullptr)
	{
		MSG_BOX("이펙트는 DESC가 꼭 필요해용");
		return E_FAIL;
	}

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	return S_OK;
}

void CEffectBase::Priority_Update(_float fTimeDelta)
{

}

void CEffectBase::Update(_float fTimeDelta)
{
}

void CEffectBase::Late_Update(_float fTimeDelta)
{
}

HRESULT CEffectBase::Render()
{
	return S_OK;
}

void CEffectBase::Set_KeyFrames(KEYFRAME tNewKeyframe)
{
	m_KeyFrames.push_back(tNewKeyframe);
}

void CEffectBase::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
}
