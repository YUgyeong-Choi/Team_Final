#pragma once
#include "Unit.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class CPhysXController;
NS_END

NS_BEGIN(Client)


class CWego : public CUnit
{
public:
	typedef struct tagWegoDesc : public CUnit::tagUnitDesc
	{
	}WEGO_DESC;
protected:
	CWego(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWego(const CWego& Prototype);
	virtual ~CWego() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private: /* [ Setup ÇÔ¼ö ] */
	HRESULT Ready_Components();
	HRESULT Ready_Controller();
	CPhysXController* m_pControllerCom = { nullptr };
public:
	static CWego* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg = nullptr) override;
	virtual void Free() override;
};

NS_END

