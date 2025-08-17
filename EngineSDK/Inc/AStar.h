#pragma once

#include "Base.h"


NS_BEGIN(Engine)


class CAStar final : public CBase
{
private:
	CAStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CAStar() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual HRESULT Update();
	virtual HRESULT Render(); // 경로 그릴 용도?

	_bool Find_Route();

	void Adjust_Route();

	void Add_Requester(class CGameObject*);

private:
	//디버깅용
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	_bool m_bCloned = {};

	class CGameInstance* m_pGameInstance = { nullptr };
	
	vector<class CGameObject*> m_Requester;
	
	map<class CGameObject*, list<_float4>> m_RouteMap;

public:
	static CAStar* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
	
};

NS_END