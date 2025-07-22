#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(const aiNode* pAINode, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pAINode->mName.data);

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(_float4x4));

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));	

	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	m_iParentBoneIndex = iParentBoneIndex;

	return S_OK;
}

HRESULT CBone::Initialize( ifstream& ifs)
{
	_uint NameLength = {};
	ifs.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));					// ª¿ ¿Ã∏ß ±Ê¿Ã 
	ifs.read(reinterpret_cast<char*>(m_szName), NameLength);						// ª¿ ¿Ã∏ß
	ifs.read(reinterpret_cast<char*>(&m_TransformationMatrix), sizeof(_float4x4));	// «‡∑ƒ 
	ifs.read(reinterpret_cast<char*>(&m_iParentBoneIndex), sizeof(_int));			// ∫Œ∏ ª¿ ¿Œµ¶Ω∫ 
	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	if (-1 == m_iParentBoneIndex)
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformMatrix);

	else
	{
		m_CombinedTransformationMatrix;
		m_TransformationMatrix;
		Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix;

		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[m_iParentBoneIndex]->m_CombinedTransformationMatrix));
	}
}


CBone* CBone::Create(const aiNode* pAINode, _int iParentBoneIndex)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(pAINode, iParentBoneIndex)))
	{
		MSG_BOX("Failed to Created : CBone");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CBone* CBone::Create( ifstream& ifs)
{
	CBone* pInstance = new CBone();

	if (FAILED(pInstance->Initialize(ifs)))
	{
		MSG_BOX("Failed to Created : CBone");
		Safe_Release(pInstance);
	}

	return pInstance;
}


CBone* CBone::Clone()
{
	return new CBone(*this);
}

void CBone::Free()
{
	__super::Free();


}
