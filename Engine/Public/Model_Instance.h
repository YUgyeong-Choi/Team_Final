#pragma once

#include "Serializable.h"
#include "Component.h"
#include "Mesh.h"

NS_BEGIN(Engine)

class ENGINE_DLL CModel_Instance final : public CComponent
{
protected:
	CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel_Instance(const CModel_Instance& Prototype);
	virtual ~CModel_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

private:
	_float4x4					m_PreTransformMatrix = {};
	_uint						m_iNumMeshes = {};
	vector<class CMesh_Instance*>		m_Meshes;

private:
	_uint						m_iNumMaterials = {};
	vector<class CMaterial*>	m_Materials;

private:
	MODEL						m_eType = {};

private:
	HRESULT Read_BinaryFBX(const string& filepath);
	HRESULT Ready_Meshes(ifstream& ifs);
	HRESULT Ready_Materials(ifstream& ifs, const _char* pModelFilePath);

public:
	static CModel_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END