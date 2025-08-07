#include "Monster_Test.h"
#include "GameInstance.h"

CMonster_Test::CMonster_Test(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    :CUnit{pDevice, pContext}
{
}

CMonster_Test::CMonster_Test(const CMonster_Test& Prototype)
    :CUnit{Prototype}
{
}

HRESULT CMonster_Test::Initialize_Prototype()
{
    return S_OK;
}

HRESULT CMonster_Test::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;

    
    //m_pAnimator->SetPlaying(true);

	m_pTransformCom->Set_State(STATE::POSITION, XMVectorSetW(XMLoadFloat3(&m_InitPos), 1.f));

	LoadAnimDataFromJson();

    return S_OK;
}

void CMonster_Test::Priority_Update(_float fTimeDelta)
{
   
}

void CMonster_Test::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CMonster_Test::Late_Update(_float fTimeDelta)
{
    __super::Late_Update(fTimeDelta);

    Update_State();
}

HRESULT CMonster_Test::Render()
{

    __super::Render();

    return S_OK;
}

HRESULT CMonster_Test::Ready_Components()
{
    // 무기 장착 시키기?

    return S_OK;
}

void CMonster_Test::RootMotionActive(_float fTimeDelta)
{

}

void CMonster_Test::Update_State()
{
	if (m_pGameInstance->Key_Down(DIK_T))
	{
		m_pAnimator->SetBool("Detect", true);
	}

    if (m_pAnimator->IsFinished())
    {
       // 나중에 생각
    }
}

void CMonster_Test::LoadAnimDataFromJson()
{
	string path = "../Bin/Save/AnimationEvents/" + m_pModelCom->Get_ModelName() + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = m_pModelCom->GetAnimations();

			for (const auto& animData : animationsJson)
			{
				const string& clipName = animData["ClipName"];

				for (auto& pAnim : clonedAnims)
				{
					if (pAnim->Get_Name() == clipName)
					{
						pAnim->Deserialize(animData);
						break;
					}
				}
			}
		}
	}

	path = "../Bin/Save/AnimationStates/" + m_pModelCom->Get_ModelName() + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		m_pAnimator->Deserialize(rootStates);
	}
}

CMonster_Test* CMonster_Test::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMonster_Test* pInstance = new CMonster_Test(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}
CGameObject* CMonster_Test::Clone(void* pArg)
{
	CMonster_Test* pInstance = new CMonster_Test(*this);
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMonster_Test");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CMonster_Test::Free()
{
    __super::Free();

    
}
