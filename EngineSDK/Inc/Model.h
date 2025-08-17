#pragma once

#include "Serializable.h"
#include "Component.h"
#include "Mesh.h"

NS_BEGIN(Engine)

class ENGINE_DLL CModel : public CComponent, public ISerializable
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

	_uint Get_Mesh_NumVertices(_int iMeshIndex);
	_uint Get_Mesh_NumIndices(_int iMeshIndex);
	const _float3* Get_Mesh_pVertices(_int iMeshIndex);
	const _uint* Get_Mesh_pIndices(_int iMeshIndex);

	void Set_Animation(_uint iIndex, _bool isLoop = true);
	void Set_Animation_TickPerSecond(_uint iIndex, _float fTickPerSecond);
	void Set_Animation_TickPerSecond_All(_float fTickPerSecond);
	void Reset_CurAnimationFrame();

	const vector<class CBone*>& Get_Bones() const {
		return m_Bones;
	}
	_uint Get_NumAnimations() const { return m_iNumAnimations; }
	vector<class CAnimation*>& GetAnimations() { return m_Animations; }

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
#ifdef USE_IMGUI
	unordered_map<string, _uint>& GetAnimationsByName()  { return m_AnimationMap; }
	unordered_map<_uint, string>& GetAnimationsByIndex() { return m_AnimationNameMap; }
#endif // USE_IMGUI


	const string& Get_ModelName() const
	{
		return m_ModelName;
	}
	const class CMesh* Get_Mesh(_uint iMeshIndex)
	{
		if (iMeshIndex >= m_iNumMeshes)
			return nullptr;
		return m_Meshes[iMeshIndex];
	}
public:
	virtual HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex = 0);
	HRESULT Bind_Bone_Matrices(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex);
	HRESULT Bind_SkinningSRVs(class CShader* pShader, _uint iMeshIndex)
	{
		return m_Meshes[iMeshIndex]->Bind_SkinningSRVs(pShader);
	}
	const auto& Get_PreTransformMatrix() const { return m_PreTransformMatrix; }

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

public:
	HRESULT Update_Bones();

protected:
	void MakeBoneChildrenMap();


protected:
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
	string 						 m_ModelName;
	unordered_map<string, vector<_int>> m_BoneChildrenMap; // 뼈 이름과 자식 뼈 인덱스 목록


public:
	vector<CMesh*>* Get_Meshes() { return &m_Meshes; };

public: // 어심프로 읽던 방식(근데이거왜퍼블릭임)
	HRESULT Read_OriginalFBX(const string& filepath);
	HRESULT Ready_Bones(const aiNode* pAINode, _int iParentBoneIndex);
	HRESULT Ready_Meshes();
	HRESULT Ready_Materials(const _char* pModelFilePath);
	HRESULT Ready_Animations();

protected: // 바이너리 읽는 방식
	virtual HRESULT Read_BinaryFBX(const string& filepath);
	HRESULT Ready_Bones(ifstream& ifs);
	virtual HRESULT Ready_Meshes(ifstream& ifs);
	HRESULT Ready_Materials( ifstream& ifs, const _char* pModelFilePath);
	HRESULT Ready_Animations(ifstream& ifs);

	void SubStrModelName(const string& filepath)
	{
		size_t pos = filepath.find_last_of("/\\");
		if (pos != string::npos)
		{
			m_ModelName = filepath.substr(pos + 1);
			m_ModelName = m_ModelName.substr(0, m_ModelName.find_last_of('.')); // 확장자 제거
		}
		else
			m_ModelName = filepath;
	}

public:
	HRESULT Add_Animations(const string& filepath);

public: /* bone */
	_uint Find_BoneIndex(const _char* srcName);
	const _float4x4* Get_CombinedTransformationMatrix(_uint iBoneIndex);
	const _float4x4* Get_TransformationMatrix(_uint iBoneIndex);
	HRESULT Set_BoneMatrix(_uint iBoneIndex, _fmatrix matTransform);
	const vector<_int> GetBoneChildren(const string& bonName) const 
	{
		auto it = m_BoneChildrenMap.find(bonName);
		if (it != m_BoneChildrenMap.end())
			return it->second;
		return {};
	}
public:
	const _float Get_CurrentTrackPosition();

public:
	static CModel* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

	// ISerializable을(를) 통해 상속됨
	json Serialize() override;
	void Deserialize(const json& j) override;
};

NS_END