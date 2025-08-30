#pragma once

#include "Base.h"

NS_BEGIN(Engine)


class CArea_Manager final : public CBase
{
private:
	CArea_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CArea_Manager() = default;


public:
	// 초기화
	HRESULT Initialize();
	HRESULT Reset_Parm();



public:
	_bool AddArea_AABB(_int iAreaId, const _float3& vMin, const _float3& vMax, const vector<_uint>& vecAdjacentIds, AREA::EAreaType eType, _int iPriority);
	HRESULT FinalizePartition();
	_int FindAreaContainingPoint();

public:
	const AABBBOX* GetAreaBounds(_int iAreaId) const;
	void           GetActiveAreaBounds(vector<AABBBOX>& vecOutBounds, _float fPad = 0.f) const;
	void		   GetActiveAreaIds(vector<_uint>& vecOutAreaIds) const;
	AREAMGR	   GetCurrentAreaMgr();


public: /* [ 디버깅 함수 ]*/
	void DebugDrawCells();
	void RenderDebugLines(const vector<DebugLine>& lines);
	void ToggleDebugCells() { m_DebugDrawCells = !m_DebugDrawCells; }
	XMFLOAT4 BaseColorByType(AREA::EAreaType eType);
	XMFLOAT4 Lerp(const XMFLOAT4& a, const XMFLOAT4& b, _float t);
	XMFLOAT4 GetAreaDebugColor(const AREA& tArea, _int iCurrentAreaId,_bool bOverlapped);

public:
	void SetPlayerPosition(const _vector& vPos) { m_vPlayerPos = vPos; }
	_vector GetPlayerPosition() const { return m_vPlayerPos; }

private:
	AREAMGR m_eAreaMgr = { AREAMGR::END };

private:
	_vector m_vPlayerPos = {};

	_int m_iWarmNeighbors = {};
	_float m_fEnterGrace = {};
	_float m_fExitDelay = {};
	_bool m_bUseHysteresis = {};

	_int m_iCurrentAreaId = { -1 };
	_int m_iPrevAreaId = { -1 };
	_int m_iFrameId = {};
	_bool m_bDebugDraw = {};

	vector<AREA> m_vecAreas = {};
	vector<_int> m_vecActiveAreaIds = {};
	vector<_int> m_vecWarmAreaIds = {};
	vector<AREA> m_vecAreaHandles = {};
	unordered_map<_int, _uint> m_mapAreaIdToIndex;



private: /* [ 디버깅 그리기 변수 ] */
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
	BasicEffect* m_pEffect = { nullptr };
	ID3D11InputLayout* m_pInputLayout = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };

	_bool m_DebugDrawCells = {};

public:
	static CArea_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
};

NS_END