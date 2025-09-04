#pragma once
/* [ 야외 맵으로 갈 때 key 필요한 문들 ] */
#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXDynamicActor;
class CPhysXStaticActor;
class CSoundController;
class CAnimController;
NS_END

NS_BEGIN(Client)

class CKeyDoor : public CDynamicMesh
{
public:
	typedef struct tagKeyDoorMeshDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		INTERACT_TYPE eInteractType;
		_vector vTriggerOffset;
		_vector vTriggerSize;
	}KEYDOORMESH_DESC;
protected:
	CKeyDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKeyDoor(const CKeyDoor& Prototype);
	virtual ~CKeyDoor() = default;

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

	void OpenDoor();

	void Move_Player(_float fTimeDelta);
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger(KEYDOORMESH_DESC* pDesc);

	HRESULT LoadFromJson();
	HRESULT LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom);
	HRESULT LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator);
private:
	class CPlayer* m_pPlayer = { nullptr };
	CAnimator* m_pAnimator = { nullptr };
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };
	INTERACT_TYPE m_eInteractType;

	_bool m_bCanActive = false;
	_bool m_bFinish = false;

	_bool m_bMoveStart = false;
	_bool m_bRotationStart = false;
	_bool m_bStartCutScene = false;
public:
	static CKeyDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END