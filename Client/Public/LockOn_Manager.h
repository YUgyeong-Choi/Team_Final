#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CUnit;

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
    void SetPlayer(CUnit* pPlayer);
    void Add_LockOnTarget(CUnit* pTarget);

    void Set_Active();
    void Set_Off(CUnit* pObj);

    CUnit* Get_Target() { return m_pBestTarget; }

private:
    // Hp가 0이하 제외, 벽 뒤에 있는 것 제외
    void RemoveSomeTargets();
    // 각도가 제일 작은거 <- 가운데 있 는 것
    CUnit* Find_ClosestToLookTarget();
    CUnit* Change_ToLookTarget();
    
private:
    CGameInstance* m_pGameInstance = { nullptr };
private:
    CUnit* m_pPlayer = { nullptr };
    CUnit* m_pBestTarget = { nullptr };
    vector<CUnit*> m_vecTarget;

    _bool m_bActive = false;
    _bool m_bStartLockOn = false;
    //_float4 m_vTargetpos = {};

    _float m_fCoolChangeTarget = {};
    _bool m_bCanChangeTarget = true;
public:
    virtual void Free() override;
};

NS_END

