#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CPhysXStaticActor;
NS_END

NS_BEGIN(Client)

class CTriggerBox : public CGameObject
{
public:
	typedef struct tagTriggerBoxDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_vector vPos;
		_float3 Rotation;
		_vector vTriggerOffset;
		_vector vTriggerSize;

	}TRIGGERBOX_DESC;

protected:
	CTriggerBox(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerBox(const CTriggerBox& Prototype);
	virtual ~CTriggerBox() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
protected: /* [ 초기화 변수 ] */
	_bool			m_bDoOnce = {};
protected:
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };

protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger();

public:
	static CTriggerBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END