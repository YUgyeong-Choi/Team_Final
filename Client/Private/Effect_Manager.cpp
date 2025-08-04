#include "Effect_Manager.h"
#include "GameInstance.h"
#include "EffectContainer.h"

IMPLEMENT_SINGLETON(CEffect_Manager)


CEffect_Manager::CEffect_Manager()
    : m_pGameInstance{ CGameInstance::Get_Instance() }
{
}

HRESULT CEffect_Manager::Initialize(const _wstring EffectFilePath)
{
	//json j;
	ifstream ifs(EffectFilePath);

	if (!ifs.is_open())
    {
        MSG_BOX("이펙트 파일 경로가 잘못됨.");
        return E_FAIL;
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

CEffectContainer* CEffect_Manager::Make_EffectContainer(const _wstring strEffectTag)
{
    return nullptr;
}

void CEffect_Manager::Free()
{
}
