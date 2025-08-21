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
		_bool bCanCancel;
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

	/* [ 대화 자동으로 넘어가기 ] */
	_bool m_bAutoTalk = true;

	/* [ 대화 중간에 종료 가능한가 ] */
	_bool m_bCanCancel = false;

	// 무기 선택 중 일때 사운드 update 안함
	_bool m_bActive = true;
public:
	static CTriggerTalk* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END