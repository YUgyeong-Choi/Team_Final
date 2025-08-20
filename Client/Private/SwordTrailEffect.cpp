#include "SwordTrailEffect.h"

#include "GameInstance.h"

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

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// 부모 모델, 부모 모델의 뼈 소켓매트릭스
		
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
	// 월드를 바인딩 해 줄 필요가 없는지 고민하는데 왜 니가 아니라고 단정지어
	//if (m_pSocketMatrix != nullptr)
	//{
	//	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
	//		return E_FAIL;
	//}
	//else
	//{
	//	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
	//		return E_FAIL;
	//}
	/* dx9 : 장치에 뷰, 투영행렬을 저장해두면 렌더링시 알아서 정점에 Transform해주었다. */
	/* dx11 : 셰이더에 뷰, 투영행렬을 저장해두고 우리가 직접 변환해주어야한다. */

	if (FAILED(m_pShaderCom->Bind_RawValue("g_bFlipUV", &m_bFlipUV, sizeof(_bool))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileSize", &m_fTileSize, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTileOffset", &m_fOffset, sizeof(_float2))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vColor", &m_vColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCenterColor", &m_vCenterColor, sizeof(_float4))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fIntensity", &m_fIntensity, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fThreshold", &m_fThreshold, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fTime", &m_fTimeAcc, sizeof(_float))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissiveIntensity, sizeof(_float))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	if (m_bTextureUsage[TU_DIFFUSE] == true) {
		if (FAILED(m_pTextureCom[TU_DIFFUSE]->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK1] == true) {
		if (FAILED(m_pTextureCom[TU_MASK1]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture1", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK2] == true) {
		if (FAILED(m_pTextureCom[TU_MASK2]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture2", 0)))
			return E_FAIL;
	}
	if (m_bTextureUsage[TU_MASK3] == true) {
		if (FAILED(m_pTextureCom[TU_MASK3]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture3", 0)))
			return E_FAIL;
	}

	if (FAILED(m_pGameInstance->Bind_RT_ShaderResource(TEXT("Target_Depth"), m_pShaderCom, "g_DepthTexture")))
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
