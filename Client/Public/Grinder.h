
#pragma once

#include "Client_Defines.h"
#include "Item.h"


// �ʿ��ϸ� �߰�, ��Ʈ�� �޷��ִ°� ǥ���ؾ� �ɵ�?
NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END


NS_BEGIN(Client)

class CGrinder final : public CItem
{
private:
	CGrinder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGrinder(const CGrinder& Prototype);
	virtual ~CGrinder() = default;

public:
	

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	// ��� ȿ�� ��ӹ޾Ƽ� �����ϱ�
	virtual void Activate();

	virtual ITEM_DESC Get_ItemDesc() override;

	HRESULT Ready_Components();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
private:
	//
	

public:
	static CGrinder* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END