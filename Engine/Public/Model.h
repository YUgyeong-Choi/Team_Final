#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CModel final : public CComponent
{
protected:
	CModel(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CModel(const CModel& Prototype);
	virtual ~CModel() = default;

public:
	_uint Get_NumMeshes() const {
		return m_iNumMeshes;
	}

	const _float4x4* Get_BoneMatrix(const _char* pBoneName) const;

	void Set_Animation(_uint iIndex, _bool isLoop = true) {
		m_iCurrentAnimIndex = iIndex;
		m_isLoop = isLoop;
	}

public:

	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex = 0);
	HRESULT Bind_Bone_Matrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

public:
	_bool Play_Animation(_float fTimeDelta);

private:
	Assimp::Importer			m_Importer;	

	/* �𵨿� ���� ��� ������,�� ��� �ִ� ����ü. */
	const aiScene*				m_pAIScene = { nullptr };

	MODEL						m_eType = {};
	_float4x4					m_PreTransformMatrix = {};
	_uint						m_iNumMeshes = {};
	vector<class CMesh*>		m_Meshes;

	_uint						m_iNumMaterials = {};
	vector<class CMaterial*>	m_Materials;

	vector<class CBone*>		m_Bones;

	_bool						m_isLoop{};
	_uint						m_iCurrentAnimIndex = { };
	_uint						m_iNumAnimations = {};
	vector<class CAnimation*>	m_Animations;	

public:
	HRESULT Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Animations();
public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END