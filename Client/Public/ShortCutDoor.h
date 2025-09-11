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
class CAnimator;
NS_END

NS_BEGIN(Client)

class CShortCutDoor : public CDynamicMesh
{
public:
	typedef struct tagShortCutDoorMeshDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		INTERACT_TYPE eInteractType;
		_vector vTriggerOffset;
		_vector vTriggerSize;
	}SHORTCUTDOORMESH_DESC;
protected:
	CShortCutDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CShortCutDoor(const CShortCutDoor& Prototype);
	virtual ~CShortCutDoor() = default;

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
	HRESULT Ready_Trigger(SHORTCUTDOORMESH_DESC* pDesc);

	HRESULT LoadFromJson();
	HRESULT LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom);
	HRESULT LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator);
private:
	HRESULT Render_Key();
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

	_bool m_bCanOpen = false;

	// 자물쇠
	class CModel* m_pModelComFrontKey = { nullptr };
	class CAnimator* m_pAnimatorFrontKey = { nullptr };

	class CModel* m_pModelComBackKey = { nullptr };
	class CAnimator* m_pAnimatorBackKey = { nullptr };
public:
	static CShortCutDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END