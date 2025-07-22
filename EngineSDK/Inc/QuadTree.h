#pragma once

#include "Base.h"

NS_BEGIN(Engine)


/* �� ������ �����ϴ��ϳ��� ���. */
class CQuadTree final : public CBase
{
private:
	CQuadTree();
	virtual ~CQuadTree() = default;

public:
	enum CORNER { CORNER_LT, CORNER_RT, CORNER_RB, CORNER_LB, CORNER_END }; 
	enum NEIGHBOR { NEIGHBOR_LEFT, NEIGHBOR_TOP, NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM, NEIGHBOR_END };

public:
	HRESULT Initialize(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	void Culling(class CGameInstance* pGameInstance, const _float3* pVertexPositions, _uint* pIndices, _uint* pNumIndices);
	void Make_Neighbors();
private:
	_uint			m_iCorners[CORNER_END] = {};
	_uint			m_iCenter = {};
	CQuadTree*		m_Children[CORNER_END] = {};
	CQuadTree*		m_Neighbors[NEIGHBOR_END] = { nullptr };

private:
	_bool isDraw(class CGameInstance* pGameInstance, const _float3* pVertexPositions);


public:
	static CQuadTree* Create(_uint iLT, _uint iRT, _uint iRB, _uint iLB);
	virtual void Free() override;
};

NS_END