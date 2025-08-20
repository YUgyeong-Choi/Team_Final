#pragma once

#include "Client_Defines.h"
#include "TriggerBox.h"

NS_BEGIN(Client)

class CTriggerNoMesh : public CTriggerBox
{
public:
	typedef struct tagStaticTriggerNoMeshDesc : public CTriggerBox::TRIGGERBOX_DESC
	{
		

	}STATICTRIGGERNOMESH_DESC;

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

public:
	static CTriggerNoMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END