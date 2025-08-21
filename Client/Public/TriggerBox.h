#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CPhysXStaticActor;
class CSoundController;
NS_END

NS_BEGIN(Client)

class CTriggerBox : public CGameObject
{
public:
	typedef struct tagSoundDatas
	{
		string strSoundTag;
		string strSoundText;
	}SOUNDDATA;
public:
	typedef struct tagTriggerBoxDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_vector vPos;
		_float3 Rotation;
		_vector vTriggerOffset;
		_vector vTriggerSize;
		TRIGGERBOX_TYPE eTriggerBoxType;
		vector<SOUNDDATA> m_vecSoundData;
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

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) {};
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) {};
protected: /* [ 초기화 변수 ] */
	_bool			m_bDoOnce = {};
protected:
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };
	TRIGGERBOX_TYPE m_eTriggerBoxType;

	vector<SOUNDDATA> m_vecSoundData;
	_int m_iSoundIndex = -1;
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger(TRIGGERBOX_DESC* pDesc);
public:
	static CTriggerBox* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END