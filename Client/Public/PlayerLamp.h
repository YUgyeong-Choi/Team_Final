#pragma once
#include "GameObject.h"
#include "Client_Defines.h"
#include "Light.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CPlayerLamp : public CGameObject
{
public:
	typedef struct tagPlayerLampDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*	szMeshID;
		LEVEL			eMeshLevelID;
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };
		CGameObject*	pOwner = { nullptr };

		const _float4x4* pSocketMatrix = { nullptr };
		const _float4x4* pParentWorldMatrix = { nullptr };
	}PLAYERLAMP_DESC;

protected:
	CPlayerLamp(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPlayerLamp(const CPlayerLamp& Prototype);
	virtual ~CPlayerLamp() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

protected: /* [ Setup 함수 ] */
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	HRESULT Ready_Light();

public: /* [ 소유자 (Unit) 을 가져온다. ] */
	class CPlayer* Get_Owner() const { return m_pOwner; }
	void Clear_Owner() { m_pOwner = nullptr; }

public: /* [ 램프의 소유여부 ] */
	void SetbLampVisible(_bool bIsVisible) { m_bIsVisible = bIsVisible; }
	_bool GetbLampVisible() const { return m_bIsVisible; }

public:
	_wstring Get_MeshName() { return (m_szMeshID != nullptr) ? wstring(m_szMeshID) : wstring(); }
	const _float4x4* Get_CombinedWorldMatrix() const { return &m_CombinedWorldMatrix; }

public:
	void    ToggleLamp() 
	{
		m_bIsUse = !m_bIsUse;
		if (m_bIsUse)
			m_pLight->Get_LightDesc()->bIsUse = true;
		else
			m_pLight->Get_LightDesc()->bIsUse = false;
	}

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

protected:
	const _float4x4*	m_pParentWorldMatrix = { nullptr };
	const _float4x4*	m_pSocketMatrix = { nullptr };
	_float4x4			m_CombinedWorldMatrix = {};

protected:				/* [ 기본 속성 ] */
	_bool				m_bIsVisible = {};
	_float				m_fSpeedPerSec = 5.f;
	_float				m_fRotationPerSec = XMConvertToRadians(90.f);
	_float3				m_InitPos = {};
	_float3				m_InitScale = {};
	_int				m_iRender = {};

protected: 				/* [ 기본 타입 ] */
	const _tchar*		m_szName = { nullptr };
	const _tchar*		m_szMeshID = { nullptr };
	LEVEL				m_eMeshLevelID = { LEVEL::END };

protected:              /* [ 컴포넌트 ] */
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };
	CModel*				m_pLightModelCom = { nullptr };

	class CPlayer*		m_pOwner = { nullptr };

	_bool				m_bDoOnce = {};

private:
	// 빛, 사용중인지 아닌지
	CLight* m_pLight = { nullptr };
	_bool   m_bIsUse = { false };

	_bool m_bDebug = { false };

	_uint m_iID = { 0 };
	LEVEL m_eTargetLevel = { LEVEL::END };

public:
	static CPlayerLamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END