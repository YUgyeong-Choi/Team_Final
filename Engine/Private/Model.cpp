#include "Component.h"

#include "Animation.h"
#include "Material.h"
#include "Model.h"
#include "Mesh.h"
#include "Bone.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{

}

CModel::CModel(const CModel& Prototype)
	: CComponent { Prototype }	
	, m_iNumMeshes { Prototype.m_iNumMeshes }
	, m_Meshes { Prototype.m_Meshes }
	, m_iNumMaterials { Prototype.m_iNumMaterials }
	, m_Materials { Prototype.m_Materials }
	, m_eType { Prototype.m_eType }
	, m_PreTransformMatrix { Prototype.m_PreTransformMatrix }
	//, m_Bones { Prototype.m_Bones }
	, m_iNumAnimations { Prototype.m_iNumAnimations }
	// , m_Animations { Prototype.m_Animations }
{
	for (auto& pPrototypeBone : Prototype.m_Bones)
		m_Bones.push_back(pPrototypeBone->Clone());		

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);

	for (auto& pPrototypeAnim: Prototype.m_Animations)
		m_Animations.push_back(pPrototypeAnim->Clone());

}

const _float4x4* CModel::Get_BoneMatrix(const _char* pBoneName) const
{	
	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
	{
			if (true == pBone->Compare_Name(pBoneName))
				return true;
			return false;		
	});

	if (iter == m_Bones.end())
		return nullptr;

	return (*iter)->Get_CombinedTransformationMatrix();	
	
}

HRESULT CModel::Bind_Material(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;		

	_uint		iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();

	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	return m_Materials[iMaterialIndex]->Bind_ShaderResource(pShader, pConstantName, eType, iTextureIndex);	
}

HRESULT CModel::Bind_Bone_Matrices(CShader* pShader, const _char* pConstantName, _uint iMeshIndex)
{
	return m_Meshes[iMeshIndex]->Bind_Bone_Matrices(pShader, pConstantName, m_Bones);	
}

HRESULT CModel::Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{	

	_uint		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;
	
	if (MODEL::NONANIM == eType)
		iFlag |= aiProcess_PreTransformVertices;

	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
		return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	m_eType = eType;

	if (FAILED(Ready_Bones(m_pAIScene->mRootNode, -1)))
		return E_FAIL;

	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	if (FAILED(Ready_Materials(pModelFilePath)))
		return E_FAIL;

	/* �� �ִϸ��̼� ���� �̿��ϰ� �ִ� ������� �ð��� �´� ���°����� �̸� �о �����صд�. */
	if (FAILED(Ready_Animations()))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{	
	m_Meshes[iMeshIndex]->Bind_Buffers();
	m_Meshes[iMeshIndex]->Render();	

	return S_OK;
}

_bool CModel::Play_Animation(_float fTimeDelta)
{
	_bool		isFinished = { false };
	/* 1. ���� �ִϸ��̼ǿ� �´� ���� ���¸� �о�ͼ� ���� TrnasformationMatrix�� �������ش�. */
	isFinished  = m_Animations[m_iCurrentAnimIndex]->Update_Bones(fTimeDelta, m_Bones, m_isLoop);

	/* 2. ��ü ���� ��ȸ�ϸ鼭 ������ ColmbinedTransformationMatixf�� �θ𿡼����� �ڽ����� �������ֳ�. */
	for (auto& pBone : m_Bones)
	{
		pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
	}

	/*XMMatrixDecompose()*/

	return isFinished;
}



HRESULT CModel::Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex)
{	
	CBone* pBone = CBone::Create(pAINode, iParentBoneIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);	

	_int		iParentIndex = m_Bones.size() - 1;

	for (size_t i = 0; i < pAINode->mNumChildren; i++)
	{
		Ready_Bones(pAINode->mChildren[i], iParentIndex);
	}
	return S_OK;
}

HRESULT CModel::Ready_Meshes()
{
	m_iNumMeshes = m_pAIScene->mNumMeshes;

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, m_pAIScene->mMeshes[i], m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(const _char* pModelFilePath)
{
	m_iNumMaterials = m_pAIScene->mNumMaterials;

	for (size_t i = 0; i < m_iNumMaterials; i++)
	{
		CMaterial* pMaterial = CMaterial::Create(m_pDevice, m_pContext, pModelFilePath, m_pAIScene->mMaterials[i]);
		if (nullptr == pMaterial)
			return E_FAIL;

		m_Materials.push_back(pMaterial);
	}
	return S_OK;
}

HRESULT CModel::Ready_Animations()
{
	m_iNumAnimations = m_pAIScene->mNumAnimations;

	for (size_t i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}

	return S_OK;
}

CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pInstance = new CModel(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);
	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);	
	m_Meshes.clear();

	for (auto& pAnimation : m_Animations)
		Safe_Release(pAnimation);
	m_Animations.clear();

	m_Importer.FreeScene();
}
