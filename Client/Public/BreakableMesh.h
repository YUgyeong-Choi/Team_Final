#pragma once

#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBreakableMesh : public CDynamicMesh
{
public:
	typedef struct tagBreakableMeshDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		//�� �޽�
		//wstring ModelName = {};

		//��Ʈ �޽� ����
		_uint iPartModelCount = { 0 };
		//��Ʈ �޽� �̸���
		vector<wstring> PartModelNames = {};
	}BREAKABLEMESH_DESC;

protected:
	CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBreakableMesh(const CBreakableMesh& Prototype);
	virtual ~CBreakableMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Render_Model();
	HRESULT Render_PartModels();

private:
	//��Ʈ �� ����
	_uint m_iPartModelCount = 0;

private:    /* [ ������Ʈ ] */
	vector<CModel*> m_pPartModelComs = {};

	//CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider();

public:
	static CBreakableMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END