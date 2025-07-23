#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI

#include "Static_UI.h"

NS_BEGIN(Client)
  
class CGLTool final : public CGameObject
{
private:
	CGLTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGLTool(const CGLTool& Prototype);
	virtual ~CGLTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Save_File();
	void Open_File();
	void Add_UI_From_File();

	void Add_UI();

private:
	HRESULT Render_UtilTool();
	HRESULT Render_SelectOptionTool();

	HRESULT Render_UIList();

private:
	//
	list<_wstring> m_TextureNames = {};

	_int m_iSelectTextureIndex = {-1};
	_int m_iSelectObjIndex = { -1 };
	wstring m_strSelectName = {};

	// 바로 값 적용되는거를 막기 위해, 입력은 temp로 받아두고, apply 버튼을 누르면 eUIDesc를 바꿔서 실제로 적용 되도록
	CStatic_UI::STATIC_UI_DESC eUIDesc = {};
	CStatic_UI::STATIC_UI_DESC eUITempDesc = {};

	//
	list<CStatic_UI*> m_UIList = {};
	CStatic_UI* m_pSelectObj = { nullptr };

	string  m_strSavePath = {};
	
public:
	static CGLTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END