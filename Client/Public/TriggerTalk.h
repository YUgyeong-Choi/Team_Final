#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Engine)
class CSoundController;
NS_END

NS_BEGIN(Client)

class CTriggerTalk : public CTriggerBox
{
public:
	typedef struct tagTriggerTalkDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		TRIGGERSOUND_TYPE eTriggerBoxType;
		string gameObjectTag;
		_vector vOffSetObj;
		_vector vScaleObj;
		_bool bCanCancel;
		
		// ������ �������� �����ϴ� ��ȭ�� ���⿡ ������ ������ �̸��� �־��ֱ�
		_int  iMakeObjectType = {};
		string strMakeObjectName = {};
	}TRIGGERTALK_DESC;

	enum BUTTON {BUTTON_NEXT, BUTTON_AUTO, BUTTON_FINISH, BUTTON_END};

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
	HRESULT Ready_Components();
	HRESULT Ready_TriggerObject(TRIGGERTALK_DESC* TriggerTalkDESC);

	void Next_Talk();

private:
	class CPlayer* m_pPlayer = { nullptr };

	CGameObject* m_pTriggerObject = { nullptr };
	_vector m_vObjOffSet = {};

	_bool m_bTalkActive = false;

	/* [ ��ȭ �ڵ����� �Ѿ�� ] */
	_bool m_bAutoTalk = true;

	/* [ ��ȭ �߰��� ���� �����Ѱ� ] */
	_bool m_bCanCancel = false;

	// ���� ���� �� �϶� ���� update ����
	_bool m_bActive = true;
private:
	CSoundController* m_pSoundCom = { nullptr };
	TRIGGERSOUND_TYPE m_eTriggerSoundType;
public:
	static CTriggerTalk* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END