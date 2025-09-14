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
    // 마우스를 화면 중앙으로 이동
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

void CLevel::Start_BGM(string soundName, _bool bNowPlaying, _bool bNotLoop, string willMainBGM)
{
    if (!bNowPlaying)
    {
        m_pBGM = m_pGameInstance->Get_Single_Sound(soundName);
        m_pBGM->Set_Volume(1.f * g_fBGMSoundVolume);
        m_pBGM->Play();
    }
    else
    {
		m_bBGMToZero = true;
		m_strWillChangeBGM = soundName;
    }

    if (bNotLoop) {
        m_bCheckBGMFinish = true;
		m_strWillMainBGM = willMainBGM;
    }

        
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
            m_pBGM = m_pGameInstance->Get_Single_Sound(m_strWillChangeBGM);
            m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
            m_pBGM->Play();


            m_bBGMToZero = false;
            m_bBGMToVolume = true;
        }
	}

	if (m_bBGMToVolume)
	{
		// m_fBGMVolume 이 0일텐데 1로 lerp할거임
		m_fBGMVolume = LerpFloat(m_fBGMVolume, 1.f, fTimeDelta * 2.f);
		m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);

		// 만약에 m_fBGMVolume가 1이 되면
        if (m_fBGMVolume > 0.99f)
        {
			m_fBGMVolume = 1.f;
            m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
            m_bBGMToVolume = false;
        }
	}

    if (m_bCheckBGMFinish)
    {
		if (m_pBGM && !m_pBGM->IsPlaying())
		{
            m_pBGM->Stop();
            Safe_Release(m_pBGM);
            m_pBGM = m_pGameInstance->Get_Single_Sound(m_strWillMainBGM);
            m_pBGM->Set_Volume(m_fBGMVolume * g_fBGMSoundVolume);
            m_pBGM->Play();

            m_bCheckBGMFinish = false;
		}
    }
}

void CLevel::HoldMouse()
{
    // 화면 해상도 기준 중앙 좌표 구하기
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // 마우스를 화면 중앙으로 이동
    SetCursorPos(screenWidth / 2, screenHeight / 2);
    ShowCursor(FALSE);
}

void CLevel::Free()
{
    __super::Free();

    if (m_pBGM)
    {
        m_pBGM->Stop();
        Safe_Release(m_pBGM);
    }

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
    Safe_Release(m_pGameInstance);
    
}
