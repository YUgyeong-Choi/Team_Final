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
public:
    typedef struct tagEffectManagerDesc
    {
        _float3 vPresetPos = {};
        _float4x4* pSocketMatrix = { nullptr };         // 우선적으로 사용되는 부모 행렬 (e.g. 뼈소켓매트릭스, 혹은 단순히 부모 행렬 - 이 경우 웬만하면 EC의 트랜스폼을 조정하시오 - )
        _float4x4* pParentCombinedMatrix = { nullptr }; // 위의 행렬이 별개로 부모 행렬을 또 들어야 하는 경우 사용할 것.. (뼈 소켓일 경우 -> 부모 객체의 월드 매트릭스)
    }EFFDESC;
private:
    CEffect_Manager();
    virtual ~CEffect_Manager() = default;

public:
    virtual HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring EffectFilePath);
    virtual HRESULT Update(_float fTimeDelta);
    virtual HRESULT Render();

public:
    class CGameObject* Make_Effect(_uint iLevelIndex, const _wstring& strEffectTag, const _wstring& strLayerTag, const _float3& vPresetPos = { 0.f, 0.f, 0.f }, void* pArg = nullptr);
    class CGameObject* Make_EffectContainer(_uint iLevelIndex, const _wstring& strECTag, const _float3& vPresetPos = {0.f, 0.f, 0.f});
    /* EC도 EC주소 반환할 수 있는 함수 별개로 만들기*/
    //class CEffectBase* Find_Effect(const _wstring& strEffectTag);
    class CEffectContainer* Find_EffectContainer(const _wstring& strECTag);
    HRESULT Ready_Effect(const _wstring strEffectPath);
    HRESULT Ready_EffectContainer(const _wstring strECPath);

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
    //map<const _wstring, CEffectContainer*>  m_ECPrototypes;
    map<const _wstring, EFFECT_DESC>        m_EffectCloneDescs;
    map<const _wstring, const json>         m_ECJsonDescs;

public:
    virtual void Free() override;

};

NS_END
