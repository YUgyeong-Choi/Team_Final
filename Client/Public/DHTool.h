#pragma once
#include "GameObject.h"

#include "Client_Defines.h"


NS_BEGIN(Engine)
class CShader;
NS_END

NS_BEGIN(Client)

class CDHTool final : public CGameObject
{
public:
	enum class LIGHT_TYPE
	{
		POINT,
		SPOT,
		DIRECTIONAL,
		LIGHT_TYPE_END
	};
	enum class LEVEL_TYPE
	{
		KRAT_CENTERAL_STATION,
		KRAT_HOTEL,
		LEVEL_END
	};

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
	HRESULT Create_Light(LIGHT_TYPE eType, LEVEL_TYPE eLType);
	HRESULT Add_Light(LIGHT_TYPE eType, LEVEL_TYPE eLType);

private:
	void Picking();
	void PickGuizmo();

private:
	HRESULT Save_Shader(
		_float Diffuse, _float Normal, _float AO, _float AOPower, _float Roughness, _float Metallic, _float Reflection, _float Specular, _float4 vTint);
	HRESULT Load_Shader(
		_float& Diffuse, _float& Normal, _float& AO, _float& AOPower, _float& Roughness, _float& Metallic, _float& Reflection, _float& Specular, _float4& vTint);

private:
	CShader* m_pShaderCom = { nullptr };


private: /* [ 라이팅툴 관련 변수들 ] */
	static const char* szLightName[];
	static const char* szLevelName[];
	int m_iSelectedLightType = 0;
	int m_iSelectedLevelType = 0;

private:
	_uint m_iID = { 0 };

private:
	CGameObject* m_pSelectedObject = nullptr;
	ImGuizmo::OPERATION m_eGizmoOp = ImGuizmo::TRANSLATE;

public:
	static CDHTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END