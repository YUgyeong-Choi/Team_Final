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
class CAnimController;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBossRetryDoor : public CDynamicMesh
{
public:
	typedef struct tagRetryDoorDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		INTERACT_TYPE eInteractType;
		_vector vTriggerOffset;
		_vector vTriggerSize;
	}RETRYDOOR_DESC;
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

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);


public:
	void Play_Sound(_float fTimeDelta);

	void OpenDoor();

	void Move_Player(_float fTimeDelta);


protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger(RETRYDOOR_DESC* pDesc);

	HRESULT LoadFromJson();
	HRESULT LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom);
	HRESULT LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator);
private:
	class CPlayer* m_pPlayer = { nullptr };
	class CAnimator* m_pAnimator = { nullptr };
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };
	INTERACT_TYPE m_eInteractType;

	_bool m_bCanActive = false;
	_bool m_bFinish = false;

	_bool m_bMoveStart = false;
	_bool m_bStartCutScene = false;
	_bool m_bRotationStart = false;


	// 사운드 관련
	_bool m_bStartSound = false;
	_float m_fSoundDelta = {};
public:
	static CBossRetryDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END