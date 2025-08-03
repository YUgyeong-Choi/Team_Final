#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)

class CEffect_Manager final : public CBase
{
    DECLARE_SINGLETON(CEffect_Manager)
private:
    CEffect_Manager();
    virtual ~CEffect_Manager() = default;

public:
    virtual HRESULT Initialize(const _wstring EffectFilePath);
    virtual HRESULT Update(_float fTimeDelta);
    virtual HRESULT Render();

public:
    void Test();
    class CEffectBase* Make_Effect(const _wstring strEffectTag);
    class CEffectContainer* Make_EffectContainer(const _wstring strEffectTag);

private:
    HRESULT Ready_Effect(const _wstring strEffectPath);
    HRESULT Ready_EffectContainer(const _wstring strEffectPath);

private:
    CGameInstance* m_pGameInstance = { nullptr };

private:
    map<const _wstring, class CEffectBase*> m_Effects;
    //map<const _wstring, 


public:
    virtual void Free() override;

};

NS_END
