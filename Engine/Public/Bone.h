#pragma once

#include "Base.h"

/* ��. (aiNode, aiBone, aiNodeAnim) */
/* aiNode�� �̿��ؼ� ������ �� ����. */

NS_BEGIN(Engine)

class ENGINE_DLL CBone final : public CBase
{
private:
	CBone();
	virtual ~CBone() = default;

public:
	const _float4x4* Get_CombinedTransformationMatrix() const {
		return &m_CombinedTransformationMatrix;
	}
	const _float4x4* Get_TransformationMatrix() const {
		return &m_TransformationMatrix;
	}

	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}

	const _float4x4& Get_LocalBindPose() const { return m_LocalBindPoseMatrix; }

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Initialize( ifstream& ifs);
	//void Update_TransformationMatrix(_fmatrix TransformationMatrix);
	void Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix);
	_bool Compare_Name(const _char* pName) {
		return !strcmp(m_szName, pName);
	}

	_int Get_ParentBoneIndex() const {
		return m_iParentBoneIndex;
	}

	void Set_BoneIndex(_int iBoneIndex) {
		m_iBoneIndex = iBoneIndex;
	}
	_int Get_BoneIndex() const {
		return m_iBoneIndex;
	}
	const _char* Get_Name() {
		return m_szName;
	}

private:
	_char					m_szName[MAX_PATH] = {};

	/* (A : �� �� ��ü�� �������� ��ȯ������ ǥ���� ���) */
	_float4x4				m_TransformationMatrix = {};

	/* (A : �� �� ��ü�� �������� ��ȯ������ ǥ���� ���) * �θ����. = A: �θ� �������� ȸ���Ѵ�. */
	_float4x4				m_CombinedTransformationMatrix = {};	

	_int					m_iParentBoneIndex = { -1 };
	_int					m_iBoneIndex = { -1 }; // �� ���� �ε���. (�𵨿����� �ε���)

	_float4x4  m_LocalBindPoseMatrix = {};

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	static CBone* Create( ifstream& ifs);
	CBone* Clone();
	virtual void Free() override;

};

NS_END