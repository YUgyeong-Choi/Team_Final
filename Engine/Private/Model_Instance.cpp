#include "Component.h"

#include "Material.h"
#include "Model_Instance.h"
#include "Mesh_Instance.h"

CModel_Instance::CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CModel{ pDevice, pContext }
{

}

CModel_Instance::CModel_Instance(const CModel_Instance& Prototype)
	: CModel(Prototype)
	, m_Meshes{ Prototype.m_Meshes }
{


}

HRESULT CModel_Instance::Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	_uint		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

	if (MODEL::NONANIM == eType)
		iFlag |= aiProcess_PreTransformVertices;

	//m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	//if (nullptr == m_pAIScene)
	//	return E_FAIL;

	XMStoreFloat4x4(&m_PreTransformMatrix, PreTransformMatrix);

	m_eType = eType;

	// �� ���¸��� �޽� �ִϸ��̼� �а� ����
	Read_BinaryFBX(pModelFilePath);

	//Read_OriginalFBX(pModelFilePath);

	return S_OK;
}

HRESULT CModel_Instance::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CModel_Instance::Render(_uint iMeshIndex)
{
	m_Meshes[iMeshIndex]->Bind_Buffers();
	m_Meshes[iMeshIndex]->Render();

	return S_OK;
}

HRESULT CModel_Instance::Bind_Material(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex)
{
	if (iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	_uint		iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();

	if (iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	return m_Materials[iMaterialIndex]->Bind_ShaderResource(pShader, pConstantName, eType, iTextureIndex);
}

HRESULT CModel_Instance::Ready_Meshes(ifstream& ifs)
{
	ifs.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));  // �޽� ��� 

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh_Instance* pMesh = CMesh_Instance::Create(m_pDevice, m_pContext, m_eType, ifs, m_Bones, XMLoadFloat4x4(&m_PreTransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel_Instance::Read_BinaryFBX(const string& filepath)
{
	ifstream ifs(filepath, ios::binary);
	if (!ifs.is_open()) {
		MSG_BOX("���� ���� ����.");
		return E_FAIL;
	}

	SubStrModelName(filepath);
	if (m_eType == MODEL::ANIM)
	{
		if (FAILED(Ready_Bones(ifs)))
			return E_FAIL;
	}

	if (FAILED(CModel_Instance::Ready_Meshes(ifs)))
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

CModel_Instance* CModel_Instance::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix)
{
	CModel_Instance* pInstance = new CModel_Instance(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, pModelFilePath, PreTransformMatrix)))
	{
		MSG_BOX("Failed to Created : CModel_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CModel_Instance::Clone(void* pArg)
{
	CModel_Instance* pInstance = new CModel_Instance(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel_Instance");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CModel_Instance::Free()
{
	__super::Free();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();
}
