#pragma once

#include "Navigation.h"

NS_BEGIN(Engine)


class CAStar final : public CBase
{
private:
	CAStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAStar(const CAStar& Prototype);
	virtual ~CAStar() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Update();


private:
	CNavigation* m_pNavi = { nullptr };
	
};

NS_END