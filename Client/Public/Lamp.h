#pragma once

#include "Client_Defines.h"
#include "Item.h"
#include "Light.h"


// �ʿ��ϸ� �߰�, ��Ʈ�� �޷��ִ°� ǥ���ؾ� �ɵ�?
NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END


NS_BEGIN(Client)

class CLamp final : public CItem
{
private:
	CLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLamp(const CLamp& Prototype);
	virtual ~CLamp() = default;

public:


public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	// ��� ȿ�� ��ӹ޾Ƽ� �����ϱ�
	
	virtual void Use() override;
	virtual ITEM_DESC Get_ItemDesc() override;

	

	HRESULT Ready_Components();
	HRESULT Ready_Light();

	HRESULT Bind_ShaderResources();

private:
	//



public:
	static CLamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END