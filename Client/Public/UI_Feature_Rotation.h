#pragma once

#include "Client_Defines.h"

#include "UI_Feature.h"

NS_BEGIN(Client)

class CUI_Feature_Rotation final : public CUI_Feature
{
private:
	CUI_Feature_Rotation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Feature_Rotation(const CUI_Feature_Rotation& Prototype);
	virtual ~CUI_Feature_Rotation() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void Update(_int& iCurrentFrame, CDynamic_UI* pUI, _bool isReverse = false);
	virtual HRESULT Bind_ShaderResources(class CShader* pShader);

	virtual UI_FEATURE_TOOL_DESC Get_Desc_From_Tool() override;
	virtual UI_FEATRE_DESC& Get_Desc() override;

	virtual json Serialize();
	virtual void Deserialize(const json& j);

	

private:

	_float m_fStartRotation = {};
	_float m_fEndRotation = {};
	
	_float m_fCurrentRotation = {};

	_float2 m_fRotationPos = {};

	_float2 m_fInitPos = {};

	_float4 m_vInitPos = {};
	_float4 m_vOffset = {};
	_float4 m_vCenter = {};


	UI_FEATURE_ROTATION_DESC m_eDesc = {};

public:

	static CUI_Feature_Rotation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


NS_END