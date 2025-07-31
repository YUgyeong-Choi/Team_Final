#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Client)

// 파일에 있는 거만 가져오도록
// 파일을 따로 툴로 만들어서 저장하면 될듯

class CUI_Container  : public CUIObject
{
public:
	typedef struct tagUIContainerDesc : public CUIObject::UIOBJECT_DESC
	{

		_wstring strFilePath;
	}UI_CONTAINER_DESC;
protected:
	CUI_Container(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Container(const CUI_Container& Prototype);
	virtual ~CUI_Container() = default;

public:


	virtual json Serialize();
	virtual void Deserialize(const json& j);

	vector<class CUIObject*>& Get_PartUI();

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	
	void Active_Update(_bool isActive);

	HRESULT Add_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg);

	void Add_UI_From_Tool(CUIObject* pObj);

protected:
	_wstring							m_strFilePath = {};
	vector<class CUIObject*>			m_PartObjects;

protected:


public:
	static CUI_Container* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END