#pragma once

#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXDynamicActor;
class CPhysXStaticActor;
class CSoundController;
NS_END

NS_BEGIN(Client)

class CSlideDoor : public CDynamicMesh
{
public:
	typedef struct tagDoorMeshDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		INTERACT_TYPE eInteractType;
		_vector vTriggerOffset;
		_vector vTriggerSize;
	}DOORMESH_DESC;
protected:
	CSlideDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSlideDoor(const CSlideDoor& Prototype);
	virtual ~CSlideDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	void Play_Sound();
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger(DOORMESH_DESC* pDesc);
private:
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };
	INTERACT_TYPE m_eInteractType;

	_bool m_bCanActive = false;
	_bool m_bFinish = false;
public:
	static CSlideDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END