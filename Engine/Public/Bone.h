#pragma once

#include "Base.h"

/* 뼈. (aiNode, aiBone, aiNodeAnim) */
/* aiNode를 이용해서 셋팅한 뼈 정보. */

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
	_float4x4& Get_TransformationMatrix_Float4x4() {
		return m_TransformationMatrix;
	}
	void Set_TransformationMatrix(_fmatrix TransformationMatrix) {
		XMStoreFloat4x4(&m_TransformationMatrix, TransformationMatrix);
	}
	void Set_CombinedTransformationMatrix(_fmatrix CombinedTransformationMatrix) {
		XMStoreFloat4x4(&m_CombinedTransformationMatrix, CombinedTransformationMatrix);
	}

	const _float4x4& Get_LocalBindPose() const { return m_LocalBindPoseMatrix; }

public:
	HRESULT Initialize(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Initialize( ifstream& ifs);
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

	_bool Is_RootBone() const {
		return m_bIsRootBone;
	}

	_bool Is_PelvisBone() const {
		return m_bIsPelvisBone;
	}

	void SetApplyRootMotion(_bool bApply) {
		m_bApplyRootMotion = bApply;
	}

	_int Get_UseParentIndex() const {
		return m_iUseParentIndex;
	}

	void Set_ParentBoneIndex(_int iParentIndex) {
		m_iParentBoneIndex = iParentIndex;
	}

private:
	_char					m_szName[MAX_PATH] = {};

	/* (A : 이 뼈 자체의 원점기준 변환정보를 표현한 행렬) */
	_float4x4				m_TransformationMatrix = {};
	_bool m_bIsRootBone = false;
	_bool m_bIsPelvisBone = false; // Pelvis 뼈인지 여부

	/* (A : 이 뼈 자체의 원점기준 변환정보를 표현한 행렬) * 부모행렬. = A: 부모를 기준으로 회전한다. */
	_float4x4				m_CombinedTransformationMatrix = {};	

	_int					m_iParentBoneIndex = { -1 };
	_int	                m_iPrevParentBoneIndex = { -1 }; // 이전 부모 뼈 인덱스.
	_int					m_iBoneIndex = { -1 }; // 이 뼈의 인덱스. (모델에서의 인덱스)
	_int                    m_iUseParentIndex = { -1 };
	_bool                   m_bApplyRootMotion = true;

	_float4x4  m_LocalBindPoseMatrix = {};

public:
	static CBone* Create(const aiNode* pAINode, _int iParentBoneIndex);
	static CBone* Create( ifstream& ifs);
	CBone* Clone();
	virtual void Free() override;

};

NS_END