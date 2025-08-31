#include "UI_Button_Script.h"
#include "GameInstance.h"
#include "Dynamic_Text_UI.h"

CUI_Button_Script::CUI_Button_Script(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUI_Container{pDevice, pContext}
{
}

CUI_Button_Script::CUI_Button_Script(const CUI_Button_Script& Prototype)
    :CUI_Container{Prototype}
{
}

_float2 CUI_Button_Script::Get_ButtonPos()
{
    return m_PartObjects[0]->Get_Pos();
}

void CUI_Button_Script::Set_isSelect(_bool isSelect)
{
    m_isSelect = isSelect;

    if (m_isSelect)
    {
        static_cast<CDynamic_UI*>(m_PartObjects[0])->Set_iTextureIndex(1);
        m_PartObjects[1]->Set_Color({ 0.8f,0.1f,0.1f,1.f });
    }
    else
    {
        static_cast<CDynamic_UI*>(m_PartObjects[0])->Set_iTextureIndex(0);
        m_PartObjects[1]->Set_Color({ 1.f,1.f,1.f,1.f });
    }
}

HRESULT CUI_Button_Script::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CUI_Button_Script::Initialize(void* pArg)
{
    UI_CONTAINER_DESC eDesc{};
    eDesc.strFilePath = TEXT("../Bin/Save/UI/Script/Script_Select_Button.json");
    
    __super::Initialize(&eDesc);

    Set_isSelect(false);

    return S_OK;
}

void CUI_Button_Script::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CUI_Button_Script::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

   

}

void CUI_Button_Script::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);
}

HRESULT CUI_Button_Script::Render()
{
    return S_OK;
}

void CUI_Button_Script::Update_Script(const string strText)
{
    static_cast<CDynamic_Text_UI*>(m_PartObjects[1])->Set_Caption(StringToWStringU8(strText));
}

void CUI_Button_Script::Check_Select()
{

    Set_isSelect(Check_MousePos());

    
}

_bool CUI_Button_Script::Check_MousePos()
{
    POINT ptScreen;
    GetCursorPos(&ptScreen);

    // 항상 내 게임 윈도우 기준으로 좌표 변환해야 함
    ScreenToClient(g_hWnd, &ptScreen);

    _float2 vSize = m_PartObjects[0]->Get_Size();
    _float2 vPos = m_PartObjects[0]->Get_Pos();


    if (ptScreen.x < vPos.x - 0.5f * vSize.x || ptScreen.x >vPos.x + 0.5f * vSize.x)
        return false;

    if (ptScreen.y < vPos.y - 0.5f * vSize.y || ptScreen.y > vPos.y + 0.5f * vSize.y)
        return false;


    return true;
}

void CUI_Button_Script::Update_Position(_float fX, _float fY)
{

    for (auto& pPart : m_PartObjects)
    {
        pPart->Set_Position(fX, fY);
    }

    _float2 vPos = Get_ButtonPos();

    m_PartObjects[1]->Set_Position( vPos.x - g_iWinSizeX * 0.1f, vPos.y - g_iWinSizeY * 0.015f  );

}

CUI_Button_Script* CUI_Button_Script::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CUI_Button_Script* pInstance = new CUI_Button_Script(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created : CUI_Button_Script");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CUI_Button_Script::Clone(void* pArg)
{
    CUI_Button_Script* pInstance = new CUI_Button_Script(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : CUI_Button_Script");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CUI_Button_Script::Free()
{
    __super::Free();
}
