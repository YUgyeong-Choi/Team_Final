#pragma once

#include "Client_Defines.h"
#include "Item.h"
#include "Light.h"


// 필요하면 추가, 벨트에 달려있는거 표현해야 될듯?
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

	// 사용 효과 상속받아서 구현하기
	
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