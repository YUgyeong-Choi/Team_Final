#pragma once
/* [ 야외 맵으로 갈 때 key 필요한 문들 ] */
#include "Client_Defines.h"
#include "DefaultDoor.h"
#include "Player.h"


NS_BEGIN(Client)

class CBossDoor : public CDefaultDoor
{
public:
	typedef struct tagBossDoorMeshDesc : public CDefaultDoor::DEFAULTDOOR_DESC
	{
		_bool		bNeedSecondDoor;
		_tchar		szSecondModelPrototypeTag[MAX_PATH] = { 0 };
	}BOSSDOORMESH_DESC;
protected:
	CBossDoor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBossDoor(const CBossDoor& Prototype);
	virtual ~CBossDoor() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void On_TriggerEnter(CGameObject* pOther, COLLIDERTYPE eColliderType);
	virtual void On_TriggerExit(CGameObject* pOther, COLLIDERTYPE eColliderType);
	void Register_Events();
public:
	void Create_CrashDoorEffect();
protected:
	HRESULT Ready_Components(void* pArg);
protected:
	HRESULT LoadFromJson();

	virtual void Move_Player(_float fTimeDelta) override;
private:
	// 바뀌는 문을 위한
	_bool m_bRenderSecond = false;
	CModel* m_pSecondModelCom = { nullptr };
	CAnimator* m_pSecondAnimator = { nullptr };
public:
	static CBossDoor* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END