#include "Ramp.h"
#include "GameInstance.h"

CRamp::CRamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CItem{pDevice, pContext}
{
}

CRamp::CRamp(const CRamp& Prototype)
    :CItem{Prototype}
{
}

HRESULT CRamp::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CRamp::Initialize(void* pArg)
{
    if(FAILED(__super::Initialize(pArg)))
        return E_FAIL;

	m_strProtoTag = TEXT("Prototype_GameObject_Ramp");
	

	m_isRender = true;
  
    Ready_Components();



    return S_OK;
}

void CRamp::Priority_Update(_float fTimeDelta)
{
}

void CRamp::Update(_float fTimeDelta)
{
    // 
}

void CRamp::Late_Update(_float fTimeDelta)
{
    // �̰� �ʿ��Ѱɷ� �߰��ϸ� �ɵ�?
    //m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CRamp::Render()
{
    return S_OK;
}

void CRamp::Activate()
{

	m_isLight = !m_isLight;


    // ramp�� �ִ� light�� ���� �ִ� �ϵ��� ���� �߰�

}

HRESULT CRamp::Ready_Components()
{
    // ��Ʈ�� �־�� �Ǵϱ� ramp model�̶� ���̴� �ʿ��ҵ�?

    return S_OK;
}

CRamp* CRamp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CRamp* pInstance = new CRamp(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CRamp::Clone(void* pArg)
{
	CRamp* pInstance = new CRamp(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CRamp::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
	Safe_Release(m_pShaderCom);

}
