
#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CUnit;

class CAStar_Manager final : public CBase
{
    DECLARE_SINGLETON(CAStar_Manager)
private:
    CAStar_Manager();
    virtual ~CAStar_Manager() = default;

public:
    virtual HRESULT Initialize(LEVEL eLevel);
    virtual HRESULT Priority_Update(_float fTimeDelta);
    virtual HRESULT Update(_float fTimeDelta);
    virtual HRESULT Late_Update(_float fTimeDelta);
    virtual HRESULT Render();
public:
 
    // 막혀있는 경우는 false
    _bool Find_Route();

    // 루트를 좀 더 부드럽게? 셀 중앙을 가지 않아도 되면  안가도록
    void Adjust_Route();

    


private:
    CGameInstance* m_pGameInstance = { nullptr };

public:
    virtual void Free() override;
};

NS_END

