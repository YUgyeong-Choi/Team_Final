#pragma once

#include "VIBuffer_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CMesh_Instance final : public CVIBuffer_Instance
{
public:
	typedef struct tagMesh_MeshInstanceDesc : public INSTANCE_DESC 
	{
		//월드행렬들 전달
		vector<_float4x4>* pInstanceMatrixs = { nullptr };
	}MESHINSTANCE_DESC;
	
private:
	CMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh_Instance(const CMesh_Instance& Prototype);
	virtual ~CMesh_Instance() = default;

public:
	_uint Get_MaterialIndex() const {
		return m_iMaterialIndex;
	}

public:
	//virtual HRESULT Initialize_Prototype(MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize_Prototype(MODEL eType, ifstream& ifs, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);

private:
	_char			m_szName[MAX_PATH] = {};
	_uint			m_iMaterialIndex = {};

	VTXMESH_INSTANCE* m_pVertexInstances = { nullptr };

private:
	HRESULT Ready_NonAnim_Mesh(const aiMesh* pAIMesh, _fmatrix PreTransformMatrix);
	HRESULT Ready_NonAnim_Mesh(ifstream& ifs, _fmatrix PreTransformMatrix);

public:
	//static CMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const aiMesh* pAIMesh, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix);
	static CMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, ifstream& ifs, const vector<class CBone*>& Bones, _fmatrix PreTransformMatrix);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END