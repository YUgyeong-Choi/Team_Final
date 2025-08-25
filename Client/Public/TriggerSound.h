#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Engine)
class CSoundController;
NS_END

NS_BEGIN(Client)

class CTriggerSound : public CTriggerBox
{
public:
	typedef struct tagTriggerNoMeshDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		TRIGGERSOUND_TYPE eTriggerBoxType;

	}TRIGGERNOMESH_DESC;

protected:
	CTriggerSound(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerSound(const CTriggerSound& Prototype);
	virtual ~CTriggerSound() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerStay(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType) override;
private:
	HRESULT Ready_Components();
private:
	CSoundController* m_pSoundCom = { nullptr };
	TRIGGERSOUND_TYPE m_eTriggerSoundType;
public:
	static CTriggerSound* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END