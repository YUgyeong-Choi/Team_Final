#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CLockOn_Manager final : public CBase
{
    DECLARE_SINGLETON(CLockOn_Manager)
private:
    CLockOn_Manager();
    virtual ~CLockOn_Manager() = default;

public:
    virtual HRESULT Initialize(LEVEL eLevel);
    virtual HRESULT Priority_Update(_float fTimeDelta);
    virtual HRESULT Update(_float fTimeDelta);
    virtual HRESULT Late_Update(_float fTimeDelta);
    virtual HRESULT Render();
public:
    void SetPlayer(CGameObject* pPlayer);
    void Add_LockOnTarget(CGameObject* pTarget);

    void Set_Active();

    CGameObject* Get_Target() { return m_pBestTarget; }

private:
    void RemoveBehindWallTargets();
    CGameObject* Find_ClosestToLookTarget();
private:
    CGameInstance* m_pGameInstance = { nullptr };
private:
    CGameObject* m_pPlayer = { nullptr };
    CGameObject* m_pBestTarget = { nullptr };
    vector<CGameObject*> m_vecTarget;

    _bool m_bActive = false;
    _bool m_bStartLockOn = false;
    //_float4 m_vTargetpos = {};
public:
    virtual void Free() override;

};

NS_END

