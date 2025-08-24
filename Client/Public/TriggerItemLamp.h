#pragma once

// Ä¸½¶°ú Ä¸½¶ Ãæµ¹ 

#include "Client_Defines.h"
#include "TriggerItem.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CTriggerItemLamp : public CTriggerItem
{
private:
	CTriggerItemLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerItemLamp(const CTriggerItemLamp& Prototype);
	virtual ~CTriggerItemLamp() = default;
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
private:
	HRESULT Ready_Components();
	HRESULT Bind_Shader();
private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
public:
	static CTriggerItemLamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END