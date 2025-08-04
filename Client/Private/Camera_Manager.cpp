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


    /* [ 초기 카메라 설정 ] */
    m_pCurCamera = m_pCamera_Orbital;

    return S_OK;
}

HRESULT CCamera_Manager::Update(_float fTimeDelta)
{
    /* [ 카메라를 전환하며 파이프라인에 넘긴다 ] */
	if (m_pGameInstance->Key_Down(DIK_P))
        m_pCurCamera = m_pCamera_Free;
    if (m_pGameInstance->Key_Down(DIK_O))
        m_pCurCamera = m_pCamera_Orbital;

    if (m_pCurCamera == m_pCamera_Free)
        m_bMoveable = false;
    else if (m_pCurCamera == m_pCamera_CutScene)
        m_bMoveable = false;
    else
        m_bMoveable = true;

    m_pCurCamera->Update_Camera();

    m_vCurCamRight = XMVector3Normalize(m_pCurCamera->GetRightVector());
    m_vCurCamUp = XMVector3Normalize(m_pCurCamera->GetUpVector());
    m_vCurCamLook = XMVector3Normalize(m_pCurCamera->GetLookVector());

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

void CCamera_Manager::Play_CutScene(CUTSCENE_TYPE cutSceneType)
{
    // 오비탈 -> 컷씬을 위한 정보
    _matrix oribtalMatrix = m_pCamera_Orbital->Get_TransfomCom()->Get_WorldMatrix();
    m_pCamera_CutScene->Get_TransfomCom()->Set_WorldMatrix(oribtalMatrix);
    SetCutSceneCam();
    m_pCamera_CutScene->Set_CutSceneData(cutSceneType);
    m_pCamera_CutScene->PlayCutScene();
}

void CCamera_Manager::SetPlayer(CGameObject* pPlayer)
{
    m_pCamera_Orbital->SetPlayer(pPlayer);
    if (pPlayer)
    {
        m_pCamera_Orbital->Get_TransfomCom()->Set_WorldMatrix(m_pCamera_Orbital->Get_OrbitalWorldMatrix(0.112646f, -1.535662f));
        m_pCamera_Orbital->Set_PitchYaw(0.112646f, -1.535662f);
    }
}
void CCamera_Manager::Shake_Camera(_float fIntensity, _float fDuration, _float fShakeFreqPos, _float fShakeFreqRot)
{
    if (m_pCurCamera)
        m_pCurCamera->StartShake(fIntensity, fDuration, fShakeFreqPos, fShakeFreqRot);
}


void CCamera_Manager::Free()
{
    __super::Free();

}