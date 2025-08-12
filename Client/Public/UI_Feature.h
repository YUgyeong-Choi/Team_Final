#pragma once

#include "Component.h"

#include "Client_Defines.h"
#include "Dynamic_UI.h"
#include "Serializable.h"

NS_BEGIN(Client)

class CUI_Feature abstract : public CComponent, public ISerializable
{
protected:
	CUI_Feature(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Feature(const CUI_Feature& Prototype);
	virtual ~CUI_Feature() = default;

public:
	void Set_IsLoop(_bool isLoop) { m_isLoop = isLoop; }
	void Reset() { m_iCurrentFrame = 0; }

	// ������ ����� ������ ����.
	virtual UI_FEATURE_TOOL_DESC Get_Desc_From_Tool() = 0;
	virtual UI_FEATRE_DESC& Get_Desc() = 0;

	virtual json Serialize();
	virtual void Deserialize(const json& j);

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	//���� ������ �������� �����Ѵ�, ���� �������� ui ��ü���� ��ȯ�ϰ� �Ѱ��ִ� ������??
	virtual void Update(_int& iCurrentFrame, CDynamic_UI* pUI) = 0;
	virtual HRESULT Bind_ShaderResources(class CShader* pShader) = 0;



protected:
	_int   m_iStartFrame = {0};
	_int   m_iEndFrame = {60};
	_int   m_iCurrentFrame = {};
	_int   m_iRange = {};

	_bool  m_isLoop = {false};

	_wstring m_strProtoTag = {};
	
	// ������ ���?
	UI_FEATRE_DESC* m_pDesc = {};

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END