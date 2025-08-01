#include "UI_Guide.h"
#include "Static_UI.h"
#include "GameInstance.h"

CUI_Guide::CUI_Guide(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_Guide::CUI_Guide(const CUI_Guide& Prototype)
    :CUI_Container{Prototype}
{
}

json CUI_Guide::Serialize()
{
    json j = __super::Serialize();
    
    return j;
}

void CUI_Guide::Deserialize(const json& j)
{
    __super::Deserialize(j);
}

HRESULT CUI_Guide::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Guide::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    UI_GUIDE_DESC* pDesc = static_cast<UI_GUIDE_DESC*>(pArg);

    

    for (auto& partPath : pDesc->partPaths)
    {
        CUI_Container::UI_CONTAINER_DESC eDesc = {};

        eDesc.strFilePath = partPath;

        m_Explainations.push_back(static_cast<CUI_Container*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_UI_Container"), &eDesc)));

        m_Explainations.back()->Active_Update(false);
    }

 

    CUI_Container::UI_CONTAINER_DESC eDesc = {};

    eDesc.strFilePath = TEXT("../Bin/Save/UI/Guide/Guide_Button.json");

    



    

   



    


    return S_OK;
}

void CUI_Guide::Priority_Update(_float fTimeDelta)
{
}

void CUI_Guide::Update(_float fTimeDelta)
{
    // Å° ÀÔ·Â

    Check_Button();
}

void CUI_Guide::Late_Update(_float fTimeDelta)
{

    
    m_pBackGround->Late_Update(fTimeDelta);
 

    for (auto& pPart : m_Explainations)
    {
        pPart->Late_Update(fTimeDelta);
    }
}

HRESULT CUI_Guide::Render()
{
    return S_OK;
}

void CUI_Guide::Check_Button()
{
    if (m_pGameInstance->Key_Down(DIK_SPACE))
    {

    }


    if (m_pGameInstance->Key_Down(DIK_A))
    {

    }
    else if (m_pGameInstance->Key_Down(DIK_D))
    {

    }

   
    m_Explainations[m_iIndex]->Active_Update(true);

    

}

CUI_Guide* CUI_Guide::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Guide* pInstance = new CUI_Guide(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Guide");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Guide::Clone(void* pArg)
{
    CUI_Guide* pInstance = new CUI_Guide(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Guide");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Guide::Free()
{
    __super::Free();

    Safe_Release(m_pBackGround);
    //Safe_Release(m_pButtons);

    for (auto& pPart : m_Explainations)
        Safe_Release(pPart);
}
