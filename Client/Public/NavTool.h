#pragma once
#include "YWTool.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CNavigation;
NS_END

NS_BEGIN(Client)

class CNavTool final : public CYWTool
{
private:
	CNavTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavTool(const CNavTool& Prototype);
	virtual ~CNavTool() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;
	virtual HRESULT	Render_ImGui() override;

private:
	//툴 조작
	void Control(_float fTimeDelta);

private:
	void Render_CellList();

private:
	void Make_Clockwise(_float3* Points);

private:
	//삼각형 만들기 위한 포인트
	vector<_float3> m_Points;

private:
	CNavigation* m_pNavigationCom = { nullptr };

private:
	HRESULT Ready_Components();

public:
	static CNavTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END