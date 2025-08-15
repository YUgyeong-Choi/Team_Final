#include "OctoTree_Manager.h"

#include "GameInstance.h"

COctoTree_Manager::COctoTree_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT COctoTree_Manager::Initialize()
{
    m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pContext);
    m_pEffect = new BasicEffect(m_pDevice);

    const void* pShaderByteCode = { nullptr };
    size_t		iShaderByteCodeLength = {  };

    m_pEffect->SetVertexColorEnabled(true);

    m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

    if (FAILED(m_pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
        pShaderByteCode, iShaderByteCodeLength, &m_pInputLayout)))
        return E_FAIL;

    return S_OK;
}

/* [ 해당 레벨의 객체의 월드스페이스상의 AABB , 핸들 값(0번 키에 담겨있는 인덱스번호) 를 받는다. ] */
HRESULT COctoTree_Manager::Ready_OctoTree(const vector<AABBBOX>& staticBounds, const map<Handle, _uint>& handleToIndex)
{
    // 1) 정적 데이터 스냅샷
    m_StaticObjectBounds = staticBounds;
    m_HandleToStaticIndex = handleToIndex;

    // 통계/버퍼 준비
    m_Nodes.clear();
    m_ObjectIndices.clear();
    m_VisibleCandidateNodes.clear();
    m_CulledStaticObjects.clear();
    m_LastSeenFrame.assign(m_StaticObjectBounds.size(), 0);
    m_ObjectLODRadii.resize(m_StaticObjectBounds.size());

    // 2) 월드 바운드 계산 (모든 정적 AABB를 감싸는 박스)
    if (!ComputeWorldBounds(m_StaticObjectBounds, m_WorldBounds))
        return E_FAIL;

    // 3) LOD 초기 반경 세팅 (선택: AABB 대각선 절반)
    for (_uint i = 0; i < static_cast<_uint>(m_StaticObjectBounds.size()); ++i)
        m_ObjectLODRadii[i] = AABB_HalfDiagonal(m_StaticObjectBounds[i]);

    // 4) 트리 빌드
    BuildTree();

    return S_OK;
}

void COctoTree_Manager::BuildTree()
{
    // 가장 큰 청크를 생성(초기화)
    Node root{};
    root.AABBBounds = m_WorldBounds;
    root.iDepth = 0;
    root.iChildCount = 0;
    root.iFirstChild = 0;
    root.iFirstObj = 0;
    root.iObjCount = static_cast<_uint>(m_StaticObjectBounds.size());
    root.bIsLeaf = false;

    //관리하는 오브젝트의 순번 매기기
    m_ObjectIndices.resize(root.iObjCount);
    for (_uint i = 0; i < root.iObjCount; ++i)
        m_ObjectIndices[i] = i;

	// 만든 청크를 노드 리스트에 추가
    m_Nodes.push_back(root);

    // 분할 시작
    SubdivideNode(0);
}
void COctoTree_Manager::SubdivideNode(_uint nodeIdx)
{
    _uint parent = nodeIdx;
    //Node& node = m_Nodes[nodeIdx];

    // 종료 조건 (깊이, 오브젝트 수, 최소사이즈)
    const _uint     iDepth = m_Nodes[parent].iDepth;
    const _uint     iObjCount = m_Nodes[parent].iObjCount;
    const _float3   fSize = AABB_Size(m_Nodes[parent].AABBBounds);
    const _float    fCellMin = min(fSize.x, min(fSize.y, fSize.z));

    if (iDepth >= m_iMaxDepth || iObjCount <= m_iMaxObjects || fCellMin <= m_fMinCellSize) {
        m_Nodes[parent].bIsLeaf = true;
        return;
    }




    // 자식 8개 AABB 생성 (XYZ 분할)
    AABBBOX childBounds[8];
    MakeChildBoundsXYZ(m_Nodes[parent].AABBBounds, childBounds);

    const _uint firstChild = static_cast<_uint>(m_Nodes.size());
    m_Nodes.resize(firstChild + 8);



    // 만든 자식들을 등록해준다.
    Node& node = m_Nodes[parent];
    node.iFirstChild = firstChild;
    node.iChildCount = 8;

    // 등록된 자식에 값을 채운다.
    for (_int i = 0; i < 8; ++i)
    {
        Node& child = m_Nodes[node.iFirstChild + i];
        child.AABBBounds = childBounds[i];
        child.iDepth = node.iDepth + 1;
        child.iChildCount = 0;
        child.iFirstChild = 0;
        child.iFirstObj = 0;
        child.iObjCount = 0;
        child.bIsLeaf = false;
    }



    // 현재 노드에 소속된 첫 객체와 마지막 객체
    const _uint begin = node.iFirstObj;
    const _uint end = begin + node.iObjCount;



	// 자식노드의 객체 리스트를 준비(넉넉히)
    vector<_uint> childLists[8];
    for (int i = 0; i < 8; ++i) 
    {
        childLists[i].reserve(iObjCount / 8 + 4);
    }

	// 부모한테 남아있어야할 객체 리스트를 준비(넉넉히)
    vector<_uint> remain; 
    remain.reserve(iObjCount / 2 + 4);


	// 객체를 순회하며 자식에 할당
    for (_uint i = begin; i < end; ++i)
    {
        const _uint objIdx = m_ObjectIndices[i];
        const AABBBOX& ob = m_StaticObjectBounds[objIdx];

		// 해당 객체의 AABB 가 완전히 IN , OUT, 걸침 여부를 판단
        _int fullyContainedChild = ClassifyFullyContained(childBounds, ob);
        
		// 완전히 포함된 자식이 있다면 해당 자식에 추가
        if (fullyContainedChild >= 0)
        {
            childLists[fullyContainedChild].push_back(objIdx);
        }
        else
        {
            // 경계에 걸치거나 여러 자식과 겹치면 현재 노드에 남긴다
            remain.push_back(objIdx);
        }
    }

    _uint iMovedToChildren = 0;
    for (_int iChild = 0; iChild < 8; ++iChild)
        iMovedToChildren += static_cast<_uint>(childLists[iChild].size());

    if (iMovedToChildren == 0) 
    {
        m_Nodes.resize(firstChild);
        m_Nodes[parent].bIsLeaf = true;
        m_Nodes[parent].iChildCount = 0;
        m_Nodes[parent].iFirstChild = 0;
        return;
    }


    /* [ 객체 재분배에 따른 인디시즈 재정렬 ] */

    _uint write = begin;
    // 1. 현재 노드에 남기는 것
    node.iFirstObj = write;
    node.iObjCount = static_cast<_uint>(remain.size());
    for (_uint iParrent : remain)
        m_ObjectIndices[write++] = iParrent;

    // 2. 자식들에게 넘어간 객체들
    for (int c = 0; c < 8; ++c) 
    {
        Node& child = m_Nodes[node.iFirstChild + c];
        child.iFirstObj = write;
        child.iObjCount = static_cast<_uint>(childLists[c].size());
        for (_uint iChild : childLists[c]) 
            m_ObjectIndices[write++] = iChild;
    }

    // 자식 재귀 분할
    const _uint firstChilds = m_Nodes[nodeIdx].iFirstChild;
    for (int c = 0; c < 8; ++c)
    {
        // 자식들을 전부 순회하며 객체가 등록되어있다면 재귀
        const _uint childIdx = firstChilds + c;
        //Node& child = m_Nodes[node.iFirstChild + c];
        if (m_Nodes[childIdx].iObjCount > 0)
            SubdivideNode(childIdx);
        else
        {
            m_Nodes[childIdx].bIsLeaf = true;
            m_Nodes[childIdx].iChildCount = 0;
        }
    }
}

void COctoTree_Manager::MakeChildBoundsXZ(const AABBBOX& parent, AABBBOX out[4])
{
    _float3 vCenter = AABB_Center(parent);

    // 공통 Y
    for (int i = 0; i < 4; ++i) {
        out[i].vMin.y = parent.vMin.y;
        out[i].vMax.y = parent.vMax.y;
    }

    // LT (Left-Top: X-쪽, Z-쪽)
    out[0].vMin = { parent.vMin.x, out[0].vMin.y, parent.vMin.z };
    out[0].vMax = { vCenter.x,     out[0].vMax.y, vCenter.z };

    // RT (Right-Top: X+, Z-)
    out[1].vMin = { vCenter.x,      out[1].vMin.y, parent.vMin.z };
    out[1].vMax = { parent.vMax.x,  out[1].vMax.y, vCenter.z };

    // RB (Right-Bottom: X+, Z+)
    out[2].vMin = { vCenter.x,      out[2].vMin.y, vCenter.z };
    out[2].vMax = { parent.vMax.x,  out[2].vMax.y, parent.vMax.z };

    // LB (Left-Bottom: X-, Z+)
    out[3].vMin = { parent.vMin.x,  out[3].vMin.y, vCenter.z };
    out[3].vMax = { vCenter.x,      out[3].vMax.y, parent.vMax.z };
}
void COctoTree_Manager::MakeChildBoundsXYZ(const AABBBOX& tParent, AABBBOX out[8])
{
    const _float3 vCenter = AABB_Center(tParent);

    // ---- 하층
    // 0: LT
    out[0].vMin = { tParent.vMin.x, tParent.vMin.y, tParent.vMin.z };
    out[0].vMax = { vCenter.x,      vCenter.y,      vCenter.z };

    // 1: RT
    out[1].vMin = { vCenter.x,      tParent.vMin.y, tParent.vMin.z };
    out[1].vMax = { tParent.vMax.x, vCenter.y,      vCenter.z };

    // 2: RB
    out[2].vMin = { vCenter.x,      tParent.vMin.y, vCenter.z };
    out[2].vMax = { tParent.vMax.x, vCenter.y,      tParent.vMax.z };

    // 3: LB
    out[3].vMin = { tParent.vMin.x, tParent.vMin.y, vCenter.z };
    out[3].vMax = { vCenter.x,      vCenter.y,      tParent.vMax.z };

    // ---- 상층
    // 4: LT (상층)
    out[4].vMin = { tParent.vMin.x, vCenter.y,      tParent.vMin.z };
    out[4].vMax = { vCenter.x,      tParent.vMax.y, vCenter.z };

    // 5: RT (상층)
    out[5].vMin = { vCenter.x,      vCenter.y,      tParent.vMin.z };
    out[5].vMax = { tParent.vMax.x, tParent.vMax.y, vCenter.z };

    // 6: RB (상층)
    out[6].vMin = { vCenter.x,      vCenter.y,      vCenter.z };
    out[6].vMax = { tParent.vMax.x, tParent.vMax.y, tParent.vMax.z };

    // 7: LB (상층)
    out[7].vMin = { tParent.vMin.x, vCenter.y,      vCenter.z };
    out[7].vMax = { vCenter.x,      tParent.vMax.y, tParent.vMax.z };
}

int COctoTree_Manager::ClassifyFullyContained(const AABBBOX child[8], const AABBBOX& obj) const
{
    for (int i = 0; i < 8; ++i)
    {
        if (AABB_ContainsAABB(child[i], obj, m_IncludeBorderAsInside ? 0.0f : -0.0001f))
            return i;
    }
    return -1;
}



void COctoTree_Manager::BeginQueryFrame(const XMMATRIX& view, const XMMATRIX& proj)
{
    ++m_FrameId;

    // 프러스텀 갱신
    m_Frustum.OctoBuild(view, proj);

    // 버킷 초기화
    m_VisibleCandidateNodes.clear();
    m_CulledStaticObjects.clear();
    m_TempNodeStack.clear();
    m_TempObjects.clear();

    // 통계 초기화
    m_StatVisitedNodes = 0;
    m_StatPushedObjects = 0;
    m_StatCulledByFrustum = 0;
}
void COctoTree_Manager::QueryVisible()
{
    if (m_Nodes.empty()) return;

    // 트리 탐색 시작
    m_TempNodeStack.clear();
    m_TempNodeStack.push_back(0);

    while (!m_TempNodeStack.empty())
    {
		// 가장최근에 넣은 노드를 꺼낸 후 제외
        _uint nodeId = m_TempNodeStack.back();
        m_TempNodeStack.pop_back();

        const Node& node = m_Nodes[nodeId];
        ++m_StatVisitedNodes;

        // 해당 노드를 검사
        FrustumHit hit = m_Frustum.OctoIsInAABB(node.AABBBounds.vMin, node.AABBBounds.vMax);


        // 쿼드를 탐색 후 결과에 따라 깊이 들어갈지 말지 결정
        if (hit == FrustumHit::Outside) 
        {
            // 밖이라면 건너뛰기
            ++m_StatCulledByFrustum; 
            continue; 
        }

        if (hit == FrustumHit::Inside)
        {
			// 노드가 절두체 안에 완전히 포함되면?
            PushNodeObjects_NoFrustum(node);
            continue;
        }

        // 노드가 절두체 경계에 걸쳐있다면?
        if (node.iObjCount > 0)
        {
			// 이 노드를 처리
            PushNodeObjects_WithFrustum(node);
        }
        if (!node.bIsLeaf)
        {
			// 자식 노드로 들어가서 재귀 탐색
            for (_uint c = 0; c < node.iChildCount; ++c)
                m_TempNodeStack.push_back(node.iFirstChild + c);
        }
    }
}

void COctoTree_Manager::PushNodeObjects_NoFrustum(const Node& node)
{
    // node가 m_Nodes 에서 몇번째 인덱스인지 확인
    const _uint startId = static_cast<_uint>(&node - &m_Nodes[0]);

	// 로컬 스택에 시작 노드 ID를 넣는다
    vector<_uint> localStack;
	localStack.reserve(64);
    localStack.push_back(startId);

    // 로컬 스택
    while (!localStack.empty())
    {
        const _uint id = localStack.back();
        localStack.pop_back();


        // 해당 노드에 들어있는 오브젝트들을 m_CulledStaticObjects 에 추가한다.
        const Node& n = m_Nodes[id];
        for (_uint i = 0; i < n.iObjCount; ++i)
        {
            // 해당 노드의 오브젝트 인덱스를 추출 후 중복방지 객체 추가
            const _uint objIdx = m_ObjectIndices[n.iFirstObj + i];
            PushIfNotSeenThisFrame(objIdx, m_CulledStaticObjects);
        }


		//만약 마지막 리프 노드가 아니라면 자식도 탐색한다.
        if (!n.bIsLeaf)
        {
            for (_uint c = 0; c < n.iChildCount; ++c)
                localStack.push_back(n.iFirstChild + c);
        }
    }
}
void COctoTree_Manager::PushNodeObjects_WithFrustum(const Node& node)
{
    //마지막 노드인데 경계라면 오브젝트 개별로 절두체와 비교 후 추가
    for (_uint i = 0; i < node.iObjCount; ++i)
    {
        const _uint objIdx = m_ObjectIndices[node.iFirstObj + i];
        const AABBBOX& a = m_StaticObjectBounds[objIdx];

        const FrustumHit oh = m_Frustum.OctoIsInAABB(a.vMin, a.vMax);
        if (oh != FrustumHit::Outside)
        {
            // 밖이 아니라면 추가
            PushIfNotSeenThisFrame(objIdx, m_CulledStaticObjects);
        }
        else
        {
            ++m_StatObjCulledByFrustum;
        }
    }
}



void COctoTree_Manager::FinalCullAndLOD()
{
}



void COctoTree_Manager::PushIfNotSeenThisFrame(_uint objIdx, vector<_uint>& out)
{
	// 오브젝트 아이디 혹시나 범위를 벗어나면 리사이즈(대비)
    if (objIdx >= m_LastSeenFrame.size()) 
        m_LastSeenFrame.resize(objIdx + 1, 0);


	// FrameId 는 계속 증가하므로, 해당 오브젝트가 현재 프레임에 처음 보이는 것이라면 out 에 추가
    if (m_LastSeenFrame[objIdx] != m_FrameId)
    {
        m_LastSeenFrame[objIdx] = m_FrameId;
        out.push_back(objIdx);
        ++m_StatPushedObjects;
    }
}

void COctoTree_Manager::DebugDrawCells()
{
    if (!m_DebugDrawCells)
        return;

    _matrix view = m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW);
    _matrix proj = m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ);
    m_Frustum.OctoBuild(view, proj);

    vector<DebugLine> lines; 
    lines.reserve(m_Nodes.size() * 12);

    for (const Node& n : m_Nodes)
    {
        // 프러스텀 결과(있으면) 색 다르게
        FrustumHit hit = m_Frustum.OctoIsInAABB(n.AABBBounds.vMin, n.AABBBounds.vMax);
        XMFLOAT4 col =
            (hit == FrustumHit::Inside) ? XMFLOAT4(0, 1, 0, 1) :
            (hit == FrustumHit::Intersect) ? XMFLOAT4(1, 1, 0, 1) :
            XMFLOAT4(0.4f, 0.4f, 0.4f, 1);

        // 방문 노드만 보고 싶으면 조건 추가:
        // if (!visitedThisFrame[n]) continue;

        PushAABBLines(n.AABBBounds, col, lines);
    }
    RenderDebugLines(lines);
}
void COctoTree_Manager::RenderDebugLines(const vector<DebugLine>& lines)
{
    using namespace DirectX;
    if (lines.empty())
        return;

    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
    m_pEffect->SetProjection(m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

    m_pContext->IASetInputLayout(m_pInputLayout);
    m_pEffect->Apply(m_pContext);

    m_pBatch->Begin();

    for (auto& ln : lines)
    {
        m_pBatch->DrawLine(
            VertexPositionColor(XMLoadFloat3(&ln.a), XMLoadFloat4(&ln.color)),
            VertexPositionColor(XMLoadFloat3(&ln.b), XMLoadFloat4(&ln.color))
        );
    }

    m_pBatch->End();
}

_bool COctoTree_Manager::ComputeWorldBounds(const vector<AABBBOX>& arr, AABBBOX& out)
{
    if (arr.empty()) return false;

    out = arr[0];
    for (size_t i = 1; i < arr.size(); ++i)
        AABB_ExpandByAABB(out, arr[i]);

    return true;
}
void COctoTree_Manager::InitIndexToHandle(const map<Handle, _uint>& handleToIndex, size_t count)
{
    m_StaticIndexToHandle.resize(count);

    for (auto& [h, idx] : handleToIndex)
    {
        if (idx >= m_StaticIndexToHandle.size())
            m_StaticIndexToHandle.resize(idx + 1);

        m_StaticIndexToHandle[idx] = h;
    }
}

HRESULT COctoTree_Manager::SetObjectType(const vector<OCTOTREEOBJECTTYPE>& vTypes)
{
    const _uint iExpected = static_cast<_uint>(m_StaticObjectBounds.size());
    const _uint iGiven = static_cast<_uint>(vTypes.size());

    // 인덱스 정렬(평행 배열) 보장 체크
    if (iExpected != iGiven)
    {
#ifdef _DEBUG
        OutputDebugStringW(L"[OctoTree] SetObjectType: size mismatch (Bounds vs Types)\n");
#endif
        return E_FAIL;
    }

    // 그대로 복사(동일 인덱스 정렬 유지)
    m_ObjectType = vTypes;
    return S_OK;
}

COctoTree_Manager* COctoTree_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    COctoTree_Manager* pInstance = new COctoTree_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : COctoTree_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void COctoTree_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);

    Safe_Release(m_pInputLayout);

    Safe_Delete(m_pBatch);
    Safe_Delete(m_pEffect);
    Safe_Release(m_pGameInstance);
}

void COctoTree_Manager::Frustum::BuildFromViewProj(const XMFLOAT4X4& vp)
{
    XMMATRIX matVP = XMLoadFloat4x4(&vp);
    XMMATRIX matInvVP = XMMatrixInverse(nullptr, matVP);

    // D3D NDC (z: 0~1)
    XMVECTOR vNDC[8] = {
        XMVectorSet(-1.f,-1.f,0.f,1.f), // 0 nlb
        XMVectorSet(1.f,-1.f,0.f,1.f), // 1 nrb
        XMVectorSet(1.f, 1.f,0.f,1.f), // 2 nrt
        XMVectorSet(-1.f, 1.f,0.f,1.f), // 3 nlt
        XMVectorSet(-1.f,-1.f,1.f,1.f), // 4 flb
        XMVectorSet(1.f,-1.f,1.f,1.f), // 5 frb
        XMVectorSet(1.f, 1.f,1.f,1.f), // 6 frt
        XMVectorSet(-1.f, 1.f,1.f,1.f)  // 7 flt
    };

    XMVECTOR vWorldCorner[8];
    for (int i = 0; i < 8; ++i) {
        XMVECTOR vHomo = XMVector4Transform(vNDC[i], matInvVP);
        vWorldCorner[i] = XMVectorScale(vHomo, 1.0f / XMVectorGetW(vHomo)); // perspective divide
    }

    auto MakePlane = [](XMVECTOR a, XMVECTOR b, XMVECTOR c) {
        return XMPlaneNormalize(XMPlaneFromPoints(a, b, c));
        };

    const XMVECTOR vNLB = vWorldCorner[0], vNRB = vWorldCorner[1],
        vNRT = vWorldCorner[2], vNLT = vWorldCorner[3];
    const XMVECTOR vFLB = vWorldCorner[4], vFRB = vWorldCorner[5],
        vFRT = vWorldCorner[6], vFLT = vWorldCorner[7];

    XMVECTOR vPlane[6];
    vPlane[0] = MakePlane(vNLB, vNLT, vFLB); // Left
    vPlane[1] = MakePlane(vNRT, vNRB, vFRT); // Right
    vPlane[2] = MakePlane(vNRB, vNLB, vFRB); // Bottom
    vPlane[3] = MakePlane(vNLT, vNRT, vFLT); // Top
    vPlane[4] = MakePlane(vNRT, vNLT, vNRB); // Near
    vPlane[5] = MakePlane(vFLT, vFRT, vFLB); // Far

    for (int i = 0; i < 6; ++i)
        DirectX::XMStoreFloat4(&planes[i].p, vPlane[i]);
}
FrustumHit COctoTree_Manager::Frustum::OctoIsInAABB(const _float3& bmin, const _float3& bmax) const
{
    const _float fOutsideEps = static_cast<_float>(1e-4f);

    const _float fPadNearWorld = static_cast<_float>(1.0f);
    const _float fPadOtherWorld = static_cast<_float>(0.15f); 
    const _float fPadRatio = static_cast<_float>(0.15f);

    XMVECTOR vMin = XMLoadFloat3(&bmin);
    XMVECTOR vMax = XMLoadFloat3(&bmax);
    XMVECTOR vCenter = 0.5f * (vMin + vMax);
    XMVECTOR vHalfExtent = 0.5f * (vMax - vMin);

    const _float fHalfDiag = XMVectorGetX(XMVector3Length(vHalfExtent));
    const _float fInsideSlack = -static_cast<_float>(0.35f) * fHalfDiag;

    _bool bAnyIntersect = false;

    for (int iPlane = 0; iPlane < 6; ++iPlane)
    {
        XMVECTOR vPlane = XMLoadFloat4(&planes[iPlane].p);
        XMVECTOR vNormal = XMVector3Normalize(XMVectorSetW(vPlane, 0.f));

        _float fSignedCenterDist = XMVectorGetX(XMVector3Dot(vNormal, vCenter)) + XMVectorGetW(vPlane);
        _float fProjRadius = XMVectorGetX(XMVector3Dot(XMVectorAbs(vNormal), vHalfExtent));

        fProjRadius += fPadRatio * fHalfDiag;

        if (iPlane == static_cast<_int>(PlaneID::Near))
        {
            fProjRadius += fPadNearWorld;
        }
        else
        {
            fProjRadius += fPadOtherWorld;
        }

        if (fSignedCenterDist + fProjRadius < -fOutsideEps)
            return FrustumHit::Outside;

        if (fSignedCenterDist - fProjRadius < fInsideSlack)
            bAnyIntersect = true;
    }

    return bAnyIntersect ? FrustumHit::Intersect : FrustumHit::Inside;
}
void COctoTree_Manager::Frustum::OctoBuild(const _matrix& view, const _matrix& proj)
{
    _matrix ViewProj = XMMatrixMultiply(view, proj);
    _float4x4 matVP;
    XMStoreFloat4x4(&matVP, ViewProj);

    //PrintMatrix(matVP, "VIEW*PROJ");

    BuildFromViewProj(matVP);
}
void COctoTree_Manager::PrintMatrix(const _float4x4& tMat, const _char* pLabel)
{
    printf("[%s]\n", pLabel);
    printf("% .6f % .6f % .6f % .6f\n", tMat._11, tMat._12, tMat._13, tMat._14);
    printf("% .6f % .6f % .6f % .6f\n", tMat._21, tMat._22, tMat._23, tMat._24);
    printf("% .6f % .6f % .6f % .6f\n", tMat._31, tMat._32, tMat._33, tMat._34);
    printf("% .6f % .6f % .6f % .6f\n\n", tMat._41, tMat._42, tMat._43, tMat._44);
}