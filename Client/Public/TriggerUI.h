#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Client)

class CTriggerUI : public CTriggerBox
{
public:
	typedef struct tagTriggerUIDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		

		// 만들 ui 프로토타입 이름
		_wstring strProtoName;
		// 필요한 경로들. 여러개 필요한 것들도 있어서
		vector<_wstring> strFilePaths;
		

	}TRIGGERUI_DESC;
protected:
	CTriggerUI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerUI(const CTriggerUI& Prototype);
	virtual ~CTriggerUI() = default;

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
	
	_bool	m_isFinish = {};

	_wstring m_strProtoName;
	vector<_wstring> m_strFilePaths;
public:
	static CTriggerUI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END