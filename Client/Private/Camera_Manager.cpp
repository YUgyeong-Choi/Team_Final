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
    // 플레이어 등 뒤에 위치하며 플레이어랑 똑같은 Loook벡터를 가진 컷신이 시작할 때와 끝났을 때의 Obital카메라 위치
    _vector vPlayerLook = XMVector3Normalize(m_pCamera_Orbital->Get_PlayerLook());
    _vector vOffset = vPlayerLook * -2.5f+ XMVectorSet(0.f, 1.5f, 0.f, 0.f);
    _vector vTargetCamPos = m_pCamera_Orbital->Get_PlayerPos() + vOffset;

    _vector vLook = XMVector3Normalize(vPlayerLook);
    _vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);
    _vector vRight = XMVector3Normalize(XMVector3Cross(vUp, vLook));
    vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    _matrix matWorld = XMMatrixIdentity();
    matWorld.r[0] = XMVectorSetW(vRight, 0.f);
    matWorld.r[1] = XMVectorSetW(vUp, 0.f);
    matWorld.r[2] = XMVectorSetW(vLook, 0.f);
    matWorld.r[3] = XMVectorSetW(vTargetCamPos, 1.f);

    m_pCamera_CutScene->Set_InitOrbitalWorldMatrix(matWorld);

    _matrix oribtalMatrix = m_pCamera_Orbital->Get_TransfomCom()->Get_WorldMatrix();
    m_pCamera_CutScene->Get_TransfomCom()->Set_WorldMatrix(oribtalMatrix);
    SetCutSceneCam();
    m_pCamera_CutScene->Set_CutSceneData(cutSceneType);
    m_pCamera_CutScene->PlayCutScene();
}

void CCamera_Manager::SetPlayer(CGameObject* pPlayer)
{
	m_pCamera_Orbital->SetPlayer(pPlayer);
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