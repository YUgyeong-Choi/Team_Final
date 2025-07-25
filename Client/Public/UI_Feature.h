#pragma once

#include "Component.h"

#include "Client_Defines.h"

NS_BEGIN(Client)

class CUI_Feature abstract : public CComponent
{
protected:
	CUI_Feature() = default;
	CUI_Feature(const CUI_Feature& Prototype);
	virtual ~CUI_Feature() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual HRESULT Update(_float fTimeDelta) = 0;
	virtual HRESULT Bind_ShaderResources(class CShader* pShader) = 0;



public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END