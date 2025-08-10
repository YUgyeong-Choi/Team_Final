#pragma once

#include "Client_Defines.h"
#include "GameObject.h"
#include "Light.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
NS_END

NS_BEGIN(Client)

class CDH_ToolMesh final : public CGameObject
{
public:
	typedef struct tagDH_ToolDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szMeshID;
		_float3		m_vInitPos = { 0.f, 0.f, 0.f };

		_int		iID = { 0 };
		LEVEL		eLEVEL = { LEVEL::END };
	}DHTOOL_DESC;

private:
	CDH_ToolMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDH_ToolMesh(const CDH_ToolMesh& Prototype);
	virtual ~CDH_ToolMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	_uint Get_ID() const { return m_iID; }

public:
	void SetIntensity(_float fIntensity) { m_pLight->Get_LightDesc()->fIntensity = fIntensity; }
	_float GetIntensity() { return m_pLight->Get_LightDesc()->fIntensity; }

	void SetColor(_float4 vColor) { m_pLight->Get_LightDesc()->vDiffuse = vColor; }
	_float4 GetColor() { return m_pLight->Get_LightDesc()->vDiffuse; }

	void SetRange(_float fRange) { m_pLight->Get_LightDesc()->fRange = fRange; }
	_float GetRange() { return m_pLight->Get_LightDesc()->fRange; }

	void SetfInnerCosAngle(_float fInnerCosAngle) { m_pLight->Get_LightDesc()->fInnerCosAngle = cosf(XMConvertToRadians(fInnerCosAngle));; }
	_float GetfInnerCosAngle() {_float cosAngle = m_pLight->Get_LightDesc()->fInnerCosAngle; return XMConvertToDegrees(acosf(cosAngle));}
	void SetfOuterCosAngle(_float fOuterAngleDegrees){m_pLight->Get_LightDesc()->fOuterCosAngle = cosf(XMConvertToRadians(fOuterAngleDegrees));}
	_float GetfOuterCosAngle(){	_float cosAngle = m_pLight->Get_LightDesc()->fOuterCosAngle;return XMConvertToDegrees(acosf(cosAngle));	}
	void SetfFalloff(_float fFalloff) { m_pLight->Get_LightDesc()->fFalloff = fFalloff; }
	_float GetfFalloff() { return m_pLight->Get_LightDesc()->fFalloff; }

	void SetfFogDensity(_float fFogDensity) { m_pLight->Get_LightDesc()->fFogDensity = fFogDensity; }
	_float GetfFogDensity() { return m_pLight->Get_LightDesc()->fFogDensity; }

	void SetfFogCutOff(_float fFogCutoff) { m_pLight->Get_LightDesc()->fFogCutoff = fFogCutoff; }
	_float GetfFogCutOff() { return m_pLight->Get_LightDesc()->fFogCutoff; }

	void SetbVolumetric(_bool bVolumetric) { m_pLight->Get_LightDesc()->bIsVolumetric = bVolumetric; }
	_bool GetbVolumetric() { return m_pLight->Get_LightDesc()->bIsVolumetric; }

	_int GetLightType() { return static_cast<int>(m_pLight->Get_LightDesc()->eType); }

public: /* [ 플레이어와의 거리 측정 ] */
	void SetIsPlayerFar(_bool bPlayerFar) { m_pLight->Get_LightDesc()->bIsPlayerFar = bPlayerFar; }
	_bool GetIsPlayerFar() { return m_pLight->Get_LightDesc()->bIsPlayerFar; }

public:
	void SetDebug(_bool bDebug) { m_bDebug = bDebug; }
	_bool GetDebug() const { return m_bDebug; }


protected: /* [ 플레이어 ] */
	CGameObject* m_pPlayer = { nullptr };

private:
	_uint m_iID = { 0 };
	LEVEL m_eTargetLevel = { LEVEL::END };

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };

protected: /* [ 초기화 변수 ] */
	const _tchar*	m_szMeshID = { nullptr };
	_float3			m_InitPos = {};

private:
	CLight* m_pLight = { nullptr };
	_bool m_bDebug = { true };

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();
	HRESULT Ready_Light();

public:
	static CDH_ToolMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END