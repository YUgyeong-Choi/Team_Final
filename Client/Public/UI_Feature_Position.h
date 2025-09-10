#pragma once

#include "Client_Defines.h"

#include "UI_Feature.h"

NS_BEGIN(Client)

class CUI_Feature_Position final : public CUI_Feature
{
private:
	CUI_Feature_Position(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Feature_Position(const CUI_Feature_Position& Prototype);
	virtual ~CUI_Feature_Position() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void Update(_int& iCurrentFrame, CDynamic_UI* pUI, _bool isReverse = false);
	virtual HRESULT Bind_ShaderResources(class CShader* pShader);

	virtual UI_FEATURE_TOOL_DESC Get_Desc_From_Tool() override;
	virtual UI_FEATRE_DESC& Get_Desc() override;

	virtual json Serialize();
	virtual void Deserialize(const json& j);

	void Set_Position(_bool isStartPos, _float2 fPos);
	_float2& Get_Position(_bool isStartPos);

private:

	// ���� �޾ƿͼ� �ݴ�� �������� �����ϸ� �ɵ�?
	_bool	m_isReverse = {};
	
	// �̰Ŵ� �� ������ ��������
	_float2 m_fStartPos = {};
	_float2 m_fEndPos = {};
	// �׸��� GetClientRect�� �̿��ؼ� ũ�⸦ ���ϰ� ��ġ�� ��������
	_float2 m_fCurrentPos = {};


	UI_FEATURE_POS_DESC m_eDesc = {};

public:

	static CUI_Feature_Position* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};


NS_END