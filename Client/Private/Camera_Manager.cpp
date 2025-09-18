#include "Camera_Manager.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(CCamera_Manager)

CCamera_Manager::CCamera_Manager() :
    m_pGameInstance{ CGameInstance::Get_Instance() }
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CCamera_Manager::Initialize(LEVEL eLevel)
{
    CCamera_Free::CAMERA_FREE_DESC            Desc{};
    
    Desc.vEye = _float3(-2.3f, 7.5f, 60.3f);
    Desc.vAt = _float3(0.f, 0.f, 0.f);
    Desc.fFov = XMConvertToRadians(60.f);
    Desc.fNear = 0.01f;
    Desc.fFar = 1000.f;
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
    OrbitalDesc.fFar = 1000.f;
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
    CutSceneDesc.fFar = 1000.f;
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
    m_pGameInstance->Set_CurCam(m_pCurCamera);

    return S_OK;
}

HRESULT CCamera_Manager::Update(_float fTimeDelta)
{
    m_pGameInstance->Set_CurCam(m_pCurCamera);

    /* [ 카메라를 전환하며 파이프라인에 넘긴다 ] */
    if (m_bCameraSwitchEnabled && m_pGameInstance->Key_Down(DIK_P))
    {
        m_pCurCamera = m_pCamera_Free;
        m_bMoveable = false;
    }

    if (m_bCameraSwitchEnabled && m_pGameInstance->Key_Down(DIK_O))
    {
        m_pCurCamera = m_pCamera_Orbital;
        m_bMoveable = true;
    }

    m_pCurCamera->Update_Camera();

    m_vCurCamRight = XMVector3Normalize(m_pCurCamera->GetRightVector());
    m_vCurCamUp = XMVector3Normalize(m_pCurCamera->GetUpVector());
    m_vCurCamLook = XMVector3Normalize(m_pCurCamera->GetLookVector());


    /* [ 유경이 보아라 (멤버변수에 스피드 2개있는데 포그 , 밝기 스피드) ] */
    if (KEY_DOWN(DIK_Y))
        m_bFestivalLightSwitch = !m_bFestivalLightSwitch;

	FestivalLight_OnOff(fTimeDelta);
    
    /* [ 이건 램프 조명 온, 오프 ] */
    if (KEY_DOWN(DIK_U))
    {
        CGameObject* pLampLight = m_pGameInstance->Find_CustomLight(TEXT("Lamp_Light"))->back();
        dynamic_cast<CDH_ToolMesh*>(pLampLight)->SetIntensity(0.f);
    }
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
    m_bStartGame = true;
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

void CCamera_Manager::Rot_Camera(_vector vRot, _float fDuration)
{
    if (m_pCurCamera)
		m_pCurCamera->StartRot(vRot, fDuration);
}

void CCamera_Manager::FestivalLight_OnOff(_float fTimeDelta)
{
    if (!m_vecCustomLight && m_bStartGame)
    {
        m_vecCustomLight = m_pGameInstance->Find_CustomLight(TEXT("Festival_Light"));
    }

    
    if (!m_bDoOnce && m_bStartGame && m_vecCustomLight)
    {
        for (size_t i = 0; i < m_vecCustomLight->size(); i++)
        {
            // 페스티벌 조명의 집합을 여기에 모아놓는다.
            CDH_ToolMesh* pLight = dynamic_cast<CDH_ToolMesh*>((*m_vecCustomLight)[i]);
            m_pFestivalLight.push_back(pLight);

            // 포그와 밝기 값을 저장해둔다.
            CustomLightDesc customLightDesc{};
            customLightDesc.fFestivalLightFogSaved = pLight->GetfFogDensity();
            customLightDesc.fFestivalLightSaved = pLight->GetIntensity();
            m_vecCustomLightDesc.push_back(customLightDesc);

            // 처음에는 다 끈다.
            pLight->SetIntensity(0.f);
            pLight->SetfFogDensity(0.f);
        }
        m_bDoOnce = true;
    }


    if (m_pGameInstance->GetCurAreaIds() == 60 || m_pGameInstance->GetCurAreaIds() == 59)
    {
        if (m_bFestivalLightSwitch)
        {
            for (size_t i = 0; i < m_pFestivalLight.size(); i++)
            {
                if (m_vecCustomLightDesc[i].fFestivalLightFogIntensity <= m_vecCustomLightDesc[i].fFestivalLightFogSaved)
                {
                    m_vecCustomLightDesc[i].fFestivalLightFogIntensity += fTimeDelta * m_fFestivalLightFogSpeed;
                    m_pFestivalLight[i]->SetfFogDensity(m_vecCustomLightDesc[i].fFestivalLightFogIntensity);
                }

                if (m_vecCustomLightDesc[i].fFestivalLightIntensity <= m_vecCustomLightDesc[i].fFestivalLightSaved)
                {
                    m_vecCustomLightDesc[i].fFestivalLightIntensity += fTimeDelta * m_fFestivalLightSpeed;
                    m_pFestivalLight[i]->SetIntensity(m_vecCustomLightDesc[i].fFestivalLightIntensity);
                }
            }
        }
        else if (!m_bFestivalLightSwitch)
        {
            for (size_t i = 0; i < m_pFestivalLight.size(); i++)
            {
                if (m_vecCustomLightDesc[i].fFestivalLightFogIntensity > 0.f)
                {
                    m_vecCustomLightDesc[i].fFestivalLightFogIntensity -= fTimeDelta * m_fFestivalLightFogSpeed;
                    m_pFestivalLight[i]->SetfFogDensity(m_vecCustomLightDesc[i].fFestivalLightFogIntensity);
                }

                if (m_vecCustomLightDesc[i].fFestivalLightIntensity > 0.f)
                {
                    m_vecCustomLightDesc[i].fFestivalLightIntensity -= fTimeDelta * m_fFestivalLightSpeed;
                    m_pFestivalLight[i]->SetIntensity(m_vecCustomLightDesc[i].fFestivalLightIntensity);
                }
            }
        }
    }
}


void CCamera_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pGameInstance);
}