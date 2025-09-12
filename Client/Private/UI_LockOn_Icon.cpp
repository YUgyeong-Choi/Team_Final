#include "UI_LockOn_Icon.h"
#include "GameInstance.h"
#include "LockOn_Manager.h"
#include "Unit.h"

CUI_LockOn_Icon::CUI_LockOn_Icon(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CUIObject{ pDevice, pContext }
{
}

CUI_LockOn_Icon::CUI_LockOn_Icon(const CUI_LockOn_Icon& Prototype)
	:CUIObject{ Prototype }
{
}

HRESULT CUI_LockOn_Icon::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_LockOn_Icon::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	m_fOffset = 0.2f;

	m_fInitSizeX = m_fSizeX * 2.f;
	m_fInitSizeY = m_fSizeY * 2.f;

	return S_OK;
}

void CUI_LockOn_Icon::Priority_Update(_float fTimeDelta)
{
}

void CUI_LockOn_Icon::Update(_float fTimeDelta)
{
	auto pTarget = CLockOn_Manager::Get_Instance()->Get_Target();

  	if (nullptr == pTarget || pTarget->Get_bDead())
	{
		m_isRender = false;

		
		m_fRenderTime = 0.f;
		return;
	}
	else
	{
		m_isRender = true;

		if (m_fRenderTime >= 1.f)
		{
			m_pTransformCom->Scaling(m_fSizeX, m_fSizeY);
		}
		else
		{
			_float fSizeX = LERP(m_fInitSizeX, m_fSizeX, m_fRenderTime);
			_float fSizeY = LERP(m_fInitSizeY, m_fSizeY, m_fRenderTime);

			m_pTransformCom->Scaling(fSizeX, fSizeY);

			m_fRenderTime += fTimeDelta * 2.f;
		}

		// 위치 가져와서 직교로 그리자

		// 회전 값 다 빼고, z 위치 0.01로, 위에 페이탈 이미지 덧그리도록?

		_vector vWorldPos = XMLoadFloat4(&static_cast<CUnit*>(pTarget)->Get_LockonPos());



		_matrix ViewMat = m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW);
		_matrix ProjMat = m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ);

		_vector vClipPos = XMVector4Transform(vWorldPos, ViewMat * ProjMat);

		vClipPos.m128_f32[0] /= vClipPos.m128_f32[3];
		vClipPos.m128_f32[1] /= vClipPos.m128_f32[3];
		vClipPos.m128_f32[2] /= vClipPos.m128_f32[3];

		_float fX = (vClipPos.m128_f32[0] * 0.5f + 0.5f) * g_iWinSizeX;
		_float fY = (1.f - (vClipPos.m128_f32[1] * 0.5f + 0.5f)) * g_iWinSizeY;

		_vector vPos = { fX - 0.5f * g_iWinSizeX, -fY + 0.5f * g_iWinSizeY, 0.2f,1.f};
	
		m_pTransformCom->Set_State(STATE::POSITION, vPos);
	}

}

void CUI_LockOn_Icon::Late_Update(_float fTimeDelta)
{

	if (m_isRender)
		m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
}

HRESULT CUI_LockOn_Icon::Render()
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

HRESULT CUI_LockOn_Icon::Ready_Components()
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_DynamicUI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	/* For.Com_VIBuffer */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;


	/* For.Com_Texture */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Texture_LockOn"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	

	return S_OK;
}

HRESULT CUI_LockOn_Icon::Bind_ShaderResources()
{


	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	_float4 vColor = { 1.f,1.f,1.f,1.f };

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &vColor, sizeof(_float4))))
		return E_FAIL;

	_float fAlpha = 1.f;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &fAlpha, sizeof(_float))))
		return E_FAIL;



	return S_OK;
}

CUI_LockOn_Icon* CUI_LockOn_Icon::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_LockOn_Icon* pInstance = new CUI_LockOn_Icon(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_LockOn_Icon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CUI_LockOn_Icon::Clone(void* pArg)
{
	CUI_LockOn_Icon* pInstance = new CUI_LockOn_Icon(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_LockOn_Icon");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CUI_LockOn_Icon::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVIBufferCom);

}
