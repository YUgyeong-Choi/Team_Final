#pragma once

#include "Client_Defines.h"
#include "MeshEffect.h"

NS_BEGIN(Client)

class CToolMeshEffect final : public CMeshEffect
{
private:
	CToolMeshEffect(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToolMeshEffect(const CToolMeshEffect& Prototype);
	virtual ~CToolMeshEffect() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

public:
	static CToolMeshEffect* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END