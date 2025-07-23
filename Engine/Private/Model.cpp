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
	: CComponent(Prototype)
	, m_iNumMeshes{ Prototype.m_iNumMeshes }
	, m_Meshes{ Prototype.m_Meshes }
	, m_iNumMaterials{ Prototype.m_iNumMaterials }
	, m_Materials{ Prototype.m_Materials }
	, m_eType{ Prototype.m_eType }
	, m_PreTransformMatrix{ Prototype.m_PreTransformMatrix }
	//, m_Bones { Prototype.m_Bones }
	, m_iNumAnimations{ Prototype.m_iNumAnimations }
	// , m_Animations { Prototype.m_Animations }
{
	for (auto& pPrototypeBone : Prototype.m_Bones)
		m_Bones.push_back(pPrototypeBone->Clone());

	for (auto& pMaterial : m_Materials)
		Safe_AddRef(pMaterial);

	for (auto& pMesh : m_Meshes)
		Safe_AddRef(pMesh);
	for (auto& pPrototypeAnim : Prototype.m_Animations)
		m_Animations.push_back(pPrototypeAnim->Clone(m_Bones));

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

	//m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	//if (nullptr == m_pAIScene)
	//	return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	m_eType = eType;

	// 뼈 머태리얼 메쉬 애니메이션 읽고 생성
	Read_BinaryFBX(pModelFilePath);

	//Read_OriginalFBX(pModelFilePath);

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

HRESULT CModel::Play_Animation(_float fTimeDelta)
{
	//_bool		isFinished = { false };
 //
	//isFinished = m_Animations[m_iCurrentAnimIndex]->Update_Bones(fTimeDelta, m_Bones, m_isLoop);

	for (auto& pBone : m_Bones)
	{
		pBone->Update_CombinedTransformationMatrix(m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
	}


	return S_OK;
}

void CModel::Set_Animation(_uint iIndex, _bool isLoop)
{
	if (iIndex >= m_Animations.size())
		return;

	m_iCurrentAnimIndex = iIndex;
	m_isLoop = isLoop;
}

void CModel::Set_Animation_TickPerSecond(_uint iIndex, _float fTickPerSecond)
{
	if (iIndex >= m_iNumAnimations)
		return;
	m_Animations[iIndex]->SetTickPerSecond(fTickPerSecond);
}

void CModel::Set_Animation_TickPerSecond_All(_float fTickPerSecond)
{
	for (auto& pAnim : m_Animations)
		pAnim->SetTickPerSecond(fTickPerSecond);
}

void CModel::Reset_CurAnimationFrame()
{
	m_Animations[m_iCurrentAnimIndex]->ResetTrack();
}

HRESULT CModel::Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex)
{
	CBone* pBone = CBone::Create(pAINode, iParentBoneIndex);
	if (nullptr == pBone)
		return E_FAIL;

	m_Bones.push_back(pBone);

	_int		iParentIndex = (_int)m_Bones.size() - 1;

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

HRESULT CModel::Read_BinaryFBX(const string& filepath)
{
	ifstream ifs(filepath, ios::binary);
	if (!ifs.is_open()) {
		MSG_BOX("너는 파일 열기도 못하는구나");
		return E_FAIL;
	}

	if (m_eType == MODEL::ANIM)
	{
		if (FAILED(Ready_Bones(ifs)))
			return E_FAIL;
	}

	if (FAILED(Ready_Meshes(ifs)))
		return E_FAIL;

	if (FAILED(Ready_Materials(ifs, filepath.c_str())))
		return E_FAIL;

	if (m_eType == MODEL::ANIM)
	{
		//if (FAILED(Ready_Animations(ifs)))
		//	return E_FAIL;

		if (FAILED(Add_Animations(filepath)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CModel::Ready_Bones(ifstream& ifs)
{
	_uint BonesSize = {};
	ifs.read(reinterpret_cast<_char*>(&BonesSize), sizeof(_uint));
	m_Bones.reserve(BonesSize);
	for (size_t i = 0; i < BonesSize; i++)
	{
		CBone* pBone = CBone::Create(ifs);
		if (nullptr == pBone)
			return E_FAIL;
		m_Bones.push_back(pBone);
	}
	return S_OK;
}

HRESULT CModel::Ready_Meshes(ifstream& ifs)
{
	ifs.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));  // 메쉬 몇개읨 

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh* pMesh = CMesh::Create(m_pDevice, m_pContext, m_eType, ifs, m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel::Ready_Materials(ifstream& ifs, const _char* pModelFilePath)
{
	ifs.read(reinterpret_cast<_char*>(&m_iNumMaterials), sizeof(_uint));  // 머테리얼 몇개읨 

	for (size_t i = 0; i < m_iNumMaterials; i++)
	{
		CMaterial* pMaterial = CMaterial::Create(m_pDevice, m_pContext, pModelFilePath, ifs);
		if (nullptr == pMaterial)
			return E_FAIL;

		m_Materials.push_back(pMaterial);
	}
	return S_OK;
}

HRESULT CModel::Ready_Animations(ifstream& ifs)
{
	ifs.read(reinterpret_cast<_char*>(&m_iNumAnimations), sizeof(_uint));  // 애니메이션 몇개읨 

	_uint iRootBoneIdx = Find_BoneIndex("Root_$AssimpFbx$_Translation");
	if (iRootBoneIdx == m_Bones.size())
	{
		iRootBoneIdx = Find_BoneIndex("Root");
	}

	for (size_t i = 0; i < m_iNumAnimations; i++)
	{
		CAnimation* pAnimation = CAnimation::CreateByBinary(ifs, m_Bones);
		if (nullptr == pAnimation)
			return E_FAIL;

		m_Animations.push_back(pAnimation);
	}


	return S_OK;
}

HRESULT CModel::Add_Animations(const string& filepath)
{

	string full = filepath;
	size_t pos = full.find_last_of("\\/");
	string dir = (pos != string::npos) ? full.substr(0, pos + 1) : "./";

	//  검색 패턴 (예: "C:/Models/*.anim.bin")
	string pattern = dir + "*.anim";

	WIN32_FIND_DATAA findData;
	HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// 찾은 파일 이름(build.cFileName)과 디렉터리 결합
			string animPath = dir + findData.cFileName;

			// 파일 열기
			ifstream ifs(animPath, ios::binary);
			if (!ifs.is_open())
				continue;
			_uint iAdditionalNumAnimations = {};
			ifs.read(reinterpret_cast<_char*>(&iAdditionalNumAnimations), sizeof(_uint));
			// CAnimation 생성 (bones는 이미 채워져 있음)
			CAnimation* pAnim = CAnimation::CreateByBinary(ifs, this->m_Bones);
			if (pAnim)
			{
				this->m_Animations.push_back(pAnim);
				this->m_AnimationMap[pAnim->Get_Name()] =
					static_cast<uint32_t>(this->m_Animations.size() - 1);
				this->m_AnimationNameMap[static_cast<uint32_t>(this->m_Animations.size() - 1)] = pAnim->Get_Name();
			}

		} while (FindNextFileA(hFind, &findData));  // 다음 파일 검색

		FindClose(hFind);
	}

	// 총 개수 업데이트
	this->m_iNumAnimations = static_cast<uint32_t>(this->m_Animations.size());
	return S_OK;
}

_uint CModel::Find_BoneIndex(const _char* srcName)
{
	_uint iBoneIndex = {};
	auto	iter = find_if(m_Bones.begin(), m_Bones.end(), [&](CBone* pBone)->_bool
		{
			if (true == pBone->Compare_Name(srcName))
				return true;

			++iBoneIndex;

			return false;
		});
	return iBoneIndex;
}

const _float4x4* CModel::Get_CombinedTransformationMatrix(_uint iBoneIndex)
{
	return m_Bones[iBoneIndex]->Get_CombinedTransformationMatrix();
}

const _float4x4* CModel::Get_TransformationMatrix(_uint iBoneIndex)
{
	return m_Bones[iBoneIndex]->Get_TransformationMatrix();
}

HRESULT CModel::Set_BoneMatrix(_uint iBoneIndex, _fmatrix matTransform)
{
	if (iBoneIndex > m_Bones.size())
		return E_FAIL;
	m_Bones[iBoneIndex]->Set_TransformationMatrix(matTransform);
	return S_OK;
}

const _float CModel::Get_CurrentTrackPosition()
{
	return 	m_Animations[m_iCurrentAnimIndex]->GetCurrentTrackPosition();
}

HRESULT CModel::Read_OriginalFBX(const string& filepath)
{
	if (FAILED(Ready_Bones(m_pAIScene->mRootNode, -1)))
		return E_FAIL;

	if (FAILED(Ready_Meshes()))
		return E_FAIL;

	if (FAILED(Ready_Materials(filepath.c_str())))
		return E_FAIL;

	/* 각 애니메이션 마다 이용하고 있는 뼈대들의 시간에 맞는 상태값들을 미리 읽어서 저장해둔다. */
	if (FAILED(Ready_Animations()))
		return E_FAIL;
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

	for (auto& pBone : m_Bones)
		Safe_Release(pBone);
	m_Bones.clear();


	m_Importer.FreeScene();
}
