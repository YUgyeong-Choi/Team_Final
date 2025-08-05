#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

NS_BEGIN(Client)

class CYWTool abstract : public CGameObject
{
protected:
	CYWTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CYWTool(const CYWTool& Prototype);
	virtual ~CYWTool() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render() override;
	virtual HRESULT	Render_ImGui();

public:
	virtual void Free() override;

};

NS_END