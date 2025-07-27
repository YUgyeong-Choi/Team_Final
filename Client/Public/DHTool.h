#pragma once
#include "GameObject.h"

#define USE_IMGUI
#include "Client_Defines.h"
#undef USE_IMGUI


NS_BEGIN(Engine)
class CShader;
NS_END

NS_BEGIN(Client)

class CDHTool final : public CGameObject
{
private:
	CDHTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDHTool(const CDHTool& Prototype);
	virtual ~CDHTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();


private:
	HRESULT Render_ShaderTool();
	HRESULT Render_LightTool();

private:
	HRESULT Save_Shader(
		_float Diffuse, _float Normal, _float AO, _float AOPower, _float Roughness, _float Metallic, _float Reflection, _float Specular, _float4 vTint);
	HRESULT Load_Shader(
		_float& Diffuse, _float& Normal, _float& AO, _float& AOPower, _float& Roughness, _float& Metallic, _float& Reflection, _float& Specular, _float4& vTint);

private:
	CShader* m_pShaderCom = { nullptr };

public:
	static CDHTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END