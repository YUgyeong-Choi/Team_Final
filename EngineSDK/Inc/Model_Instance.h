#pragma once

#include "Serializable.h"
#include "Model.h"
#include "Mesh_Instance.h"

NS_BEGIN(Engine)

class ENGINE_DLL CModel_Instance final : public CModel
{
protected:
	CModel_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CModel_Instance(const CModel_Instance& Prototype, void* pArg);
	virtual ~CModel_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype(MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix);
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Render(_uint iMeshIndex);

public:
	vector<CMesh_Instance*>* Get_Meshes() {
		return &m_Meshes; 
	};

private:
	vector<CMesh_Instance*>		m_Meshes;

public:
	virtual HRESULT Bind_Material(class CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eType, _uint iTextureIndex = 0) override;

private:
	virtual HRESULT Ready_Meshes(ifstream& ifs) override;
	virtual HRESULT Read_BinaryFBX(const string& filepath) override;
public:
	static CModel_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, MODEL eType, const _char* pModelFilePath, _fmatrix PreTransformMatrix = XMMatrixIdentity());
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END