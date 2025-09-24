#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

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

class CDefaultDoor : public CGameObject
{
public:
	typedef struct tagDefaultDoorhDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;

		LEVEL			m_eMeshLevelID;

		_tchar		szModelPrototypeTag[MAX_PATH] = { 0 };
		_float4 vColliderOffSet;
		_float4 vColliderSize;

		INTERACT_TYPE eInteractType;
		_float4 vTriggerSize;
		_float4 vTriggerOffset;
	}DEFAULTDOOR_DESC;

protected:
	CDefaultDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDefaultDoor(const CDefaultDoor& Prototype);
	virtual ~CDefaultDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);
	void Register_Events();
public:
	AABBBOX GetWorldAABB() const;

public:
	LEVEL Get_LevelID() const { return m_eMeshLevelID; }

protected:
	virtual void Update_ColliderPos();
protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	LEVEL			m_eMeshLevelID = { LEVEL::END };
	_bool			m_bDoOnce = {};
protected:
	// render & anim 관련
	CShader*		m_pShaderCom = { nullptr };
	CModel*			m_pModelCom = { nullptr };
	CAnimator*		m_pAnimator = { nullptr };
	CTexture*		m_pTextureCom = { nullptr };
	CTexture*		m_pEmissiveCom = { nullptr };
	_float4			m_vColliderOffSet;
	CPhysXDynamicActor* m_pPhysXActorCom = { nullptr };

	// 트리거 박스
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };

	// 이동 관련
	INTERACT_TYPE m_eInteractType;
	_bool m_bMoveStart = false;
	_bool m_bRotationStart = false;

	// 사운드 관련
	CSoundController* m_pSoundCom = { nullptr };
	_bool m_bStartSound = false;
	_float m_fSoundDelta = {};

	class CPlayer* m_pPlayer = { nullptr };

	_bool m_bCanActive = false;
	_bool m_bFinish = false;
	_bool m_bStartCutScene = false;
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Collider(void* pArg);
	HRESULT Ready_Trigger(DEFAULTDOOR_DESC* pDesc);

	HRESULT LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom);
	HRESULT LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator);

	virtual void Move_Player(_float fTimeDelta);
	virtual void Play_Sound(_float fTimeDelta);
public:
	static CDefaultDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END