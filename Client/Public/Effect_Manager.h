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
        _float4x4* pSocketMatrix = { nullptr };         // �켱������ ���Ǵ� �θ� ��� (e.g. �����ϸ�Ʈ����, Ȥ�� �ܼ��� �θ� ��� - �� ��� �����ϸ� EC�� Ʈ�������� �����Ͻÿ� - )
        _float4x4* pParentCombinedMatrix = { nullptr }; // ���� ����� ������ �θ� ����� �� ���� �ϴ� ��� ����� ��.. (�� ������ ��� -> �θ� ��ü�� ���� ��Ʈ����)
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
    /* EC�� EC�ּ� ��ȯ�� �� �ִ� �Լ� ������ �����*/
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
    HRESULT Ready_Prototype_Trail_VIBuffers(const json& j); // Ʈ���� �߰��ϸ� �̰� 2pƮ����

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
