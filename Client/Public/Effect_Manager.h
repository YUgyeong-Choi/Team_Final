#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CGameInstance;
class CGameObject;
NS_END

NS_BEGIN(Client)
class CEffectBase;

class CEffect_Manager final : public CBase
{
    DECLARE_SINGLETON(CEffect_Manager)
private:
    CEffect_Manager();
    virtual ~CEffect_Manager() = default;

public:
    virtual HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring EffectFilePath);
    virtual HRESULT Update(_float fTimeDelta);
    virtual HRESULT Render();

public:
    void Test();
    class CEffectBase* Make_Effect(const _wstring strEffectTag);
    class CEffectContainer* Make_EffectContainer(const _wstring strECTag);

private:
    HRESULT Ready_Prototypes();
    HRESULT Ready_Effect(const _wstring strEffectPath);
    HRESULT Ready_EffectContainer(const _wstring strECPath);
    HRESULT Ready_Prototype_Components(const json& j, EFFECT_TYPE eEffType);
    HRESULT Ready_Prototype_Models(const json& j);
    HRESULT Ready_Prototype_Textures(const json& j);
    HRESULT Ready_Prototype_VIBuffers(const json& j);

private:
    ID3D11Device*           m_pDevice = { nullptr };
    ID3D11DeviceContext*    m_pContext = { nullptr };
    CGameInstance*          m_pGameInstance = { nullptr };

public:
    typedef struct tagEffectCloneDesc
    {
        _wstring    strEffectTextureTag[TU_END];
        _wstring    strEffectVIBufferTag;

    }EFFECT_DESC;

private:
    map<const _wstring, CEffectContainer*>  m_ECPrototypes;
    map<const _wstring, EFFECT_DESC>        m_EffectCloneDescs;

public:
    virtual void Free() override;

};

NS_END
