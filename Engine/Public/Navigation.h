#pragma once

#include "Component.h"

/* 내 게임 내에 길을 구성해주는 삼각형들을 모아서 보관한다. */

NS_BEGIN(Engine)

class ENGINE_DLL CNavigation final : public CComponent
{
public:
	enum class ShaderPass
	{
		DSS_DEFAULT = 0,
		DSS_NONE = 1,
	};

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
	void Set_ShaderPass(ShaderPass eShaderPass) {
		m_iShaderPass = ENUM_CLASS(eShaderPass);
	}

public:
	_float Compute_NavigationY(const _vector pTransform);

public:
	HRESULT Add_Cell(const _float3* pPoints);
	
	//선택된 셀을 삭제한다.
	HRESULT Delete_Cell();

	//월드포지션으로 셀을 선택한다.
	HRESULT Select_Cell(_fvector vWorldPos);

	//가장 가까운 점으로 스냅해준다.(fDist보다 가까울 경우만)
	HRESULT Snap(_float3* vWorldPos, _float fSnapThreshold);

public:
	HRESULT Save(const _char* Map);

public:
	vector<class CCell*>& Get_Cells() {
		return m_Cells;
	}

	_int Get_Index() {
		return m_iIndex;
	}

	void Set_Index(_int iIndex) {
		m_iIndex = iIndex;
	}

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
	_uint m_iShaderPass = { ENUM_CLASS(ShaderPass::DSS_DEFAULT)};
#endif

private:
	HRESULT SetUp_Neighbors();

public:
	static CNavigation* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pNavigationDataFile);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;
};	

NS_END