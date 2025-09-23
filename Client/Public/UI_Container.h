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
		_bool	 useLifeTime = {};
		_float   fLifeTime = {};
		_wstring strFilePath;
		// 활성화되면 작동할 사운드?
		string strSoundTag = {};
	}UI_CONTAINER_DESC;
protected:
	CUI_Container(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Container(const CUI_Container& Prototype);
	virtual ~CUI_Container() = default;

public:


	virtual json Serialize();
	virtual void Deserialize(const json& j);

	vector<class CUIObject*>& Get_PartUI();
	virtual void Set_isReverse(_bool isReverse);

	void Set_UseLifeTime() { m_useLifeTime = true; }
	void Set_LifeTime(_float fLifeTime) { m_fLifeTime = fLifeTime; }

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	
	virtual void Active_Update(_bool isActive);

	HRESULT Add_PartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg);

	void Add_UI_From_Tool(CUIObject* pObj);

protected:
	_wstring							m_strFilePath = {};
	vector<class CUIObject*>			m_PartObjects;

	_bool								m_isReverse = {false};

	_bool								m_useLifeTime = { false };
	_float								m_fLifeTime = {};
	_bool								m_isSound = {};
	string								m_strSoundTag = {};

protected:


public:
	static CUI_Container* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END