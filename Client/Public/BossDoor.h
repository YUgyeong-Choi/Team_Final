#pragma once
/* [ 야외 맵으로 갈 때 key 필요한 문들 ] */
#include "Client_Defines.h"
#include "DynamicMesh.h"
#include "Player.h"
NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXStaticActor;
class CSoundController;
class CAnimController;
NS_END

NS_BEGIN(Client)

class CBossDoor : public CDynamicMesh
{
public:
	typedef struct tagBossDoorMeshDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		INTERACT_TYPE eInteractType;
		_vector vTriggerOffset;
		_vector vTriggerSize;

		_bool		bNeedSecondDoor;
		_tchar		szSecondModelPrototypeTag[MAX_PATH] = { 0 };
	}BOSSDOORMESH_DESC;
protected:
	CBossDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossDoor(const CBossDoor& Prototype);
	virtual ~CBossDoor() = default;

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
	void Play_Sound();
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger(BOSSDOORMESH_DESC* pDesc);
protected:
	HRESULT LoadFromJson();
	HRESULT LoadAnimationEventsFromJson(const string& modelName, CModel* pModelCom);
	HRESULT LoadAnimationStatesFromJson(const string& modelName, CAnimator* pAnimator);

	void Move_Player(_float fTimeDelta);
private:
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };

	// 모델 생성은 상위에서
	CAnimator* m_pAnimator = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };

	// 바뀌는 문을 위한
	CModel* m_pSecondModelCom = { nullptr };
	CAnimator* m_pSecondAnimator = { nullptr };
	
	// 두번째꺼 렌더하려면
	_bool m_bRenderSecond = false;

	// 오프셋 
	_float3 m_OffSetCollider;

	INTERACT_TYPE m_eInteractType;

	_bool m_bCanActive = false;
	_bool m_bFinish = false;

	_bool m_bMoveStart = false;
	_bool m_bRotationStart = false;
	_bool m_bStartCutScene = false;

	CPlayer* m_pPlayer = { nullptr };
public:
	static CBossDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END