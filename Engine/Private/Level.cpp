#include "Level.h"
#include "GameInstance.h"

CLevel::CLevel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice { pDevice }
    , m_pContext { pContext }
    , m_pGameInstance { CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CLevel::Initialize()
{
    return S_OK;
}

void CLevel::Priority_Update(_float fTimeDelta)
{

}

void CLevel::Update(_float fTimeDelta)
{
    // ���콺�� ȭ�� �߾����� �̵�
    /*if (MOUSE_PRESSING(DIM::RBUTTON))
        HoldMouse();*/
}

void CLevel::Late_Update(_float fTimeDelta)
{

}

HRESULT CLevel::Render()
{
    return S_OK;
}

void CLevel::HoldMouse()
{
    // ȭ�� �ػ� ���� �߾� ��ǥ ���ϱ�
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // ���콺�� ȭ�� �߾����� �̵�
    SetCursorPos(screenWidth / 2, screenHeight / 2);
    ShowCursor(FALSE);
}

void CLevel::Free()
{
    __super::Free();

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
    
}
