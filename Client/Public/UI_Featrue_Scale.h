#pragma once

#include "Client_Defines.h"

#include "UI_Feature.h"

NS_BEGIN(Client)

class CUI_Featrue_Scale final : public CUI_Feature
{
private:
	CUI_Featrue_Scale(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Featrue_Scale(const CUI_Featrue_Scale& Prototype);
	virtual ~CUI_Featrue_Scale() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void Update(_int& iCurrentFrame, CDynamic_UI* pUI);
	virtual HRESULT Bind_ShaderResources(class CShader* pShader);

	virtual UI_FEATURE_TOOL_DESC Get_Desc_From_Tool() override;
	virtual UI_FEATRE_DESC& Get_Desc() override;

private:

	// 상태 받아와서 반대로 움직일지 결정하면 될듯?
	_bool	m_isReverse = {};

	// 이거는 다 비율로 가져오자
	_float2 m_fStartPos = {};
	_float2 m_fEndPos = {};
	// 그리고 GetClientRect를 이용해서 크기를 구하고 위치를 정해주자
	_float2 m_fCurrentPos = {};
	_float	m_fMoveTime = { 1.f };


	UI_FEATURE_SCALE_DESC m_eDesc = {};

public:

	static CUI_Featrue_Scale* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


NS_END