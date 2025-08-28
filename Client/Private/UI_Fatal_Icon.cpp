#include "UI_Fatal_Icon.h"
#include "GameInstance.h"

CUI_Fatal_Icon::CUI_Fatal_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{pDevice, pContext}
{
}

CUI_Fatal_Icon::CUI_Fatal_Icon(const CUI_Fatal_Icon& Prototype)
	:CUIObject{Prototype}
{
}

HRESULT CUI_Fatal_Icon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Fatal_Icon::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fOffset = 0.005f;

	m_vColor = { 1.f, 0.27f, 0.f, 1.f };

	m_fCurrentAlpha = 1.f;

	return S_OK;
}

void CUI_Fatal_Icon::Priority_Update(_float fTimeDelta)
{
	if (nullptr == m_pPlayer)
		Find_Player();
}

void CUI_Fatal_Icon::Update(_float fTimeDelta)
{
	if (nullptr == m_pPlayer)
		return;

	// 보스 페이탈 로직 보고 다시 바꾸기, 그로기면 이제 빨간색으로 띄운게 필요할듯?

	if (nullptr == m_pPlayer->GetFatalTarget() || !m_pPlayer->GetbIsBackAttack() || !m_pPlayer->GetFatalTarget()->Get_isActive())
	{
		m_isRender = false;
		return;
	}
	else
	{
		m_isRender = true;

		// 위치 가져와서 직교로 그리자

		// 회전 값 다 빼고, z 위치 0.01로, 위에 페이탈 이미지 덧그리도록?

		_vector vWorldPos = XMLoadFloat4(&m_pPlayer->GetFatalTarget()->Get_LockonPos());



		_matrix ViewMat = m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW);
		_matrix ProjMat = m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ);

		_vector vClipPos = XMVector4Transform(vWorldPos, ViewMat * ProjMat);

		vClipPos.m128_f32[0] /= vClipPos.m128_f32[3];
		vClipPos.m128_f32[1] /= vClipPos.m128_f32[3];
		vClipPos.m128_f32[2] /= vClipPos.m128_f32[3];

		_float fX = (vClipPos.m128_f32[0] * 0.5f + 0.5f) * g_iWinSizeX;
		_float fY = (1.f - (vClipPos.m128_f32[1] * 0.5f + 0.5f)) * g_iWinSizeY;

		_vector vPos = { fX - 0.5f * g_iWinSizeX, -fY + 0.5f * g_iWinSizeY,0.01f,1.f };

		m_pTransformCom->Set_State(STATE::POSITION, vPos);

	}


}

void CUI_Fatal_Icon::Late_Update(_float fTimeDelta)
{
	if (nullptr == m_pPlayer)
		return;

	if (m_isRender)
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
	
}

HRESULT CUI_Fatal_Icon::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_Texture", 0)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Begin(D_UI_SOFTEFFECT)))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Bind_Buffers()))
		return E_FAIL;

	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;

	return S_OK;
}

void CUI_Fatal_Icon::Find_Player()
{
	CGameObject* pObj = m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player"));

	if (nullptr != pObj)
		m_pPlayer = static_cast<CPlayer*>(pObj);
}

HRESULT CUI_Fatal_Icon::Ready_Components()
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;


	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_Fatal"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;



	return S_OK;
}

HRESULT CUI_Fatal_Icon::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;
	
	// 이제 페이탈이 가능하면 이 색깔로, 그로기면 빨간색으로? 

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
		return E_FAIL;

	return S_OK;
}

CUI_Fatal_Icon* CUI_Fatal_Icon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Fatal_Icon* pInstance = new CUI_Fatal_Icon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_LockOn_Icon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_Fatal_Icon::Clone(void* pArg)
{
	CUI_Fatal_Icon* pInstance = new CUI_Fatal_Icon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_Fatal_Icon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_Fatal_Icon::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}