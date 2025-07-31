#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Sequence.h"
#include "UI_Text.h"
#include "UI_Container.h"


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



private:
	
	void Obj_Serialize();
	void Obj_Deserialize();

	
	void Open_File();


	void Add_Static_UI();
	void Add_Dynamic_UI();
	void Add_UI_Text();


	// dynamicui가 가지고 있는 시퀀스로 바꿔서 보여주기
	void Add_Sequence_To_DynamicUI();

	// 시퀀스를 ui에 적용하기
	void Apply_Sequence_To_DynamicUI();

	void Input_Static_Desc();
	void Input_Dynamic_Desc();
	void Input_Sequence_Desc();
	void Input_Text();

private:

	HRESULT Render_SelectOptionTool();
	HRESULT Render_UIList();
	HRESULT Render_Sequence();

private:
	//
	list<_wstring> m_TextureNames = {};
	
	_int m_iSelectTextureIndex = {-1};
	_int m_iSelectObjIndex = { -1 };
	_int m_iDynamicObjIndex = { -1 };
	_int m_iTextObjIndex = { -1 };
	wstring m_strSelectName = {};

	//  temp로 비율을 받고 실제 정보로 바꾸기
	CStatic_UI::STATIC_UI_DESC eStaticUIDesc = {};
	CStatic_UI::STATIC_UI_DESC eStaticUITempDesc = {};
	list<CStatic_UI*> m_StaticUIList = {};
	CStatic_UI* m_pSelectStaticObj = { nullptr };

	CDynamic_UI::DYNAMIC_UI_DESC eDynamicUITempDesc = {};
	CDynamic_UI::DYNAMIC_UI_DESC eDynamicUIDesc = {};
	list<CDynamic_UI*> m_DynamicUIList = {};
	CDynamic_UI* m_pSelectDynamicObj = { nullptr };

	string  m_strSavePath = {};

	// sequence
	_float m_fElapsedTime = {};
	_bool m_isPlay = { false };
	_int m_iCurrentFrame = {};
	_bool m_bExpanded = { true };
	_int m_iSelectedEntry = { -1 };
	CUI_Sequence*	m_pSequence = { nullptr };
	
	UI_FEATURE_TOOL_DESC m_eFeatureDesc = {};

	//
	_bool m_isFromTool = { true };

	string m_strInput = {};

	CUI_Text::TEXT_UI_DESC m_eTextTempUIDesc = {};
	CUI_Text::TEXT_UI_DESC m_eTextUIDesc = {};
	list<CUI_Text*> m_TextUIList = {};
	CUI_Text* m_pSelectTxtObj = { nullptr };

	
	list<CUI_Container*> m_ContainerList = {};
	CUI_Container* m_pContainerObj = {};

	
public:
	static CGLTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END