#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Client)

class CTriggerNoMesh : public CTriggerBox
{
public:
	typedef struct tagTriggerNoMeshDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		

	}TRIGGERNOMESH_DESC;

protected:
	CTriggerNoMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTriggerNoMesh(const CTriggerNoMesh& Prototype);
	virtual ~CTriggerNoMesh() = default;

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
public:
	static CTriggerNoMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END