#pragma once

#include "Component.h"
#include "Mesh.h"

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
	_uint Get_NumBones() const {
		return _uint(m_Bones.size());
	}
	_uint Get_CurAnimationIndex() const {
		return m_iCurrentAnimIndex;
	}

	void Set_Animation(_uint iIndex, _bool isLoop = true);
	void Set_Animation_TickPerSecond(_uint iIndex, _float fTickPerSecond);
	void Set_Animation_TickPerSecond_All(_float fTickPerSecond);
	void Reset_CurAnimationFrame();

	const vector<class CBone*>& Get_Bones() const {
		return m_Bones;
	}
	_uint Get_NumAnimations() const { return m_iNumAnimations; }
	vector<class CAnimation*> GetAnimations()& { return m_Animations; }

	class CAnimation* GetAnimationClip(_uint iIndex)
	{
		if (iIndex >= m_iNumAnimations)
			return nullptr;
		return m_Animations[iIndex];
	}
	class CAnimation* GetAnimationClipByName(const string& name)
	{
		auto it = m_AnimationMap.find(name);
		if (it != m_AnimationMap.end())
			return m_Animations[it->second];
		return nullptr;
	}

public:
	HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex = 0);
	HRESULT Bind_Bone_Matrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

public:
	HRESULT Play_Animation(_float fTimeDelta);

private:
	Assimp::Importer			m_Importer;	

	/* 모델에 대한 모든 정보를 담고 있는 구조체. */
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


	unordered_map<string, _uint> m_AnimationMap;
	unordered_map<_uint, string> m_AnimationNameMap;

public:
	vector<CMesh*>* Get_Meshes() { return &m_Meshes; };

public: // 어심프로 읽던 방식(근데이거왜퍼블릭임)
	HRESULT Read_OriginalFBX(const string& filepath);
	HRESULT Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Animations();

private: // 바이너리 읽는 방식
	HRESULT Read_BinaryFBX(const string& filepath);
	HRESULT Ready_Bones(ifstream& ifs);
	HRESULT Ready_Meshes(ifstream& ifs);
	HRESULT Ready_Materials( ifstream& ifs, const _char* pModelFilePath);
	HRESULT Ready_Animations(ifstream& ifs);

public:
	HRESULT Add_Animations(const string& filepath);

public: /* bone */
	_uint Find_BoneIndex(const _char* srcName);
	const _float4x4* Get_CombinedTransformationMatrix(_uint iBoneIndex);
	const _float4x4* Get_TransformationMatrix(_uint iBoneIndex);
	HRESULT Set_BoneMatrix(_uint iBoneIndex, _fmatrix matTransform);

public:
	const _float Get_CurrentTrackPosition();
	const _float Get_Duration();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END