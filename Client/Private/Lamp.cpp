#include "Lamp.h"
#include "GameInstance.h"

CLamp::CLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CItem{pDevice, pContext}
{
}

CLamp::CLamp(const CLamp& Prototype)
    :CItem{Prototype}
{
}

HRESULT CLamp::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CLamp::Initialize(void* pArg)
{
    if(FAILED(__super::Initialize(pArg)))
        return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_Lamp");
	
	m_eTargetLevel = LEVEL::KRAT_CENTERAL_STATION;

	m_isRender = true;

    if(FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_Light()))
		return E_FAIL;

	m_pTransformCom->SetUp_Scale(0.35f, 0.35f, 0.35f);

	

	m_bDebug = false;

	SetbVolumetric(true);
	SetRange(3.f);
	SetColor(_float4(1.f, 0.7f, 0.4f, 1.f));

	SetIsPlayerFar(true);
	SetIntensity(1.5f);

	m_isLight = false;

    return S_OK;
}

void CLamp::Priority_Update(_float fTimeDelta)
{

	if (m_bDead)
		m_pGameInstance->Remove_Light(ENUM_CLASS(LEVEL::DH), m_pLight);

	/* [ 플레이어 찾기 ] */

	if (nullptr == m_pParentWorldMatrix)
	{
		auto pPlayer = m_pGameInstance->Get_LastObject(ENUM_CLASS(LEVEL::KRAT_CENTERAL_STATION), TEXT("Layer_Player"));

		m_pParentWorldMatrix = pPlayer->Get_TransfomCom()->Get_WorldMatrix_Ptr();
	}
	
}

void CLamp::Update(_float fTimeDelta)
{
    // 
}

void CLamp::Late_Update(_float fTimeDelta)
{
 
	if (m_isLight)
	{
		if (nullptr == m_pParentWorldMatrix)
			return;

		_matrix PlayerMat = XMLoadFloat4x4(m_pParentWorldMatrix);
		_vector vPosition = PlayerMat.r[3];
		vPosition = XMVectorSetY(vPosition, XMVectorGetY(vPosition) + 1.f);
		PlayerMat.r[3] = vPosition;
		m_pTransformCom->Set_WorldMatrix(PlayerMat);
	
		_vector vLightPos = m_pTransformCom->Get_State(STATE::POSITION);
		XMStoreFloat4(&m_pLight->Get_LightDesc()->vPosition, vLightPos);

		_vector vLook = m_pTransformCom->Get_State(STATE::LOOK);
		XMStoreFloat4(&m_pLight->Get_LightDesc()->vDirection, vLook);
	}
	

	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CLamp::Render()
{
	if (!m_bDebug)
		return S_OK;

	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	/* [ 픽킹 아이디 넘기기 ] */
	_float fID = static_cast<_float>(m_iID);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_fID", &fID, sizeof(_float))))
		return E_FAIL;

	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pShaderCom->Begin(1);

		m_pModelCom->Render(i);
	}

	return S_OK;
}

void CLamp::Activate()
{
	
	m_isLight = !m_isLight;

	if (m_isLight)
	{
		
		m_pLight->Get_LightDesc()->bIsUse = true;
	}
	else
	{
		
		m_pLight->Get_LightDesc()->bIsUse = false;
	}


    // Lamp에 있는 light도 껏다 켯다 하도록 로직 추가

}

ITEM_DESC CLamp::Get_ItemDesc()
{
	ITEM_DESC eDesc = {};

	if (m_isLight)
		eDesc.iItemIndex = 1;
	else
		eDesc.iItemIndex = 0;
	eDesc.strPrototag = m_strProtoTag;
	eDesc.isUsable = true;
	eDesc.isConsumable = false;


	return eDesc;
}

HRESULT CLamp::Ready_Components()
{

	/* Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* Com_Model */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Model_PointLight")),
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	return S_OK;

}

HRESULT CLamp::Ready_Light()
{
	LIGHT_DESC			LightDesc{};

	
	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = _float4(10.f, 5.0f, 10.f, 1.f);

	LightDesc.fAmbient = 0.2f;
	LightDesc.fIntensity = 1.f;
	LightDesc.fRange = 10.f;
	LightDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	LightDesc.fFogDensity = 0.f;
	LightDesc.fFogCutoff = 15.f;
	LightDesc.bIsVolumetric = true;
	LightDesc.bIsPlayerFar = false;
	

	if (FAILED(m_pGameInstance->Add_LevelLightDataReturn(ENUM_CLASS(m_eTargetLevel), LightDesc, &m_pLight)))
		return E_FAIL;

	return S_OK;
}

HRESULT CLamp::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}

CLamp* CLamp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CLamp* pInstance = new CLamp(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CLamp::Clone(void* pArg)
{
	CLamp* pInstance = new CLamp(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CLamp::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
