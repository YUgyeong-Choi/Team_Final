#pragma once
#include "YWTool.h"

#include "Client_Defines.h"

NS_BEGIN(Client)

class CDecalTool final : public CYWTool
{
private:
	CDecalTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecalTool(const CDecalTool& Prototype);
	virtual ~CDecalTool() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;
	virtual HRESULT	Render_ImGui() override;

private:
	//Åø Á¶ÀÛ
	void Control(_float fTimeDelta);

private:
	HRESULT Spawn_DecalObject();
	HRESULT Save();

private:
	void Render_Detail();
	void Detail_Transform();

private:
	ImGuizmo::OPERATION m_currentOperation = { ImGuizmo::TRANSLATE };

private:
	class CDecalToolObject* m_pFocusObject = { nullptr };


public:
	static CDecalTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END