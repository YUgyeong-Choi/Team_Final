#pragma once
#include "Client_Defines.h"
#include "DefaultDoor.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXDynamicActor;
class CPhysXStaticActor;
class CSoundController;
class CAnimController;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBossRetryDoor : public CDefaultDoor
{
protected:
	CBossRetryDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossRetryDoor(const CBossRetryDoor& Prototype);
	virtual ~CBossRetryDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Reset() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);
public:
	void Move_Player(_float fTimeDelta);
protected:
	HRESULT Ready_Components(void* pArg);
private:
	_bool m_bWalkFront = false;
	_bool m_bEnd = false;
public:
	static CBossRetryDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END