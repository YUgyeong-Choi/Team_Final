#include "AnimatedProp.h"
#include "GameInstance.h"
#include "Player.h"
#include "Animator.h"
CAnimatedProp::CAnimatedProp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CAnimatedProp::CAnimatedProp(const CAnimatedProp& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CAnimatedProp::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CAnimatedProp::Initialize(void* pArg)
{
	CAnimatedProp::ANIMTEDPROP_DESC* pDesc = static_cast<ANIMTEDPROP_DESC*>(pArg);

	m_bUseSecondMesh = pDesc->bUseSecondMesh;
	m_szMeshID = pDesc->szMeshID;
	m_eMeshLevelID = pDesc->eMeshLevelID;
	m_szSecondMeshID = pDesc->szSecondMeshID;
	m_bCullNone = pDesc->bCullNone;
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(pDesc->WorldMatrix);


	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (m_bUseSecondMesh && m_pSecondTransformCom)
		m_pSecondTransformCom->Set_WorldMatrix(pDesc->vSecondWorldMatrix);

	if (FAILED(LoadFromJson()))
		return E_FAIL;

	if (m_bUseSecondMesh)
	{
		m_pAnimator->SetPlaying(false);
		m_pSecondAnimator->SetPlaying(false);
	}
	Register_Events();
	return S_OK;
}

void CAnimatedProp::Priority_Update(_float fTimeDelta)
{
	
}

void CAnimatedProp::Update(_float fTimeDelta)
{
	__super::Update(fTimeDelta);

	if (m_pAnimator)
		m_pAnimator->Update(fTimeDelta);

	if (m_pModelCom)
		m_pModelCom->Update_Bones();

	if (m_bUseSecondMesh == false)
		return;
	if(m_pSecondAnimator)
		m_pSecondAnimator->Update(fTimeDelta);
	if (m_pSecondModelCom)
		m_pSecondModelCom->Update_Bones();
}

void CAnimatedProp::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_PBRMESH, this);
}

HRESULT CAnimatedProp::Bind_ShaderResources(_bool bSecondModel)
{
	CTransform* pTransform = bSecondModel ? m_pSecondTransformCom : m_pTransformCom;
	if (FAILED(pTransform->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance->Get_Transform_Float4x4(D3DTS::PROJ))))
		return E_FAIL;

	return S_OK;
}
HRESULT CAnimatedProp::Render()
{
	if (FAILED(Bind_ShaderResources(false)))
		return E_FAIL;
	_int iPass = m_bCullNone ? 9 : 0;
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;
				m_bDoOnce = true;
			}

			if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
				return E_FAIL;
		}


		_float m_fEmissive = 0.f;
		m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float));
		if (FAILED(m_pModelCom->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);
	
		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;

		if (FAILED(m_pModelCom->Render(i)))
			return E_FAIL;
	}

	if (m_bUseSecondMesh == false)
		return S_OK;

	if (FAILED(Bind_ShaderResources(true)))
		return E_FAIL;
	iNumMesh = m_pSecondModelCom->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		m_pSecondModelCom->Bind_Material(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0);
		m_pSecondModelCom->Bind_Material(m_pShaderCom, "g_NormalTexture", i, aiTextureType_NORMALS, 0);

		if (FAILED(m_pSecondModelCom->Bind_Material(m_pShaderCom, "g_ARMTexture", i, aiTextureType_SPECULAR, 0)))
		{
			if (!m_bDoOnce)
			{
				/* Com_Texture */
				if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Texture_DefaultARM")),
					TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
					return E_FAIL;
				m_bDoOnce = true;
			}

			if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_ARMTexture", 0)))
				return E_FAIL;
		}


		_float m_fEmissive = 0.f;
		m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float));
		if (FAILED(m_pSecondModelCom->Bind_Material(m_pShaderCom, "g_Emissive", i, aiTextureType_EMISSIVE, 0)))
		{
			_float fEmissive = 0.f;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &fEmissive, sizeof(_float))))
				return E_FAIL;
		}

		m_pSecondModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);


		if (FAILED(m_pShaderCom->Begin(iPass)))
			return E_FAIL;

		if (FAILED(m_pSecondModelCom->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}


void CAnimatedProp::Register_Events()
{
	if (m_bUseSecondMesh)
	{
		m_pAnimator->RegisterEventListener("StartAnim",[this]()
			{
				if (m_pSecondAnimator)
					m_pSecondAnimator->SetPlaying(true);
			});

	}
}


HRESULT CAnimatedProp::Ready_Components()
{
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), _wstring(TEXT("Prototype_Component_Model_")) + m_szMeshID,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;


	m_pAnimator = CAnimator::Create(m_pDevice, m_pContext);
	if (nullptr == m_pAnimator)
		return E_FAIL;
	if (FAILED(m_pAnimator->Initialize(m_pModelCom)))
		return E_FAIL;

	if (m_bUseSecondMesh)
	{
		if (FAILED(__super::Add_Component(ENUM_CLASS(m_eMeshLevelID), _wstring(TEXT("Prototype_Component_Model_")) + m_szSecondMeshID,
			TEXT("Com_Model2"), reinterpret_cast<CComponent**>(&m_pSecondModelCom))))
			return E_FAIL;

		m_pSecondAnimator = CAnimator::Create(m_pDevice, m_pContext);
		if (nullptr == m_pSecondAnimator)
			return E_FAIL;
		if (FAILED(m_pSecondAnimator->Initialize(m_pSecondModelCom)))
			return E_FAIL;
		m_pSecondTransformCom = CTransform::Create(m_pDevice, m_pContext);
		if (nullptr == m_pSecondTransformCom)
			return E_FAIL;

		CTransform::TRANSFORM_DESC Desc{};
		Desc.fRotationPerSec = XMConvertToRadians(0.f);
		Desc.fSpeedPerSec = 0.f;
		if (FAILED(m_pSecondTransformCom->Initialize(&Desc)))
			return E_FAIL;
	}

	return S_OK;
}


HRESULT CAnimatedProp::LoadFromJson()
{
	string modelName = m_pModelCom->Get_ModelName();
	if (FAILED(LoadAnimationEventsFromJson(modelName, m_pModelCom)))
		return E_FAIL;
	if (FAILED(LoadAnimationStatesFromJson(modelName,m_pAnimator)))
		return E_FAIL;
	if (m_pSecondModelCom && m_pSecondAnimator)
	{
		string modelName2 = m_pSecondModelCom->Get_ModelName();
		if (FAILED(LoadAnimationEventsFromJson(modelName2, m_pSecondModelCom)))
			return E_FAIL;
		if (FAILED(LoadAnimationStatesFromJson(modelName2, m_pSecondAnimator)))
			return E_FAIL;
	}
	return S_OK;
}

HRESULT CAnimatedProp::LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom)
{
	string path = "../Bin/Save/AnimationEvents/" + modelName + "_events.json";
	ifstream ifs(path);
	if (ifs.is_open())
	{
		json root;
		ifs >> root;
		if (root.contains("animations"))
		{
			auto& animationsJson = root["animations"];
			auto& clonedAnims = pModelCom->GetAnimations();

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
	else
	{
		MSG_BOX("Failed to open animation events file.");
		return E_FAIL;
	}
	return S_OK;
}

HRESULT CAnimatedProp::LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator)
{
	string path = "../Bin/Save/AnimationStates/" + modelName + "_States.json";
	ifstream ifsStates(path);
	if (ifsStates.is_open())
	{
		json rootStates;
		ifsStates >> rootStates;
		pAnimator->Deserialize(rootStates);
	}
	else
	{
		MSG_BOX("Failed to open animation states file.");
		return E_FAIL;
	}
	return S_OK;
}


CAnimatedProp* CAnimatedProp::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CAnimatedProp* pInstance = new CAnimatedProp(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CAnimatedProp");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CAnimatedProp::Clone(void* pArg)
{
	CAnimatedProp* pInstance = new CAnimatedProp(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CAnimatedProp");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CAnimatedProp::Free()
{
	__super::Free();
	Safe_Release(m_pAnimator);
	Safe_Release(m_pSecondAnimator);

	Safe_Release(m_pModelCom);
	Safe_Release(m_pSecondModelCom);

	Safe_Release(m_pTextureCom);
	Safe_Release(m_pShaderCom);

	Safe_Release(m_pSecondTransformCom);
}
