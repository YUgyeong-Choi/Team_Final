#pragma once

#include "Base.h"

//#include <mutex>

NS_BEGIN(Engine)

class COctoTree_Manager final : public CBase
{
	enum class PlaneID { Left, Right, Bottom, Top, Near, Far };
public:
	struct Node
	{
		AABBBOX	AABBBounds;
		_uint	iFirstChild;
		_uint	iChildCount;
		_uint	iDepth;
		_uint	iFirstObj;
		_uint	iObjCount;
		_bool	bIsLeaf; 
	};


	struct PlaneStruct { XMFLOAT4 p; };
	struct Frustum
	{
		PlaneStruct planes[6];

		void BuildFromViewProj(const XMFLOAT4X4& vp);
		FrustumHit OctoIsInAABB(const _float3& bmin, const _float3& bmax) const;
		void OctoBuild(const _matrix& view, const _matrix& proj);
	};

private:
	COctoTree_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~COctoTree_Manager() = default;


public:
	// 초기화
	HRESULT Initialize();
	HRESULT Ready_OctoTree(const vector<AABBBOX>& staticBounds, const map<Handle, _uint>& handleToIndex);

	// 트리 빌드
	void BuildTree();
	void SubdivideNode(_uint nodeIdx);

	void MakeChildBoundsXZ(const AABBBOX& parent, AABBBOX out[4]);
	void MakeChildBoundsXYZ(const AABBBOX& tParent, AABBBOX out[8]);
	int ClassifyFullyContained(const AABBBOX child[8], const AABBBOX& obj) const;

	// 프레임 준비
	void BeginQueryFrame(const XMMATRIX& view, const XMMATRIX& proj);

	// 쿼리
	void QueryVisible();
	void PushNodeObjects_NoFrustum(const Node& node);
	void PushNodeObjects_WithFrustum(const Node& node);


	_bool IntersectsAnyArea_Object(const AABBBOX& tObjBox, _float fEps = 0.1f) const;
	void PushIfNotSeenThisFrame(_uint objIdx, vector<_uint>& out);

	// 디버그
	void DebugDrawCells();
	void ToggleDebugCells() { m_DebugDrawCells = !m_DebugDrawCells; }

	void RenderDebugLines(const vector<DebugLine>& lines);

	_bool ComputeWorldBounds(const vector<AABBBOX>& arr, AABBBOX& out);

public:
	vector<_uint> GetCulledStaticObjects() const { return m_CulledStaticObjects; }
	Handle StaticIndexToHandle(_uint idx) const { return m_StaticIndexToHandle[idx]; }

	vector<class CGameObject*> GetIndexToObj() const { return m_vecIndexToObj; }
	void PushBackIndexToObj(class CGameObject* vec) { m_vecIndexToObj.push_back(vec); }
	void ClearIndexToObj() { m_vecIndexToObj.clear(); }


public:
	void InitIndexToHandle(const map<Handle, _uint>& handleToIndex, size_t count);

public:
	HRESULT SetObjectType(const vector<OCTOTREEOBJECTTYPE>& vTypes);
	const vector<OCTOTREEOBJECTTYPE>& GetObjectType() const { return m_ObjectType; }

public:
	void PushOctoTreeObjects(class CGameObject* pObject){
		// 락 걸기
		//lock_guard<mutex> lock(m_mtx);

		m_vecOctoTreeObjects.push_back(pObject);
	}
	vector<class CGameObject*> GetOctoTreeObjects() const { return m_vecOctoTreeObjects; }
	void ClaerOctoTreeObjects() { m_vecOctoTreeObjects.clear(); }

private:
	//mutex m_mtx = {};

private: /* [ 절두체 변수들 ] */
	_float4 planes[6];
	_float4 m_DebugPlane;


private: /* [ 쿼드트리 변수들 ] */
	AABBBOX m_WorldBounds;

	_uint	m_iMaxDepth =  6;
	_uint	m_iMaxObjects = 16;
	
	_float	m_fMinCellSize = 10.f;
	vector<OCTOTREEOBJECTTYPE> m_ObjectType;

	_bool	m_IncludeBorderAsInside = { true };

	uint64_t			m_FrameId;
	vector<uint64_t>    m_LastSeenFrame;

private:
	vector<class CGameObject*> m_vecOctoTreeObjects;

private: /* [ 쿼드트리 노드들 ] */
	vector<Node>					m_Nodes;
	vector<_uint>					m_ObjectIndices;
	vector<AABBBOX>					m_StaticObjectBounds;
	vector<AABBBOX>					m_LastQueriedAreas;
	vector<Handle>					m_StaticIndexToHandle;
	vector<class CGameObject*>		m_vecIndexToObj;

	map<Handle, _uint>				m_HandleToStaticIndex;

private:
	Frustum m_Frustum;
	vector<_uint> m_VisibleCandidateNodes;
	vector<_uint> m_CulledStaticObjects;

	vector<_uint> m_TempNodeStack;
	vector<_uint> m_TempObjects;

private: /* [ LOD 관련 변수들 ] */
	_float m_LODNear = 20.f;
	_float m_LODMid = 50.f;
	_float m_LODFar = 120.f;

	vector<_float> m_ObjectLODRadii;

private: /* [ 디버깅 변수 ] */
	_uint m_StatVisitedNodes;
	_uint m_StatPushedObjects;
	_uint m_StatCulledByFrustum;
	_uint m_StatObjCulledByFrustum;
	_bool m_DebugDrawCells = false;

private: /* [ 디버깅 그리기 변수 ] */
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
	BasicEffect* m_pEffect = { nullptr };
	ID3D11InputLayout* m_pInputLayout = { nullptr };

	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	class CGameInstance* m_pGameInstance = { nullptr };


public:
	static COctoTree_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;
	static void PrintMatrix(const _float4x4& tMat, const _char* pLabel);
};

NS_END