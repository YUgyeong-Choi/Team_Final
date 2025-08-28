#pragma once

#include "Client_Defines.h"
#include "StaticMesh.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel_Instance;
NS_END

NS_BEGIN(Client)

class CStaticMesh_Instance final : public CStaticMesh
{
public:
	typedef struct tagStaticMeshInstanceDesc : public CStaticMesh::STATICMESH_DESC
	{
		//�ν��Ͻ� ����
		_uint iNumInstance = { 0 };

		//���
		vector<_float4x4>* pInstanceMatrixs = { nullptr };

		//����Ʈ ���
		_int iLightShape = { 0 };

		//��� ���ߴ°�(STATION? HOTEL?...)
		wstring wsMap = {};
	}STATICMESHINSTANCE_DESC;

private:
	CStaticMesh_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CStaticMesh_Instance(const CStaticMesh_Instance& Prototype);
	virtual ~CStaticMesh_Instance() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	_bool IsInAnyArea(const vector<_uint>& vecActiveAreaIds, const vector<_uint>& vecCheckAreaIds);

private:
	HRESULT SetEmissive();

private:
	wstring m_wsMap = {}; //��� ���ߴ°�(STATION? HOTEL?...)

private:
	CModel_Instance* m_pModelCom = { nullptr };
	_uint	m_iNumInstance = { 0 };
private:
	_int m_iLightShape = { 0 };
private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();

public:
	static CStaticMesh_Instance* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END