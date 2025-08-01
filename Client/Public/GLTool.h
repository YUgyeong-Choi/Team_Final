#pragma once
#include "GameObject.h"

#include "Client_Defines.h"

#include "Static_UI.h"
#include "Dynamic_UI.h"
#include "UI_Sequence.h"
#include "UI_Text.h"
#include "UI_Container.h"
#include "UI_Button.h"


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

	void Add_Container();
	void Add_UI_Select_Prototype();

	void Save_Container();

	// ����� �����̳ʿ� �ø���
	void Upload_Merge_Container();
	// ���� ���õ� �����̳ʿ� ��ü ������ �ű��
	void Download_Merge_Container();
	// ��ü �����̳ʸ� ����.
	void Clear_Merge_Container();
	// ���� �����̳� �߿� ���õ� ui�� �����
	void Delete_Merge_Container_Select_Obj();

	void Set_Container_Active();
	_bool Check_Dynamic_UI();

	void Delete_Container();
	void Delete_SelectPart();


	// dynamicui�� ������ �ִ� �������� �ٲ㼭 �����ֱ�
	void Add_Sequence_To_DynamicUI();

	// �������� ui�� �����ϱ�
	void Apply_Sequence_To_DynamicUI();

	void Input_Static_Desc();
	void Input_Dynamic_Desc();
	void Input_Sequence_Desc();
	void Input_Text();
	void Input_Button_Desc();




private:

	HRESULT Render_SelectOptionTool();
	HRESULT Render_UIList();
	HRESULT Render_Sequence();

private:
	//prototype
	list<_wstring> m_ProtoNames = {};
	_wstring m_strSelectProtoName = {};
	_int m_iSelectProtoIndex = { -1 };

	//texture
	list<_wstring> m_TextureNames = {};
	_wstring m_strSelectTextureName = {};
	_int m_iSelectTextureIndex = { -1 };

	//container
	list<CUI_Container*> m_ContainerList = {};
	CUI_Container* m_pContainerObj = { nullptr };
	_int m_iSelectContainerIndex = { -1 };

	//containerpart
	CUIObject* m_pSelectConatinerPart = { nullptr };
	_int m_iSelectObjIndex = { -1 };

	//
	CUI_Container* m_pMergeContainer = {};
	_int m_iMergeContainerPartIndex = { -1 };

	

	//  temp�� ������ �ް� ���� ������ �ٲٱ�
	CStatic_UI::STATIC_UI_DESC eStaticUIDesc = {};
	CStatic_UI::STATIC_UI_DESC eStaticUITempDesc = {};

	CUI_Text::TEXT_UI_DESC m_eTextTempUIDesc = {};
	CUI_Text::TEXT_UI_DESC m_eTextUIDesc = {};
	string m_strInput = {};

	CDynamic_UI::DYNAMIC_UI_DESC eDynamicUITempDesc = {};
	CDynamic_UI::DYNAMIC_UI_DESC eDynamicUIDesc = {};

	CUI_Button::BUTTON_UI_DESC eButtonUITempDesc = {};
	CUI_Button::BUTTON_UI_DESC eButtonUIDesc = {};
	string m_strCaption = {};

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

	_bool m_bActive = {false};

	
public:
	static CGLTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END