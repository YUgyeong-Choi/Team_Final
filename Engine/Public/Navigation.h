#pragma once

#include "Component.h"

/* �� ���� ���� ���� �������ִ� �ﰢ������ ��Ƽ� �����Ѵ�. */

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	typedef struct tagNavigationDesc
	{
		_int			iIndex = { -1 };
	}NAVIGATION_DESC;
private:
	CNavigation(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNavigation(const CNavigation& Prototype);
	virtual ~CNavigation() = default;

public:
	HRESULT Initialize_Prototype(const _tchar* pNavigationDataFile);
	HRESULT Initialize(void* pArg);

	void Update(_fmatrix WorldMatrix);
	_bool isMove(_fvector vWorldPos);
	_vector SetUp_Height(_fvector vWorldPos);

public:
	_float Compute_NavigationY(const _vector pTransform);

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;

#endif

private:	
	vector<class CCell*>				m_Cells;
	_int								m_iIndex = { -1 };

	static _float4x4					m_WorldMatrix;

#ifdef _DEBUG
	class CShader* m_pShader = { nullptr };
#endif

private:
	HRESULT SetUp_Neighbors();

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};	

NS_END