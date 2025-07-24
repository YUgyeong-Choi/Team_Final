#include "Camera_Manager.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CCamera_Manager)

CCamera_Manager::CCamera_Manager() :
    m_pGameInstance{ CGameInstance::Get_Instance() }
{
}

HRESULT CCamera_Manager::Initialize(LEVEL eLevel)
{
    CCamera_Free::CAMERA_FREE_DESC            Desc{};
    
    Desc.vEye = _float3(-2.3f, 7.5f, 60.3f);
    Desc.vAt = _float3(0.f, 0.f, 0.f);
    Desc.fFov = XMConvertToRadians(60.f);
    Desc.fNear = 0.01f;
    Desc.fFar = 500.f;
    Desc.fSpeedPerSec = 30.f;
    Desc.fRotationPerSec = XMConvertToRadians(180.f);

    CGameObject* pCamera = nullptr;
    CGameObject** ReturnCam = &pCamera;
    if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_Free"),
        ENUM_CLASS(eLevel), TEXT("Layer_Camera"), ReturnCam, &Desc)))
        return E_FAIL;

    m_pCamera_Free = static_cast<CCamera_Free*>(*ReturnCam);


    CCamera_Orbital::CAMERA_ORBITAL_DESC       OrbitalDesc{};

    OrbitalDesc.vEye = _float3(0.f, 20.f, -15.f);
    OrbitalDesc.vAt = _float3(0.f, 0.f, 0.f);
    OrbitalDesc.fFov = XMConvertToRadians(60.f);
    OrbitalDesc.fNear = 0.01f;
    OrbitalDesc.fFar = 500.f;
    OrbitalDesc.fMouseSensor = 0.07f;
    OrbitalDesc.fSpeedPerSec = 30.f;
    OrbitalDesc.fRotationPerSec = XMConvertToRadians(180.f);

    CGameObject* pCamera2 = nullptr;
    CGameObject** ReturnCam2 = &pCamera2;
    if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_Orbital"),
        ENUM_CLASS(eLevel), TEXT("Layer_Camera"), ReturnCam2, &Desc)))
        return E_FAIL;

    m_pCamera_Orbital = static_cast<CCamera_Orbital*>(*ReturnCam2);


    CCamera_CutScene::CAMERA_CutScene_DESC       CutSceneDesc{};

    CutSceneDesc.vEye = _float3(0.f, 20.f, -15.f);
    CutSceneDesc.vAt = _float3(0.f, 0.f, 0.f);
    CutSceneDesc.fFov = XMConvertToRadians(60.f);
    CutSceneDesc.fNear = 0.01f;
    CutSceneDesc.fFar = 500.f;
    CutSceneDesc.fMouseSensor = 0.07f;
    CutSceneDesc.fSpeedPerSec = 30.f;
    CutSceneDesc.fRotationPerSec = XMConvertToRadians(180.f);

    CGameObject* pCamera3 = nullptr;
    CGameObject** ReturnCam3 = &pCamera3;
    if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_Camera_CutScene"),
        ENUM_CLASS(eLevel), TEXT("Layer_Camera"), ReturnCam3, &CutSceneDesc)))
        return E_FAIL;

    m_pCamera_CutScene = static_cast<CCamera_CutScene*>(*ReturnCam3);


    /* [ �ʱ� ī�޶� ���� ] */
    m_pCurCamera = m_pCamera_Orbital;

    return S_OK;
}

HRESULT CCamera_Manager::Update(_float fTimeDelta)
{
    /* [ ī�޶� ��ȯ�ϸ� ���������ο� �ѱ�� ] */
	if (m_pGameInstance->Key_Down(DIK_9))
        m_pCurCamera = m_pCamera_Free;
    if (m_pGameInstance->Key_Down(DIK_8))
        m_pCurCamera = m_pCamera_Orbital;

    if (m_pCurCamera == m_pCamera_Free)
        m_bMoveable = false;
    else if (m_pCurCamera == m_pCamera_CutScene)
        m_bMoveable = false;
    else
        m_bMoveable = true;

    m_pCurCamera->Update_Camera();

    return S_OK;
}

HRESULT CCamera_Manager::Render()
{
    return S_OK;
}

_fvector CCamera_Manager::GetCurCamPos()
{
    return m_pCurCamera->GetPosition();
}

CCamera* CCamera_Manager::GetCurCam()
{
    return m_pCurCamera;
}

void CCamera_Manager::Shake_Camera(_float fIntensity, _float fDuration, _float fShakeFreqPos, _float fShakeFreqRot)
{
    if (m_pCurCamera)
        m_pCurCamera->StartShake(fIntensity, fDuration, fShakeFreqPos, fShakeFreqRot);
}

void CCamera_Manager::ActiveDialogView(_vector NPCPos, _vector NPCLook)
{
    m_pCamera_Orbital->ActiveDialogView(NPCPos, NPCLook);
}
void CCamera_Manager::InactiveDialogView()
{
    m_pCamera_Orbital->SetbActiveDialogView(false);
}


void CCamera_Manager::Free()
{
    __super::Free();

}