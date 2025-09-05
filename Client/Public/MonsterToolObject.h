#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CShader;
class CAnimator;
class CPhysXActor;
class CPhysXDynamicActor;
NS_END

NS_BEGIN(Client)
class CMonsterToolObject : public CGameObject
{
	/* [ 모든 움직이는 객체의 부모클래스입니다. ] */
	/*  ( 플레이어, 몬스터, 보스몬스터, NPC )  */
	friend class CMonsterTool;

public:
	typedef struct tagMonsterToolObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_tchar			szMeshID[MAX_PATH];
		LEVEL			eMeshLevelID;
		_int			iRender = 0;
		_float3 		InitPos = { 0.f, 0.f, 0.f };
		_float3 		InitScale = { 1.f, 1.f, 1.f };
		_float4x4	WorldMatrix = _float4x4(
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		);
		_int iID = { 0 };

		SPAWN_TYPE eSpawnType = { SPAWN_TYPE::END };

	}MONSTERTOOLOBJECT_DESC;

protected:
	CMonsterToolObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMonsterToolObject(const CMonsterToolObject& Prototype);
	virtual ~CMonsterToolObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	//virtual HRESULT Render_Shadow() override;

	//void SetCascadeShadow();

private:
	void LoadAnimDataFromJson();


protected: /* [ Setup 함수 ] */
	HRESULT Bind_Shader();
	HRESULT Ready_Components();
	HRESULT Ready_Collider();

	virtual void Register_Events() {}

private:
	SPAWN_TYPE m_eSpawnType = { SPAWN_TYPE::IDLE };


private:
	_int m_iID = { -1 };

protected:				/* [ 기본 속성 ] */

	_bool				m_isActive = { true };
	_float				m_fSpeedPerSec = 5.f;
	_float				m_fRotationPerSec = XMConvertToRadians(90.f);
	_float3				m_InitPos = {};
	_float3				m_InitScale = {};
	_int				m_iRender = {};
	_bool				m_bUseLockon = {};

protected: 				/* [ 기본 타입 ] */
	_tchar				m_szMeshID[MAX_PATH];
	LEVEL				m_eMeshLevelID = { LEVEL::END };

protected:				/* [ 그림자 관련 ] */
	SHADOW				m_eShadow = SHADOW::SHADOW_END;

protected:              /* [ 컴포넌트 ] */
	CModel* m_pModelCom = { nullptr };
	CShader* m_pShaderCom = { nullptr };
	CAnimator* m_pAnimator = { nullptr };

public:
	static CMonsterToolObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END