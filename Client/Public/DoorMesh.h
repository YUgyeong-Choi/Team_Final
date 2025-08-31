#pragma once

#include "Client_Defines.h"
#include "DynamicMesh.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CModel;
class CPhysXDynamicActor;
class CPhysXStaticActor;
class CSoundController;
class CSound_Core;
NS_END

NS_BEGIN(Client)

class CDoorMesh : public CDynamicMesh
{
public:
	typedef struct tagDoorMeshDesc : public CDynamicMesh::DYNAMICMESH_DESC
	{
		INTERACT_TYPE eInteractType;
		_vector vTriggerOffset;
		_vector vTriggerSize;
		CSound_Core* pBGM;
		CSound_Core* pBGM2;
	}DOORMESH_DESC;
protected:
	CDoorMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDoorMesh(const CDoorMesh& Prototype);
	virtual ~CDoorMesh() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);

	void Play_Sound();
	void Play_BGM(_float fTimeDelta);
protected:
	HRESULT Ready_Components(void* pArg);
	HRESULT Ready_Trigger(DOORMESH_DESC* pDesc);
private:
	CPhysXStaticActor* m_pPhysXTriggerCom = { nullptr };
	CSoundController* m_pSoundCom = { nullptr };
	INTERACT_TYPE m_eInteractType;

	_bool m_bCanActive = false;
	_bool m_bFinish = false;

	// 레벨에서 받아온 BGM
	CSound_Core* m_pBGM = { nullptr };
	CSound_Core* m_pBGM2 = { nullptr };
	_bool m_bInSound = false;
	_bool m_bBGMToZero = false;
	_bool m_bBGMToVolume = false;
	_float m_fBGMVolume = 1.f;
public:
	static CDoorMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END