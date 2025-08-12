#pragma once

#include "Component.h"

#include "Client_Defines.h"
#include "UIObject.h"
#include "Serializable.h"

NS_BEGIN(Client)

class CUI_Component abstract : public CComponent, public ISerializable
{
protected:
	CUI_Component(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Component(const CUI_Component& Prototype);
	virtual ~CUI_Component() = default;

public:

	virtual json Serialize();
	virtual void Deserialize(const json& j);

	_wstring Get_ProtoTag() { return m_strProtoTag; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	
	virtual void Update(_float fTimeDelta, CUIObject* pUI) {};
	virtual HRESULT Bind_ShaderResources(class CShader* pShader) { return S_OK; }

protected:
	// 어떤 컴포넌트인지 구분?
	_wstring m_strProtoTag = {};



public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END