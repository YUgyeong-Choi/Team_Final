#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Engine)
class CSound_Core;
NS_END

NS_BEGIN(Client)

class CTriggerBGM : public CTriggerBox
{
public:
	typedef struct tagTriggerBGMDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		string strInBGM;
		string strOutBGM;
	}TRIGGERBGM_DESC;

protected:
	CTriggerBGM(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerBGM(const CTriggerBGM& Prototype);
	virtual ~CTriggerBGM() = default;

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
	_bool m_bInSound = false;

	string m_strInBGM;
	string m_strOutBGM;
public:
	static CTriggerBGM* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END