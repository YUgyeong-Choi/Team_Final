#include "Bone.h"

CBone::CBone()
{
}

HRESULT CBone::Initialize(const aiNode* pAINode, _int iParentBoneIndex)
{
	strcpy_s(m_szName, pAINode->mName.data);

	memcpy(&m_TransformationMatrix, &pAINode->mTransformation, sizeof(_float4x4));

	XMStoreFloat4x4(&m_TransformationMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_TransformationMatrix)));	

	m_LocalBindPoseMatrix = m_TransformationMatrix; // ���� ��� ���� ���߿� ���忡 ����ϱ�

	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity());

	m_iParentBoneIndex = iParentBoneIndex;
	m_iPrevParentBoneIndex = iParentBoneIndex; // ���� �θ� �� �ε��� �ʱ�ȭ
	return S_OK;
}

HRESULT CBone::Initialize( ifstream& ifs)
{
	_uint NameLength = {};
	ifs.read(reinterpret_cast<char*>(&NameLength), sizeof(_uint));					// �� �̸� ���� 
	ifs.read(reinterpret_cast<char*>(m_szName), NameLength);						// �� �̸�
	ifs.read(reinterpret_cast<char*>(&m_TransformationMatrix), sizeof(_float4x4));	// ��� 
	ifs.read(reinterpret_cast<char*>(&m_iParentBoneIndex), sizeof(_int));			// �θ� �� �ε��� 
	this->m_LocalBindPoseMatrix = m_TransformationMatrix; // ���� ��� ���� ���߿� ���忡 ����ϱ�
	XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMMatrixIdentity()); // �ʱ�ȭ
	
	if (!strcmp(m_szName, "Root"))
	{
  		m_bIsRootBone = true;
	}
	else if (!strcmp(m_szName, "Bip001-Pelvis"))
	{
 		m_bIsPelvisBone = true;
	}
	else
	{
		m_bIsRootBone = false;
		m_bIsPelvisBone = false;
	}

	return S_OK;
}

void CBone::Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix)
{
	_int iOriginalParentBoneIndex = m_iParentBoneIndex; // ���� �θ� �� �ε��� ����
	_int iUseParentBoneIndex = (m_bApplyRootMotion && m_iParentBoneIndex == 1) ? -1 : iOriginalParentBoneIndex; // Ʈ�������̼� ���� �θ�� -1�� ����
	m_iUseParentIndex = iUseParentBoneIndex; // ���� �θ� �� �ε��� ����
	if (-1 == iUseParentBoneIndex)
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, XMLoadFloat4x4(&m_TransformationMatrix) * PreTransformMatrix);
	}

	else
	{
		XMStoreFloat4x4(&m_CombinedTransformationMatrix,
			XMLoadFloat4x4(&m_TransformationMatrix) * XMLoadFloat4x4(&Bones[iUseParentBoneIndex]->m_CombinedTransformationMatrix));
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
