//#pragma once
//
//#include "Client_Defines.h"
//#include "DynamicMesh.h"
//
//NS_BEGIN(Engine)
//class CModel;
//class CShader;
//class CAnimator;
//NS_END
//
//NS_BEGIN(Client)
//
//class CBreakableMesh : public CDynamicMesh
//{
//
//protected:
//	CBreakableMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
//	CBreakableMesh(const CBreakableMesh& Prototype);
//	virtual ~CBreakableMesh() = default;
//
//public:
//	virtual HRESULT Initialize_Prototype() override;
//	virtual HRESULT Initialize(void* pArg) override;
//	virtual void Priority_Update(_float fTimeDelta) override;
//	virtual void Update(_float fTimeDelta) override;
//	virtual void Late_Update(_float fTimeDelta) override;
//	virtual HRESULT Render() override;
//
//
//private:    /* [ ÄÄÆ÷³ÍÆ® ] */
//	CModel* m_pModelCom = { nullptr };
//	CShader* m_pShaderCom = { nullptr };
//
//protected:
//	HRESULT Ready_Components(void* pArg);
//	HRESULT Bind_ShaderResources();
//	HRESULT Ready_Collider();
//
//public:
//	static CBreakableMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
//	virtual CGameObject* Clone(void* pArg) override;
//	virtual void Free() override;
//};
//
//NS_END