#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Client)

class CTriggerTalk : public CTriggerBox
{
public:
	typedef struct tagTriggerTalkDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		string gameObjectTag;
	}TRIGGERTALK_DESC;

protected:
	CTriggerTalk(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerTalk(const CTriggerTalk& Prototype);
	virtual ~CTriggerTalk() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
private:
	void Play_Sound();
	HRESULT Ready_TriggerObject(void* pArg);
private:
	CGameObject* m_pTriggerObject = { nullptr };
	_bool m_bTalkActive = false;
	_bool m_bAutoTalk = true;
public:
	static CTriggerTalk* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END