#pragma once

#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

NS_BEGIN(Client)

class CImGuiTool abstract : public CGameObject
{
protected:
	CImGuiTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CImGuiTool(const CImGuiTool& Prototype);
	virtual ~CImGuiTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

protected:
	IMGUIWINDATA* m_pWindowData = { nullptr };
public:
	virtual CGameObject* Clone(void* pArg) =0;
	virtual void Free() override;

};

NS_END