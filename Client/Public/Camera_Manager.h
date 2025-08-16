#pragma once

#include "Client_Defines.h"
#include "Camera.h"
#include "Camera_Free.h"
#include "Camera_Orbital.h"
#include "Camera_CutScene.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CCamera_Manager final : public CBase
{
    DECLARE_SINGLETON(CCamera_Manager)
private:
    CCamera_Manager();
    virtual ~CCamera_Manager() = default;

public:
    virtual HRESULT Initialize(LEVEL eLevel);
    virtual HRESULT Update(_float fTimeDelta);
    virtual HRESULT Render();
public:
    _fvector GetCurCamPos();
    CCamera* GetCurCam();
    CCamera_CutScene* GetCutScene() { return m_pCamera_CutScene; }
    CCamera_Orbital* GetOrbitalCam() { return m_pCamera_Orbital; }
    CCamera_Free* GetFreeCam() { return m_pCamera_Free; }
    _bool GetbMoveable() { return m_bMoveable; }
    void SetbMoveable(_bool bMove) { m_bMoveable = bMove; }
    _bool GetIsShake() { return m_pCurCamera->GetIsShake(); }
    _vector GetPureCamPos() const { return m_pCurCamera->GetPureCamPos(); }

    void Play_CutScene(CUTSCENE_TYPE cutSceneType);
public:
    const _vector& GetCurCamRight() const { return m_vCurCamRight; }
    const _vector& GetCurCamUp() const { return m_vCurCamUp; }
    const _vector& GetCurCamLook() const { return m_vCurCamLook; }
public:
    void SetPlayer(CGameObject* pPlayer);

public:
    void	SetFreeCam() { m_pCurCamera = m_pCamera_Free; m_bMoveable = false; }
    void	SetOrbitalCam() { m_pCurCamera = m_pCamera_Orbital; m_bMoveable = true; }
    void	SetCutSceneCam() { m_pCurCamera = m_pCamera_CutScene; m_bMoveable = false; }
    void	Shake_Camera(_float fIntensity = 1.f, _float fDuration = 1.f, _float fShakeFreqPos = 100.f, _float fShakeFreqRot = 40.f);

public:
	void Set_StartGame(_bool bStart) { m_bStartGame = bStart; }
	_bool Get_StartGame() const { return m_bStartGame; }

private:
    CCamera_Free* m_pCamera_Free = { nullptr };
    CCamera_Orbital* m_pCamera_Orbital = { nullptr };
    CCamera_CutScene* m_pCamera_CutScene = { nullptr };

private:
    _vector m_vCurCamRight = {};
    _vector m_vCurCamUp = {};
    _vector m_vCurCamLook = {};

private:
    _bool m_bStartGame = {};

private:
    CGameInstance* m_pGameInstance = { nullptr };

private:
    CCamera* m_pCurCamera = { nullptr };
private:
    // 플레이어가 움직임 방지 및 카메라 회전 방지
    _bool				m_bMoveable = {};
public:
    virtual void Free() override;

};

NS_END

