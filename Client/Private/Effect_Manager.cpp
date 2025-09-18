#include "Effect_Manager.h"
#include "GameInstance.h"
#include "EffectContainer.h"
#include "ParticleEffect.h"
#include "SpriteEffect.h"
#include "MeshEffect.h"
#include "SwordTrailEffect.h"

IMPLEMENT_SINGLETON(CEffect_Manager)


CEffect_Manager::CEffect_Manager()
    : m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
}

HRESULT CEffect_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring EffectFilePath)
{
    if (!m_pDevice)
    {
        m_pDevice = pDevice;
        Safe_AddRef(m_pDevice);
    }
	if (!m_pContext)
	{
		m_pContext = pContext;
		Safe_AddRef(m_pContext);
	}

    Ready_Prototypes();

    for (const auto& iter : directory_iterator(EffectFilePath))
    {
        if (iter.is_regular_file())
        {
            path filepath = iter.path();

            if (filepath.extension() == L".json")
            {
                _wstring prefix = filepath.stem().wstring().substr(0, 2);

                /* 우선 EffectContainer 기준으로만 파싱함 */
                if (prefix == L"EC")
                    Ready_EffectContainer(filepath.wstring());

                /* 추가 안 할 수도 있음 지금 구조가 거지같아서 */
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
            MSG_BOX("이펙트 파일 경로가 잘못됨.");
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT CEffect_Manager::Update(_float fTimeDelta)
{
    //m_fAccTime += fTimeDelta;
    return S_OK;
}

HRESULT CEffect_Manager::Render()
{
    return S_OK;
}

CGameObject* CEffect_Manager::Make_Effect(_uint iLevelIndex, const _wstring& strEffectTag, const _wstring& strLayerTag, const _float3& vPresetPos, void* pArg)
{
    auto	iter = m_ECJsonDescs.find(strEffectTag);
    if (iter == m_ECJsonDescs.end())
        return nullptr;

    json jItem = iter->second;

    // 이름
    //if (jItem.contains("Name"))
    //	m_strSeqItemName = jItem["Name"].get<string>();

    // 타입
    EFFECT_TYPE eEffectType = {};
    _wstring prefix = strEffectTag.substr(0, 2);
    if (prefix == L"SE")
        eEffectType = EFF_SPRITE;
    if (prefix == L"PE")
        eEffectType = EFF_PARTICLE;
    if (prefix == L"ME")
        eEffectType = EFF_MESH;
    if (prefix == L"TE")
        eEffectType = EFF_TRAIL;

    // Effect 객체 생성 및 역직렬화
    CGameObject* pInstance = { nullptr };

    switch (eEffectType)
    {
    case Client::EFF_SPRITE:
    {
        CSpriteEffect::DESC desc = {};
        desc.fRotationPerSec = XMConvertToRadians(90.f);
        desc.fSpeedPerSec = 5.f;
        desc.bTool = false;
        pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
            PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_SpriteEffect"), &desc));
    }
    break;
    case Client::EFF_PARTICLE:
    {
        CParticleEffect::DESC* pDesc = static_cast<CParticleEffect::DESC*>(pArg);
        pDesc->fRotationPerSec = XMConvertToRadians(90.f);
        pDesc->fSpeedPerSec = 5.f;
        pDesc->bTool = false;
        if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ParticleEffect"),
            iLevelIndex, strLayerTag, &pInstance, pDesc)))
            return nullptr;
        //pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
        //    PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ParticleEffect"), &desc));
    }
    break;
    case Client::EFF_MESH:
    {
        CMeshEffect::DESC desc = {};
        desc.fRotationPerSec = XMConvertToRadians(90.f);
        desc.fSpeedPerSec = 5.f;
        desc.bTool = false;
        pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
            PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_MeshEffect"), &desc));
    }
    break;
    case Client::EFF_TRAIL:
    {
        if (pArg == nullptr)
        {
			MSG_BOX("CEffect_Manager::Make_Effect - pArg is nullptr");
			return nullptr;
        }

        // 소드트레일만??? 트레일은 개별적으로 처리할 듯? 
        CSwordTrailEffect::DESC* pDesc = static_cast<CSwordTrailEffect::DESC*>(pArg);
        if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_TrailEffect"),
            iLevelIndex, strLayerTag, &pInstance, pArg)))
            return nullptr;
    }
    break;
    }
        
    if (pInstance != nullptr)
    {
        if (FAILED(static_cast<CEffectBase*>(pInstance)->Ready_Effect_Deserialize(jItem)))
        {
            Safe_Release(pInstance);
            return nullptr;
        }
    }
    else
        return nullptr;

    return pInstance;
}


CGameObject* CEffect_Manager::Make_EffectContainer(_uint iLevelIndex, const _wstring& strECTag, void* pArg)
{
    auto	iter = m_ECJsonDescs.find(strECTag);
    if (iter == m_ECJsonDescs.end())
        return nullptr;
    
    CEffectContainer::DESC ECDesc = {};
    XMStoreFloat4x4(&ECDesc.PresetMatrix, XMMatrixIdentity());

    if (pArg != nullptr)
    {
        CEffectContainer::DESC* pDesc = static_cast<CEffectContainer::DESC*>(pArg);
        ECDesc.pParentMatrix = pDesc->pParentMatrix;
        ECDesc.pSocketMatrix = pDesc->pSocketMatrix;
        ECDesc.PresetMatrix = pDesc->PresetMatrix;
    }
    ECDesc.j = iter->second;
    ECDesc.iLevelID = iLevelIndex;
    ECDesc.fRotationPerSec = XMConvertToRadians(90.f); // 수정하고싶으면 툴에서 수정해서 파싱하던가 하쇼
    ECDesc.fSpeedPerSec = 10.f;

    CGameObject* pInstance = { nullptr };
    if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_EffectContainer"),
        iLevelIndex, TEXT("Layer_Effect"), &pInstance , &ECDesc)))
        return nullptr;



    return pInstance;
}

CEffectContainer* CEffect_Manager::Find_EffectContainer(const _wstring& strECTag)
{
    auto	iter = m_ECs.find(strECTag);
    if (iter == m_ECs.end())
        return nullptr;

    return iter->second;
}

void CEffect_Manager::Release_EffectContainer(const _wstring& strECTag)
{
    auto	iter = m_ECs.find(strECTag);
    if (iter == m_ECs.end())
        return ;
    Safe_Release(iter->second);
    m_ECs.erase(iter);
}

void CEffect_Manager::Set_Dead_EffectContainer(const _wstring& strECTag)
{
    auto pEC = Find_EffectContainer(strECTag);
    if (pEC == nullptr)
        return;
    pEC->End_Effect();
    Release_EffectContainer(strECTag);
}

void CEffect_Manager::Set_Active_Effect(const _wstring& strECTag, _bool bActive)
{
    auto pEC = Find_EffectContainer(strECTag);
    if (pEC == nullptr)
        return;
    pEC->Set_isActive(bActive);
}

void CEffect_Manager::Remove_AllStoredECs()
{
    for (auto iter = m_ECs.begin(); iter != m_ECs.end(); iter++)
    {
        Safe_Release(iter->second);
    }
    m_ECs.clear();
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
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_TrailEffect"),
        CSwordTrailEffect::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    /* For.Prototype_GameObject_CEffectContainer */
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_EffectContainer"),
        CEffectContainer::Create(m_pDevice, m_pContext))))
        return E_FAIL;

    return S_OK;
}

HRESULT CEffect_Manager::Ready_Effect(const _wstring strEffectPath)
{
    json jItem;
    ifstream ifs(strEffectPath);
    if (!ifs.is_open())
    {
        MSG_BOX("Effect Filepath Open Failed");
        return E_FAIL;
    }

    ifs >> jItem;
    ifs.close();

    EFFECT_TYPE eEffectType = {};
    if (jItem.contains("EffectType"))
        eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<_int>());

    /* 필요한 프로토타입 컴포넌트 생성 */
    Ready_Prototype_Components(jItem, eEffectType);

    //switch (eEffectType)
    //{
    //case Client::EFF_SPRITE:
    //{

    //}
    //    break;
    //case Client::EFF_PARTICLE:
    //{

    //}
    //    break;
    //case Client::EFF_MESH:
    //{

    //}
    //    break;
    //case Client::EFF_TRAIL:
    //{

    //}
    //    break;
    //case Client::EFF_ONETRAIL:
    //{

    //}
    //    break;
    //default:
    //    break;
    //}
    m_ECJsonDescs.emplace(make_pair(path(strEffectPath).stem(), jItem));

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
            eEffectType = static_cast<EFFECT_TYPE>(jItem["EffectType"].get<_int>());

        /* 필요한 프로토타입 컴포넌트 생성 */
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

    m_ECJsonDescs.emplace(make_pair(path(strECPath).stem(), j));

    return S_OK;
}

// 이펙트매니저가 이펙트 전체 관리하게 변경 전에 땜빵처리입니다..
void CEffect_Manager::Store_EffectContainer(const _wstring& strECTag, class CEffectContainer* pEC)
{
    if (nullptr != Find_EffectContainer(strECTag))
        return;
    m_ECs.emplace(make_pair(strECTag, pEC));
    Safe_AddRef(pEC);
}

HRESULT CEffect_Manager::Ready_Prototype_Components(const json& j, EFFECT_TYPE eEffType)
{
    Ready_Prototype_Textures(j);
    if (eEffType == EFF_MESH)
        Ready_Prototype_Models(j);
    else if (eEffType == EFF_PARTICLE)
        Ready_Prototype_Particle_VIBuffers(j);
    else if (eEffType == EFF_TRAIL)
		Ready_Prototype_Trail_VIBuffers(j);
    else if (eEffType == EFF_ONETRAIL)
		Ready_Prototype_Trail_VIBuffers(j);
    return S_OK;
}

HRESULT CEffect_Manager::Ready_Prototype_Models(const json& j)
{
    string          ModelFileName;
    string          ModelFilePath = "../Bin/Resources/Models/EffectMesh/";
    _wstring        ModelPreTag = TEXT("Prototype_Component_Model_");

    if (j.contains("ModelTag"))
        ModelFileName = j["ModelTag"].get<string>();

    ModelFilePath += (ModelFileName + ".bin");
    ModelPreTag += StringToWString(ModelFileName);

    _matrix		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), ModelPreTag,
        CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, ModelFilePath.c_str(), PreTransformMatrix))))
        return E_FAIL;


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

HRESULT CEffect_Manager::Ready_Prototype_Particle_VIBuffers(const json& j)
{
    CVIBuffer_Point_Instance::DESC VIBufferDesc = {};
    _wstring strPrototypeTag = TEXT("Prototype_Component_VIBuffer_");
    /* 이 버퍼 어디 저장해둬야 할 것 같다 */
    //왜저장하려했더라;

    if (j.contains("Name"))
    {
        strPrototypeTag += StringToWString(j["Name"].get<std::string>());
    }

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
        VIBufferDesc.ePType = static_cast<PARTICLETYPE>(j["PType"].get<_int>());

    if (j.contains("Loop"))
        VIBufferDesc.isLoop = j["Loop"].get<_bool>();

    if (j.contains("Spin"))
        VIBufferDesc.bSpin = j["Spin"].get<_bool>();

    if (j.contains("Orbit"))
        VIBufferDesc.bOrbit = j["Orbit"].get<_bool>();

    if (j.contains("RotationAxis") && j["RotationAxis"].is_array() && j["RotationAxis"].size() == 3)
        VIBufferDesc.vRotationAxis = { j["RotationAxis"][0].get<_float>(), j["RotationAxis"][1].get<_float>(), j["RotationAxis"][2].get<_float>() };

    if (j.contains("OrbitAxis") && j["OrbitAxis"].is_array() && j["OrbitAxis"].size() == 3)
        VIBufferDesc.vOrbitAxis = { j["OrbitAxis"][0].get<_float>(), j["OrbitAxis"][1].get<_float>(), j["OrbitAxis"][2].get<_float>() };

    if (j.contains("RotationSpeed") && j["RotationSpeed"].is_array() && j["RotationSpeed"].size() == 2)
        VIBufferDesc.vRotationSpeed = { j["RotationSpeed"][0].get<_float>(), j["RotationSpeed"][1].get<_float>() };

    if (j.contains("OrbitSpeed") && j["OrbitSpeed"].is_array() && j["OrbitSpeed"].size() == 2)
        VIBufferDesc.vOrbitSpeed = { j["OrbitSpeed"][0].get<_float>(), j["OrbitSpeed"][1].get<_float>() };

    if (j.contains("Accel") && j["Accel"].is_array() && j["Accel"].size() == 2)
        VIBufferDesc.vAccel = { j["Accel"][0].get<_float>(), j["Accel"][1].get<_float>() };

    if (j.contains("MaxSpeed"))
        VIBufferDesc.fMaxSpeed = j["MaxSpeed"].get<_float>();

    if (j.contains("MinSpeed"))
        VIBufferDesc.fMinSpeed = j["MinSpeed"].get<_float>();

    if (j.contains("IsGravity"))
        VIBufferDesc.bGravity = j["IsGravity"].get<_bool>();
    if (j.contains("Gravity"))
        VIBufferDesc.fGravity = j["Gravity"].get<_float>();

    //if (j.contains("Local"))
    //    VIBufferDesc.bLocal = j["Local"].get<_bool>();

    // UV Grid
    if (j.contains("isTileLoop"))
        VIBufferDesc.isTileLoop = j["isTileLoop"].get<_bool>();
    else
        VIBufferDesc.isTileLoop = false;
    if (j.contains("TileX"))
        VIBufferDesc.vTileCnt.x = (_float)(j["TileX"].get<_int>());
    else
        VIBufferDesc.vTileCnt.x = 1.f;
    if (j.contains("TileY"))
        VIBufferDesc.vTileCnt.y = (_float)(j["TileY"].get<_int>());
    else
        VIBufferDesc.vTileCnt.y = 1.f;

    if (j.contains("TileTickPerSec"))
        VIBufferDesc.fTileTickPerSec = j["TileTickPerSec"].get<_float>();


    if (j.contains("ShrinkThreshold"))
        VIBufferDesc.fShrinkThreshold = j["ShrinkThreshold"].get<_float>();

    if (j.contains("IsCircleRange"))
        VIBufferDesc.isCircleRange = j["IsCircleRange"].get<_bool>();

    if (j.contains("CircleNormal") && j["CircleNormal"].is_array() && j["CircleNormal"].size() == 3)
        VIBufferDesc.vCircleNormal = { j["CircleNormal"][0].get<_float>(), j["CircleNormal"][1].get<_float>(), j["CircleNormal"][2].get<_float>() };

    if (j.contains("LoopInSet"))
        VIBufferDesc.bLoopInSet = j["LoopInSet"].get<_bool>();
    if (j.contains("LoopInSet_Delay"))
        VIBufferDesc.fLoopInSet_LoopDelay = j["LoopInSet_Delay"].get<_float>();


    VIBufferDesc.isTool = false;

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), strPrototypeTag,
        CVIBuffer_Point_Instance::Create(m_pDevice, m_pContext, &VIBufferDesc))))
        return E_FAIL;


    return S_OK;
}

HRESULT CEffect_Manager::Ready_Prototype_Trail_VIBuffers(const json& j)
{
    CVIBuffer_SwordTrail::DESC VIBufferDesc = {};
    _wstring strPrototypeTag = TEXT("Prototype_Component_VIBuffer_");

    if (j.contains("Name"))
    {
        strPrototypeTag += StringToWString(j["Name"].get<std::string>());
    }

    if (j.contains("LifeDuration"))
        VIBufferDesc.fLifeDuration = j["LifeDuration"].get<_float>();

    if (j.contains("NodeInterval"))
        VIBufferDesc.fNodeInterval = j["NodeInterval"].get<_float>();

    if (j.contains("Subdivisions"))
        VIBufferDesc.Subdivisions = j["Subdivisions"].get<_int>();

    if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::STATIC), strPrototypeTag,
        CVIBuffer_SwordTrail::Create(m_pDevice, m_pContext, &VIBufferDesc))))
        return E_FAIL;

    return S_OK;
}

void CEffect_Manager::Free()
{
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    Safe_Release(m_pGameInstance);

    for (auto& pEC : m_ECs)
    {
        Safe_Release(pEC.second);
    }
    m_ECs.clear();
}

