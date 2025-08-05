

#pragma once

#include "Client_Defines.h"
#include "Item.h"



NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END


NS_BEGIN(Client)

class CPortion final : public CItem
{
private:
	CPortion(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPortion(const CPortion& Prototype);
	virtual ~CPortion() = default;

public:


public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	// ��� ȿ�� ��ӹ޾Ƽ� �����ϱ�
	virtual void Activate() override;

	virtual ITEM_DESC Get_ItemDesc() override;

	HRESULT Ready_Components();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
private:
	//
	_int	m_iMaxCount = { 3 };
	



public:
	static CPortion* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END