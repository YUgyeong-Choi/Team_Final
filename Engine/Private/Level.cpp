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

    if (m_pBGM && !m_pBGM->IsPlaying())
    {
        m_pBGM->Stop();
        Safe_Release(m_pBGM);

        if (m_BGMQueued != "")
        {
            m_CurBGMName = m_BGMQueued;
            m_BGMQueued = "";
        }

        m_pBGM = m_pGameInstance->Get_Single_Sound(m_CurBGMName);
        if (m_pBGM)
        {
            m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
            m_pBGM->Play();
        }
    }
}

void CLevel::Late_Update(_float fTimeDelta)
{

}

HRESULT CLevel::Render()
{
    return S_OK;
}

void CLevel::Start_BGM(string soundName)
{	
    m_pBGM = m_pGameInstance->Get_Single_Sound(soundName);
    m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
    m_pBGM->Play();
    m_CurBGMName = soundName;
}

void CLevel::Change_BGM(string soundName)
{
    /* [ ���� ] */
    m_bBGMToZero = true;
    m_BGMNext = soundName;
}

void CLevel::Change_BGM(string soundNoLoopName, string soundName)
{
    m_bBGMToZero = true;
    m_BGMNext = soundNoLoopName;
    m_BGMQueued = soundName;
}

static _float LerpFloat(_float a, _float b, _float t)
{
	return a + (b - a) * t;
}

void CLevel::Update_ChangeBGM(_float fTimeDelta)
{
	if (m_bBGMToZero)
	{
        m_fBGMVolume = LerpFloat(m_fBGMVolume, 0.f, fTimeDelta * 3.f);
        m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

        if (m_fBGMVolume < 0.01f)
        {
            m_pBGM->Stop();
            Safe_Release(m_pBGM);
            m_pBGM = m_pGameInstance->Get_Single_Sound(m_BGMNext);
            if (m_pBGM)
            {
                m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
                m_pBGM->Play();
				m_CurBGMName = m_BGMNext;
                m_BGMNext = "";
                m_bBGMToVolume = true;
            }
            else
            {
                m_CurBGMName = "";
                m_BGMNext = "";
            }

            m_bBGMToZero = false;
        }
	}

	if (m_bBGMToVolume)
	{
		m_fBGMVolume = LerpFloat(m_fBGMVolume, 1.f, fTimeDelta * 1.5f);
		m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

        if (m_fBGMVolume > 0.99f)
        {
			m_fBGMVolume = 1.f;
            m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
            m_bBGMToVolume = false;
        }
	}
}

void CLevel::Stop_BGM()
{
    if (m_pBGM) {
        m_pBGM->Stop();
        Safe_Release(m_pBGM);
		m_CurBGMName = "";
    }
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

    if (m_pBGM)
    {
        m_pBGM->Stop();
        //Safe_Release(m_pBGM);

        _int iRefCount = Safe_Release(m_pBGM);
        if (iRefCount != 0)
        {
            //MSG_BOX("���� ������ ����");
            printf_s("SoundCore Ref Count %d\n", iRefCount);
        }

    }

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
    
}
