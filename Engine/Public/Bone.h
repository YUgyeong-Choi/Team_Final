#pragma once

#include "Base.h"

/* ��. (aiNode, aiBone, aiNodeAnim) */
/* aiNode�� �̿��ؼ� ������ �� ����. */

NS_BEGIN(Engine)

class CBone final : public CBase
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

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Initialize( ifstream& ifs);
	//void Update_TransformationMatrix(_fmatrix TransformationMatrix);
	void Update_CombinedTransformationMatrix(const vector<CBone*>& Bones, _fmatrix PreTransformMatrix);
	_bool Compare_Name(const _char* pName) {
		return !strcmp(m_szName, pName);
	}

private:
	_char					m_szName[MAX_PATH] = {};

	/* (A : �� �� ��ü�� �������� ��ȯ������ ǥ���� ���) */
	_float4x4				m_TransformationMatrix = {};

	/* (A : �� �� ��ü�� �������� ��ȯ������ ǥ���� ���) * �θ����. = A: �θ� �������� ȸ���Ѵ�. */
	_float4x4				m_CombinedTransformationMatrix = {};	

	_int					m_iParentBoneIndex = { -1 };

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	static CBone* Create( ifstream& ifs);
	CBone* Clone();
	virtual void Free() override;

};

NS_END