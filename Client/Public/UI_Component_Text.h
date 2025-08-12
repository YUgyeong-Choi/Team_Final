#pragma once

#include "Client_Defines.h"
#include "UI_Component.h"

NS_BEGIN(Client)

class CUI_Component_Text final : public CUI_Component
{
public:
	typedef struct tagTextDesc
	{
		_wstring strFontTag = {};
		// 채울 내용
		_wstring strCaption = {};
		//중앙 기준으로 얼마나 떨어져서 그리게 할건지
		_float2 fOffset = {};
		// 크기
		_float fScale = {};

		TEXTALIGN eAlign = {};

		_float fRotation = {};

		_float2 vPos = {};

		_float4 vColor = {};

	}TEXT_DESC;


private:
	CUI_Component_Text(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Component_Text(const CUI_Component_Text& Prototype);
	virtual ~CUI_Component_Text() = default;


public:

	virtual json Serialize() override;
	virtual void Deserialize(const json& j) override;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	// 폰트를 그린다
	void Draw(CUIObject* pUI);

	void Change_Caption(_wstring& strCaption);

private:
	// 사용할 폰트
	_wstring m_strFontTag = {};
	// 채울 내용
	_wstring m_strCaption = {};
	//중앙 기준으로 얼마나 떨어져서 그리게 할건지
	_float2 m_fOffset = {};
	// 크기
	_float m_fScale = {};
	// 정렬 타입
	TEXTALIGN m_eAlignType = { TEXTALIGN::LEFT };
	
	_float m_fRotation = {};

	_float2 m_vPos = {};

	_float4 m_vColor = {};





public:
	static CUI_Component_Text* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END