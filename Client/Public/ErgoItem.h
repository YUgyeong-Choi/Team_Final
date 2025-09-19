#pragma once

#include "Client_Defines.h"
#include "Item.h"


// �ʿ��ϸ� �߰�, ��Ʈ�� �޷��ִ°� ǥ���ؾ� �ɵ�?
NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END


NS_BEGIN(Client)

class CErgoItem final : public CItem
{
public:
	typedef struct tagErgoItemDesc : public CItem::GAMEOBJECT_DESC
	{
		ITEM_TAG eItemTag = { ITEM_TAG::END };
	}ERGOITEM_DESC;

private:
	CErgoItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CErgoItem(const CErgoItem& Prototype);
	virtual ~CErgoItem() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual void Use(){};

	_bool Check_Player_Close();

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;


	HRESULT Bind_ShaderResources();
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Effect(void* pArg);
	HRESULT Ready_Collider();

	ITEM_TAG m_eItemTag = { ITEM_TAG::END };

	CGameObject* m_pTarget = {nullptr};

	_bool		m_bDoOnce = { false };

	//�޽� �����ΰ�?
	_bool		m_bPulseCell = { false };
	//_float3		m_vOriginPos = {};
	//_float		m_fAccTime = { 0.f };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

public:
	static CErgoItem* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END