#include "Component.h"

#include "Material.h"
#include "Model_Instance.h"
#include "Mesh_Instance.h"

CModel_Instance::CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent{ pDevice, pContext }
{

}

CModel_Instance::CModel_Instance(const CModel_Instance& Prototype)
	: CComponent(Prototype)
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

	// 뼈 머태리얼 메쉬 애니메이션 읽고 생성
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

HRESULT CModel_Instance::Read_BinaryFBX(const string& filepath)
{
	ifstream ifs(filepath, ios::binary);
	if (!ifs.is_open()) {
		MSG_BOX("파일 열기 실패.");
		return E_FAIL;
	}

	if (FAILED(Ready_Meshes(ifs)))
		return E_FAIL;

	if (FAILED(Ready_Materials(ifs, filepath.c_str())))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel_Instance::Ready_Meshes(ifstream& ifs)
{
	ifs.read(reinterpret_cast<_char*>(&m_iNumMeshes), sizeof(_uint));  // 메쉬 몇개읨 

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh_Instance* pMesh = CMesh_Instance::Create(m_pDevice, m_pContext, m_eType, ifs, XMLoadFloat4x4(&m_PreTransformMatrix));
		if (nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}

	return S_OK;
}

HRESULT CModel_Instance::Ready_Materials(ifstream& ifs, const _char* pModelFilePath)
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

	for (auto& pMaterial : m_Materials)
		Safe_Release(pMaterial);
	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
		Safe_Release(pMesh);
	m_Meshes.clear();

}