#pragma once
#include "YWTool.h"

#include "Client_Defines.h"

NS_BEGIN(Engine)
class CNavigation;
NS_END

NS_BEGIN(Client)

class CNavTool final : public CYWTool
{
public:
	typedef struct tagNavToolDesc : public CYWTool::GAMEOBJECT_DESC
	{
		wstring wsMapName = { L"" }; //맵 이름
	}NAVTOOL_DESC;

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
	virtual HRESULT Load(const _char* Map) override;
	virtual HRESULT Save(const _char* Map) override;

private:
	//툴 조작
	void Control(_float fTimeDelta);

private:
	void Render_CellList();
	void Render_Settings();
	void Render_Detail();

private:
	//포인트 추가
	void Add_Point();
	
	//포인트들을 시계방향으로 정렬
	void Make_Clockwise(_float3* Points);

private:
	_float m_fSnapThreshold = { 0.5f }; //스냅 거리

private:
	//삼각형 만들기 위한 포인트
	vector<_float3> m_Points;

private:
	_bool m_bUseDSS = { true };


private:
	CNavigation* m_pNavigationCom = { nullptr };

private:
	HRESULT Ready_Components(void* pArg);

public:
	static CNavTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END