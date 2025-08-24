#pragma once

#include "Base.h"

/* ���� �����ϴ� �ﰢ�� �ϳ�. */

NS_BEGIN(Engine)

class CCell final : public CBase
{
public:
	enum POINT { POINT_A, POINT_B, POINT_C, POINT_END };
	enum LINE { LINE_AB, LINE_BC, LINE_CA, LINE_END };
private:
	CCell(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CCell() = default;

public:
	_vector Get_Point(POINT ePoint) {
		return XMLoadFloat3(&m_vPoints[ePoint]);
	}

	void Clear_Neighbors() {

		for (_int i = 0; i < LINE_END; ++i)
		{
			m_iNeighborIndices[i] = -1;
		}
	}

public:
	void Set_Neighbor(LINE eLine, CCell* pCell) {
		m_iNeighborIndices[eLine] = pCell->m_iIndex;
		//m_pNeighbors[eLine] = pCell;
	}

	_int Get_Index() const {
		return m_iIndex;
	}

	void Set_Index(_int iIndex) {
		m_iIndex = iIndex;
	}

	//�̿��� �ε����� ��ȯ�Ѵ�.
	_int* Get_NeighborIndices() {
		return m_iNeighborIndices;
	}

	//CCell** Get_Neighbors() {
	//	return m_pNeighbors;
	//}

public:
	HRESULT Initialize(const _float3* pPoints, _int iIndex);
	_bool isIn(_fvector vLocalPos, _int* pNeighborIndex, _float* pDist = nullptr);
	NavigationEdge* FindEdge(_fvector vPosition);
	_bool Compare(_fvector vSour, _fvector vDest);
	_float Compute_Height(_fvector vLocalPos);

#ifdef _DEBUG
public:
	HRESULT Render();

#endif

private:	
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

	_float3			m_vPoints[POINT_END] = {};
	_float3			m_vNormals[LINE_END] = {};
	_int			m_iNeighborIndices[LINE_END] = { -1, -1, -1 };
	//CCell*			m_pNeighbors[LINE_END] = { nullptr, nullptr, nullptr };
	_int			m_iIndex = {};

	NavigationEdge m_LastEdge;

#ifdef _DEBUG
	class CVIBuffer_Cell* m_pVIBuffer = { nullptr };
#endif

public:
	static CCell* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _float3* pPoints, _int iIndex);
	virtual void Free() override;

};

NS_END