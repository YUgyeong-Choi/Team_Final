#include "Effect_Manager.h"
#include "GameInstance.h"
#include "EffectContainer.h"
#include "ParticleEffect.h"
#include "SpriteEffect.h"
#include "MeshEffect.h"

IMPLEMENT_SINGLETON(CEffect_Manager)


CEffect_Manager::CEffect_Manager()
    : m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
}
/*
* [���� ���]
* ����Ʈ�� ������ ���� EffectContainer��
* ���� ����Ʈ��(SE, PE, ME, TE) ������ ������ �ϴ���
* EC�� ������ �� ���Ϸ� �ɰ��� ���� ����Ʈ�� ����ִ��� ���ϸ��� �����Ұ���
* �׳� ��� ����Ʈ�� EC�� ���� ��������Ʈ���� �׷쿡 ��������
*/

HRESULT CEffect_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring EffectFilePath)
{
    m_pDevice = pDevice;
    Safe_AddRef(m_pDevice);
    m_pContext= pContext;
    Safe_AddRef(m_pContext);


    //json j;

    for (const auto& iter : directory_iterator(EffectFilePath))
    {
        if (iter.is_regular_file())
        {
            path filepath = iter.path();

            if (filepath.extension() == L".json")
            {
                _wstring prefix = filepath.stem().wstring().substr(0, 2);

                /* �켱 EffectContainer �������θ� �Ľ��� */
                if (prefix == L"EC")
                    Ready_EffectContainer(filepath.wstring());

                /* �߰� �� �� ���� ���� ���� ������ �������Ƽ� */
                if (prefix == L"SE")
                    int a = 0;
                if (prefix == L"PE")
                    int a = 0;
                if (prefix == L"ME")
                    int a = 0;
                if (prefix == L"TE")
                    int a = 0;

            }
        }
        else
        {
            MSG_BOX("����Ʈ ���� ��ΰ� �߸���.");
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT CEffect_Manager::Update(_float fTimeDelta)
{
    return S_OK;
}

HRESULT CEffect_Manager::Render()
{
    return S_OK;
}

void CEffect_Manager::Test()
{
}

CEffectBase* CEffect_Manager::Make_Effect(const _wstring strEffectTag)
{

    return nullptr;
}

CEffectContainer* CEffect_Manager::Make_EffectContainer(const _wstring strECTag)
{



    return nullptr;
}

HRESULT CEffect_Manager::Ready_Prototypes()
{
    /* For.Prototype_GameObject_SpriteEffect */
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SpriteEffect"),
        CSpriteEffect::Create(m_pDevice, m_pContext))))
        return E_FAIL;
    
    /* For.Prototype_GameObject_ParticleEffect */
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ParticleEffect"),
        CParticleEffect::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_MeshEffect */
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MeshEffect"),
        CMeshEffect::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    ///* For.Prototype_GameObject_TrailEffect */
    //if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_TrailEffect"),
    //    CTrailEffect::Create(m_pDevice, m_pContext))))
    //    return E_FAIL;

    /* For.Prototype_GameObject_CEffectContainer */
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_CEffectContainer"),
        CEffectContainer::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    return S_OK;
}


HRESULT CEffect_Manager::Ready_EffectContainer(const _wstring strECPath)
{
    //CEffectContainer* pInstance = CEffectContainer::Create();
    json j;
    ifstream ifs(strECPath);
    if (!ifs.is_open())
    {
        MSG_BOX("EC Filepath Open Failed");
        return E_FAIL;
    }

    ifs >> j;
    ifs.close();

    if (!j.contains("EffectObject") || !j["EffectObject"].is_array())
    {
        return E_FAIL;
    }
    for (const auto& jItem : j["EffectObject"])
    {
        EFFECT_TYPE eEffectType = {};
        if (jItem.contains("EffectType"))
            eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<int>());


        /* �ʿ��� ������Ÿ�� ������Ʈ ���� */
        Ready_Prototype_Components(jItem, eEffectType);


        /*switch (eEffectType)
        {
        case Client::EFF_SPRITE:
            break;
        case Client::EFF_PARTICLE:
            break;
        case Client::EFF_MESH:
            break;
        case Client::EFF_TRAIL:
            break;
        default:
            break;
        }*/


    }


    return S_OK;
}

HRESULT CEffect_Manager::Ready_Prototype_Components(const json& j, EFFECT_TYPE eEffType)
{
    Ready_Prototype_Textures(j);
    if (eEffType == EFF_MESH)
        Ready_Prototype_Models(j);
    if (eEffType == EFF_PARTICLE)
        Ready_Prototype_VIBuffers(j);
    return S_OK;
}

HRESULT CEffect_Manager::Ready_Prototype_Models(const json& j)
{

    return S_OK;
}

HRESULT CEffect_Manager::Ready_Prototype_Textures(const json& j)
{
    _wstring        TextureFilePath = TEXT("../Bin/Resources/Textures/Effect/");
    _wstring        TexturePreTag = TEXT("Prototype_Component_Texture_");
    _bool			bTextureUsage[TU_END];
    _wstring		TextureTag[TU_END];

    /* [Deserialize Json] */
    if (j.contains("TextureUsage") && j["TextureUsage"].is_array())
    {
        for (int i = 0; i < TU_END && i < j["TextureUsage"].size(); ++i)
            bTextureUsage[i] = j["TextureUsage"][i].get<_bool>();
    }
    if (j.contains("TextureTags") && j["TextureTags"].is_array())
    {
        for (int i = 0; i < TU_END && i < j["TextureTags"].size(); ++i)
            TextureTag[i] = StringToWString(j["TextureTags"][i].get<std::string>());
    }
    
    /* [Create Prototype Component Texture] */
    if (bTextureUsage[TU_DIFFUSE] == true && TextureTag[TU_DIFFUSE].size() != 0)
    {
        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TexturePreTag + TextureTag[TU_DIFFUSE],
            CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + TextureTag[TU_DIFFUSE] + TEXT(".dds")).c_str(), 1))))
            return E_FAIL;
    }
    if (bTextureUsage[TU_MASK1] == true && TextureTag[TU_MASK1].size() != 0)
    {
        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TexturePreTag + TextureTag[TU_MASK1],
            CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + TextureTag[TU_MASK1] + TEXT(".dds")).c_str(), 1))))
            return E_FAIL;
    }
    if (bTextureUsage[TU_MASK2] == true && TextureTag[TU_MASK2].size() != 0)
    {
        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TexturePreTag + TextureTag[TU_MASK2],
            CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + TextureTag[TU_MASK2] + TEXT(".dds")).c_str(), 1))))
            return E_FAIL;
    }
    if (bTextureUsage[TU_MASK3] == true && TextureTag[TU_MASK3].size() != 0)
    {
        if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TexturePreTag + TextureTag[TU_MASK3],
            CTexture::Create(m_pDevice, m_pContext, (TextureFilePath + TextureTag[TU_MASK3] + TEXT(".dds")).c_str(), 1))))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CEffect_Manager::Ready_Prototype_VIBuffers(const json& j)
{
    CVIBuffer_Point_Instance::DESC VIBufferDesc = {};
    _wstring strPrototypeTag = TEXT("Prototype_Component_VIBuffer_");
    /* �� ���� ��� �����ص־� �� �� ���� */


    /* �̰� ���� ������ ���� �� �������� �߰��� �� */
    if (j.contains("Name"))
    {
        strPrototypeTag += StringToWString(j["Name"].get<std::string>());
    }
    // �±׾��̳� ���� ���ص� �� �� ���� ��ƼŬ ������ �ϳ��� �̸��� �����ص� ������ �� 

    if (j.contains("NumInstance"))
        VIBufferDesc.iNumInstance = j["NumInstance"].get<_uint>();

    if (j.contains("Range") && j["Range"].is_array() && j["Range"].size() == 3)
        VIBufferDesc.vRange = { j["Range"][0].get<_float>(), j["Range"][1].get<_float>(), j["Range"][2].get<_float>() };

    if (j.contains("Size") && j["Size"].is_array() && j["Size"].size() == 2)
        VIBufferDesc.vSize = { j["Size"][0].get<_float>(), j["Size"][1].get<_float>() };

    if (j.contains("Center") && j["Center"].is_array() && j["Center"].size() == 3)
        VIBufferDesc.vCenter = { j["Center"][0].get<_float>(), j["Center"][1].get<_float>(), j["Center"][2].get<_float>() };

    if (j.contains("Pivot") && j["Pivot"].is_array() && j["Pivot"].size() == 3)
        VIBufferDesc.vPivot = { j["Pivot"][0].get<_float>(), j["Pivot"][1].get<_float>(), j["Pivot"][2].get<_float>() };

    if (j.contains("LifeTime_Particle") && j["LifeTime_Particle"].is_array() && j["LifeTime_Particle"].size() == 2)
        VIBufferDesc.vLifeTime = { j["LifeTime_Particle"][0].get<_float>(), j["LifeTime_Particle"][1].get<_float>() };

    if (j.contains("Speed") && j["Speed"].is_array() && j["Speed"].size() == 2)
        VIBufferDesc.vSpeed = { j["Speed"][0].get<_float>(), j["Speed"][1].get<_float>() };

    if (j.contains("PType"))
        VIBufferDesc.ePType = static_cast<PARTICLETYPE>(j["PType"].get<int>());

    if (j.contains("Loop"))
        VIBufferDesc.isLoop = j["Loop"].get<_bool>();

    //if (j.contains("Local"))
    //    VIBufferDesc.bLocal = j["Local"].get<_bool>();

    VIBufferDesc.isTool = false;

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), strPrototypeTag,
        CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &VIBufferDesc))))
        return E_FAIL;

    return S_OK;
}

void CEffect_Manager::Free()
{
    Safe_Release(m_pGameInstance);

    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
