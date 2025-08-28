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
    class CGameObject* Make_Effect(_uint iLevelIndex, const _wstring& strEffectTag, const _wstring& strLayerTag, const _float3& vPresetPos = { 0.f, 0.f, 0.f }, void* pArg = nullptr);
    class CGameObject* Make_EffectContainer(_uint iLevelIndex, const _wstring& strECTag, void* pArg = nullptr);
    HRESULT Ready_Effect(const _wstring strEffectPath);
    HRESULT Ready_EffectContainer(const _wstring strECPath);


// 이펙트 매니저가 이펙트 수명관리를 하지 않기 때문에 임시로 영구Loop이펙트들만 매니저에 등록 후 관리할 수 있도록 처리함
// 나중에 이펙트 매니저쪽으로 Update가 전부 넘어가면 변경 예정 (지금은 ObjectManager가 처리함)
public: 
    void Store_EffectContainer(const _wstring& strECTag, class CEffectContainer* pEC);
    class CEffectContainer* Find_EffectContainer(const _wstring& strECTag);
    void Release_EffectContainer(const _wstring& strECTag);
    void Set_Dead_EffectContainer(const _wstring& strECTag);
    void Set_Active_Effect(const _wstring& strECTag, _bool bActive);

private:
    HRESULT Ready_Prototypes();
    HRESULT Ready_Prototype_Components(const json& j, EFFECT_TYPE eEffType);
    HRESULT Ready_Prototype_Models(const json& j);
    HRESULT Ready_Prototype_Textures(const json& j);    
    HRESULT Ready_Prototype_Particle_VIBuffers(const json& j);
    HRESULT Ready_Prototype_Trail_VIBuffers(const json& j); // 트레일 추가하면 이게 2p트레일

private:
    ID3D11Device*           m_pDevice = { nullptr };
    ID3D11DeviceContext*    m_pContext = { nullptr };
    CGameInstance*          m_pGameInstance = { nullptr };

public:
    typedef struct tagEffectCloneDesc
    {
        //_wstring    strEffectTextureTag[TU_END];
        _wstring    strEffectVIBufferTag;
        json        tEffectJson;
    }EFFECT_DESC;

private:
    typedef map<const _wstring, class CEffectContainer*> CONTAINERS;
    //map<const _wstring, CEffectContainer*>  m_ECPrototypes;
    map<const _wstring, EFFECT_DESC>    m_EffectCloneDescs;
    map<const _wstring, const json>     m_ECJsonDescs;

    CONTAINERS                          m_ECs;

public:
    virtual void Free() override;

};

NS_END
