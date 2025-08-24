#pragma once

#include "Client_Defines.h"
#include "Item.h"
#include "Light.h"


// 필요하면 추가, 벨트에 달려있는거 표현해야 될듯?
NS_BEGIN(Engine)
class CModel;
class CShader;
NS_END


NS_BEGIN(Client)

class CLamp final : public CItem
{
private:
	CLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLamp(const CLamp& Prototype);
	virtual ~CLamp() = default;

public:
	_bool	Get_isLight() { return m_isLight; }

	void SetIsPlayerFar(_bool bPlayerFar) { m_pLight->Get_LightDesc()->bIsPlayerFar = bPlayerFar; }
	_bool GetIsPlayerFar() { return m_pLight->Get_LightDesc()->bIsPlayerFar; }

	void SetbVolumetric(_bool bVolumetric) { m_pLight->Get_LightDesc()->bIsVolumetric = bVolumetric; }
	_bool GetbVolumetric() { return m_pLight->Get_LightDesc()->bIsVolumetric; }

	void SetColor(_float4 vColor) { m_pLight->Get_LightDesc()->vDiffuse = vColor; }
	_float4 GetColor() { return m_pLight->Get_LightDesc()->vDiffuse; }

	void SetRange(_float fRange) { m_pLight->Get_LightDesc()->fRange = fRange; }
	_float GetRange() { return m_pLight->Get_LightDesc()->fRange; }

	void SetIntensity(_float fIntensity) { m_pLight->Get_LightDesc()->fIntensity = fIntensity; }
	_float GetIntensity() { return m_pLight->Get_LightDesc()->fIntensity; }

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	// 사용 효과 상속받아서 구현하기
	
	virtual void Use() override;
	virtual ITEM_DESC Get_ItemDesc() override;

	HRESULT Ready_Components();
	HRESULT Ready_Light();

	HRESULT Bind_ShaderResources();

private:
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
private:
	//
	_bool		m_isLight = { false };
	CLight*		m_pLight = { nullptr };
	const _float4x4* m_pParentWorldMatrix = { nullptr };

private:
	_bool m_bDebug = { true };
	
	_uint m_iID = { 0 };
	LEVEL m_eTargetLevel = { LEVEL::END };

public:
	static CLamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;

};

NS_END