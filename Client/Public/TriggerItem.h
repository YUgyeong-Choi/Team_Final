#pragma once

// Ä¸½¶°ú Ä¸½¶ Ãæµ¹ 

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CTriggerItem : public CGameObject
{
public:
	typedef struct tagTriggerItemDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4x4 triggerWorldMatrix;
		_float4 vOffSetObj;
		_float4 vScaleObj;

		// »ý¼ºÇÒ ¾ÆÀÌÅÛ Å¸ÀÔ
		// ÀÌ¸§
		

	}TRIGGERITEM_DESC;
protected:
	CTriggerItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerItem(const CTriggerItem& Prototype);
	virtual ~CTriggerItem() = default;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
public:
	static CTriggerItem* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END