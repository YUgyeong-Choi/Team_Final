#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CPlayerLamp : public CGameObject
{
	/* [ 모든 무기객체의 부모클래스입니다. ] */


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

public: /* [ 소유자 (Unit) 을 가져온다. ] */
	class CUnit* Get_Owner() const { return m_pOwner; }
	void Clear_Owner() { m_pOwner = nullptr; }

public: /* [ 램프의 소유여부 ] */
	void SetbLampVisible(_bool bIsVisible) { m_bIsVisible = bIsVisible; }
	_bool GetbLampVisible() const { return m_bIsVisible; }

public:
	_wstring Get_MeshName() { return (m_szMeshID != nullptr) ? wstring(m_szMeshID) : wstring(); }
	const _float4x4* Get_CombinedWorldMatrix() const { return &m_CombinedWorldMatrix; }

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

protected:              /* [ 컴포넌트 ] */
	CModel*				m_pModelCom = { nullptr };
	CShader*			m_pShaderCom = { nullptr };

	class CUnit*		m_pOwner = { nullptr };

public:
	static CPlayerLamp* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END