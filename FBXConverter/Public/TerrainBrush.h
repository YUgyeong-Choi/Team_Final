#pragma once

#include "Client_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CTerrainBrush final : public CGameObject
{
private:
	CTerrainBrush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTerrainBrush(const CTerrainBrush& Prototype);
	virtual ~CTerrainBrush() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual EVENT Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:

public:
	static CTerrainBrush* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END