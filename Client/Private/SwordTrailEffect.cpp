#include "SwordTrailEffect.h"
#include "ParticleEffect.h"
#include "GameInstance.h"
#include "Effect_Manager.h"
#include "EffectContainer.h"
#include "Client_Calculation.h"

CSwordTrailEffect::CSwordTrailEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CEffectBase{ pDevice, pContext }
{

}

CSwordTrailEffect::CSwordTrailEffect(const CSwordTrailEffect& Prototype)
	: CEffectBase( Prototype )
{

}

HRESULT CSwordTrailEffect::Initialize_Prototype()
{
	//m_KeyFrames.push_back(EFFKEYFRAME{});
	return S_OK;
}

HRESULT CSwordTrailEffect::Initialize(void* pArg)
{
	DESC* pDesc = static_cast<DESC*>(pArg);

	if (!pDesc ||
		!pDesc->pParentCombinedMatrix ||
		!pDesc->pInnerSocketMatrix ||
		!pDesc->pOuterSocketMatrix)
	{
		MSG_BOX("트레일은 무조건 뼈 소켓 매트릭스를 받아와야함.");
		return E_FAIL;
	}

	m_pParentCombinedMatrix = pDesc->pParentCombinedMatrix;
	m_pInnerSocketMatrix = pDesc->pInnerSocketMatrix;
	m_pOuterSocketMatrix = pDesc->pOuterSocketMatrix;
	m_strEmitterTag = pDesc->strEmitterTag;
	m_bHasEmitter = pDesc->bHasEmitter;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// 부모 모델, 부모 모델의 뼈 소켓매트릭스
	m_eEffectType = EFF_TRAIL;

	return S_OK;
}

void CSwordTrailEffect::Priority_Update(_float fTimeDelta)
{

}

void CSwordTrailEffect::Update(_float fTimeDelta)
{
	Update_Keyframes();

	//if (m_pSocketMatrix != nullptr)
	//{
	//	XMStoreFloat4x4(&m_CombinedWorldMatrix,
	//		XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()) * XMLoadFloat4x4(m_pSocketMatrix)
	//	);
	//}

	if (m_bAnimation)
		m_iTileIdx = static_cast<_int>(m_fCurrentTrackPosition);
	else
		m_iTileIdx = 0;

	if (m_iTileX == 0)
		m_iTileX = 1;
	if (m_iTileY == 0)
		m_iTileY = 1;
	m_fTileSize.x = 1.0f / _float(m_iTileX);
	m_fTileSize.y = 1.0f / _float(m_iTileY);
	m_fOffset.x = (m_iTileIdx % m_iTileX) * m_fTileSize.x;
	m_fOffset.y = (m_iTileIdx / m_iTileX) * m_fTileSize.y;
}



void CSwordTrailEffect::Late_Update(_float fTimeDelta)
{
	if (!m_pParentCombinedMatrix || !m_pInnerSocketMatrix || !m_pOuterSocketMatrix)
		return;

	_matrix matCombined = XMLoadFloat4x4(m_pParentCombinedMatrix);
	_matrix matInner = XMLoadFloat4x4(m_pInnerSocketMatrix);
	_matrix matOuter = XMLoadFloat4x4(m_pOuterSocketMatrix);

	_matrix matInnerWorld = XMMatrixMultiply(matInner, matCombined);
	_matrix matOuterWorld = XMMatrixMultiply(matOuter, matCombined);

	// 월드 좌표 추출
	XMStoreFloat3(&m_vInnerPos, XMVector3TransformCoord(XMVectorZero(), matInnerWorld));
	XMStoreFloat3(&m_vOuterPos, XMVector3TransformCoord(XMVectorZero(), matOuterWorld));

	m_pVIBufferCom->Update_Trail(m_vInnerPos, m_vOuterPos, fTimeDelta);
	if (m_bTrailActive)
		m_vPrevOuterPos = m_vCurOuterPos;
	m_vCurOuterPos = m_vOuterPos;

	if (m_bHasEmitter && m_bTrailActive)
	{
		_vector vDir = XMVector3Normalize(XMLoadFloat3(&m_vPrevOuterPos) - XMLoadFloat3(&m_vCurOuterPos));
		if (XMVectorGetX(XMVector3Length(vDir)) < 0.00001f)
			vDir = XMVectorSet(0.f, 0.f, 1.f, 0.f);
		const vector<_float3>& vNodes =  m_pVIBufferCom->Get_InterpolatedNewNodes();
		if (!vNodes.empty())
		{
			for (auto& vPos : vNodes)
			{
				CParticleEffect::DESC desc = {};
				desc.iLevelID = m_iLevelID;
				desc.bHasPresetMat = true;
				XMStoreFloat4x4(&desc.PresetMatrix,
					XMMatrixRotationQuaternion(XMQuaternionRotationVectorToVector(_vector{0.f,0.f,1.f,0.f}, vDir)) *
					XMMatrixTranslation(vPos.x, vPos.y, vPos.z) /** XMMatrixRotationAxis() 이거대체어케넣지*/
				);
				
				CParticleEffect* pEmitter = static_cast<CParticleEffect*>(MAKE_SINGLEEFFECT(ENUM_CLASS(m_iLevelID), m_strEmitterTag, TEXT("Layer_Effect"), 0.f, 0.f, 0.f, &desc));
				if (pEmitter == nullptr)
					MSG_BOX("emitter failed");
			}
		}
	}   

	m_pGameInstance->Add_RenderGroup((RENDERGROUP)m_iRenderGroup, this);
}

HRESULT CSwordTrailEffect::Render()
{
	if (FAILED(Bind_ShaderResources()))	
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(m_iShaderPass)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CSwordTrailEffect::Set_TrailActive(_bool bActive)
{
	if (m_pVIBufferCom)
		m_pVIBufferCom->Set_TrailActive(bActive);
	m_bTrailActive = bActive;
}

HRESULT CSwordTrailEffect::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_TrailEffect"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	_wstring strPrototypeTag = TEXT("Prototype_Component_VIBuffer_");
	strPrototypeTag += m_strBufferTag;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), strPrototypeTag,
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	_wstring TextureTag = TEXT("Prototype_Component_Texture_");
	if (m_bTextureUsage[TU_DIFFUSE] == true){
		/* For.Com_Texture */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_DIFFUSE], 
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_DIFFUSE]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true){
		/* For.Com_TextureMask1 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK1],
			TEXT("Com_TextureMask1"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK1]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true){
		/* For.Com_TextureMask2 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK2],
			TEXT("Com_TextureMask2"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK2]))))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true){
		/* For.Com_TextureMask3 */
		if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TextureTag + m_TextureTag[TU_MASK3],
			TEXT("Com_TextureMask3"), reinterpret_cast<CComponent**>(&m_pTextureCom[TU_MASK3]))))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CSwordTrailEffect::Bind_ShaderResources()
{

	if (FAILED(__super::Bind_ShaderResources()))
		return E_FAIL;

	return S_OK;
}

CSwordTrailEffect* CSwordTrailEffect::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSwordTrailEffect* pInstance = new CSwordTrailEffect(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMainApp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CSwordTrailEffect::Clone(void* pArg)
{
	CSwordTrailEffect* pInstance = new CSwordTrailEffect(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CSwordTrailEffect");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CSwordTrailEffect::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
}

json CSwordTrailEffect::Serialize()
{
	json j = __super::Serialize();




	return j;
}

void CSwordTrailEffect::Deserialize(const json& j)
{
	__super::Deserialize(j);

	if (j.contains("Name"))
	{
		m_strBufferTag = StringToWString(j["Name"].get<std::string>());
	}
}
