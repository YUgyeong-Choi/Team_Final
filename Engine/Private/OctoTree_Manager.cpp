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

/* [ �ش� ������ ��ü�� ���彺���̽����� AABB , �ڵ� ��(0�� Ű�� ����ִ� �ε�����ȣ) �� �޴´�. ] */
HRESULT COctoTree_Manager::Ready_OctoTree(const vector<AABBBOX>& staticBounds, const map<Handle, _uint>& handleToIndex)
{
    // 1) ���� ������ ������
    m_StaticObjectBounds = staticBounds;
    m_HandleToStaticIndex = handleToIndex;

    // ���/���� �غ�
    m_Nodes.clear();
    m_ObjectIndices.clear();
    m_VisibleCandidateNodes.clear();
    m_CulledStaticObjects.clear();
    m_LastSeenFrame.assign(m_StaticObjectBounds.size(), 0);
    m_ObjectLODRadii.resize(m_StaticObjectBounds.size());

    // 2) ���� �ٿ�� ��� (��� ���� AABB�� ���δ� �ڽ�)
    if (!ComputeWorldBounds(m_StaticObjectBounds, m_WorldBounds))
        return E_FAIL;

    // 3) LOD �ʱ� �ݰ� ���� (����: AABB �밢�� ����)
    for (_uint i = 0; i < static_cast<_uint>(m_StaticObjectBounds.size()); ++i)
        m_ObjectLODRadii[i] = AABB_HalfDiagonal(m_StaticObjectBounds[i]);

    // 4) Ʈ�� ����
    BuildTree();

    return S_OK;
}

void COctoTree_Manager::BuildTree()
{
    // ���� ū ûũ�� ����(�ʱ�ȭ)
    Node root{};
    root.AABBBounds = m_WorldBounds;
    root.iDepth = 0;
    root.iChildCount = 0;
    root.iFirstChild = 0;
    root.iFirstObj = 0;
    root.iObjCount = static_cast<_uint>(m_StaticObjectBounds.size());
    root.bIsLeaf = false;

    //�����ϴ� ������Ʈ�� ���� �ű��
    m_ObjectIndices.resize(root.iObjCount);
    for (_uint i = 0; i < root.iObjCount; ++i)
        m_ObjectIndices[i] = i;

	// ���� ûũ�� ��� ����Ʈ�� �߰�
    m_Nodes.push_back(root);

    // ���� ����
    SubdivideNode(0);
}
void COctoTree_Manager::SubdivideNode(_uint nodeIdx)
{
    _uint parent = nodeIdx;
    //Node& node = m_Nodes[nodeIdx];

    // ���� ���� (����, ������Ʈ ��, �ּһ�����)
    const _uint     iDepth = m_Nodes[parent].iDepth;
    const _uint     iObjCount = m_Nodes[parent].iObjCount;
    const _float3   fSize = AABB_Size(m_Nodes[parent].AABBBounds);
    const _float    fCellMin = min(fSize.x, min(fSize.y, fSize.z));

    if (iDepth >= m_iMaxDepth || iObjCount <= m_iMaxObjects || fCellMin <= m_fMinCellSize) {
        m_Nodes[parent].bIsLeaf = true;
        return;
    }




    // �ڽ� 8�� AABB ���� (XYZ ����)
    AABBBOX childBounds[8];
    MakeChildBoundsXYZ(m_Nodes[parent].AABBBounds, childBounds);

    const _uint firstChild = static_cast<_uint>(m_Nodes.size());
    m_Nodes.resize(firstChild + 8);



    // ���� �ڽĵ��� ������ش�.
    Node& node = m_Nodes[parent];
    node.iFirstChild = firstChild;
    node.iChildCount = 8;

    // ��ϵ� �ڽĿ� ���� ä���.
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



    // ���� ��忡 �Ҽӵ� ù ��ü�� ������ ��ü
    const _uint begin = node.iFirstObj;
    const _uint end = begin + node.iObjCount;



	// �ڽĳ���� ��ü ����Ʈ�� �غ�(�˳���)
    vector<_uint> childLists[8];
    for (int i = 0; i < 8; ++i) 
    {
        childLists[i].reserve(iObjCount / 8 + 4);
    }

	// �θ����� �����־���� ��ü ����Ʈ�� �غ�(�˳���)
    vector<_uint> remain; 
    remain.reserve(iObjCount / 2 + 4);


	// ��ü�� ��ȸ�ϸ� �ڽĿ� �Ҵ�
    for (_uint i = begin; i < end; ++i)
    {
        const _uint objIdx = m_ObjectIndices[i];
        const AABBBOX& ob = m_StaticObjectBounds[objIdx];

		// �ش� ��ü�� AABB �� ������ IN , OUT, ��ħ ���θ� �Ǵ�
        _int fullyContainedChild = ClassifyFullyContained(childBounds, ob);
        
		// ������ ���Ե� �ڽ��� �ִٸ� �ش� �ڽĿ� �߰�
        if (fullyContainedChild >= 0)
        {
            childLists[fullyContainedChild].push_back(objIdx);
        }
        else
        {
            // ��迡 ��ġ�ų� ���� �ڽİ� ��ġ�� ���� ��忡 �����
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


    /* [ ��ü ��й迡 ���� �ε���� ������ ] */

    _uint write = begin;
    // 1. ���� ��忡 ����� ��
    node.iFirstObj = write;
    node.iObjCount = static_cast<_uint>(remain.size());
    for (_uint iParrent : remain)
        m_ObjectIndices[write++] = iParrent;

    // 2. �ڽĵ鿡�� �Ѿ ��ü��
    for (int c = 0; c < 8; ++c) 
    {
        Node& child = m_Nodes[node.iFirstChild + c];
        child.iFirstObj = write;
        child.iObjCount = static_cast<_uint>(childLists[c].size());
        for (_uint iChild : childLists[c]) 
            m_ObjectIndices[write++] = iChild;
    }

    // �ڽ� ��� ����
    const _uint firstChilds = m_Nodes[nodeIdx].iFirstChild;
    for (int c = 0; c < 8; ++c)
    {
        // �ڽĵ��� ���� ��ȸ�ϸ� ��ü�� ��ϵǾ��ִٸ� ���
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

    // ���� Y
    for (int i = 0; i < 4; ++i) {
        out[i].vMin.y = parent.vMin.y;
        out[i].vMax.y = parent.vMax.y;
    }

    // LT (Left-Top: X-��, Z-��)
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

    // ---- ����
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

    // ---- ����
    // 4: LT (����)
    out[4].vMin = { tParent.vMin.x, vCenter.y,      tParent.vMin.z };
    out[4].vMax = { vCenter.x,      tParent.vMax.y, vCenter.z };

    // 5: RT (����)
    out[5].vMin = { vCenter.x,      vCenter.y,      tParent.vMin.z };
    out[5].vMax = { tParent.vMax.x, tParent.vMax.y, vCenter.z };

    // 6: RB (����)
    out[6].vMin = { vCenter.x,      vCenter.y,      vCenter.z };
    out[6].vMax = { tParent.vMax.x, tParent.vMax.y, tParent.vMax.z };

    // 7: LB (����)
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

    // �������� ����
    m_Frustum.OctoBuild(view, proj);

    // ��Ŷ �ʱ�ȭ
    m_VisibleCandidateNodes.clear();
    m_CulledStaticObjects.clear();
    m_TempNodeStack.clear();
    m_TempObjects.clear();

    // ��� �ʱ�ȭ
    m_StatVisitedNodes = 0;
    m_StatPushedObjects = 0;
    m_StatCulledByFrustum = 0;
}
void COctoTree_Manager::QueryVisible()
{
    if (m_Nodes.empty()) return;

    // Ʈ�� Ž�� ����
    m_TempNodeStack.clear();
    m_TempNodeStack.push_back(0);

    while (!m_TempNodeStack.empty())
    {
		// �����ֱٿ� ���� ��带 ���� �� ����
        _uint nodeId = m_TempNodeStack.back();
        m_TempNodeStack.pop_back();

        const Node& node = m_Nodes[nodeId];
        ++m_StatVisitedNodes;

        // �ش� ��带 �˻�
        FrustumHit hit = m_Frustum.OctoIsInAABB(node.AABBBounds.vMin, node.AABBBounds.vMax);


        // ���带 Ž�� �� ����� ���� ���� ���� ���� ����
        if (hit == FrustumHit::Outside) 
        {
            // ���̶�� �ǳʶٱ�
            ++m_StatCulledByFrustum; 
            continue; 
        }

        if (hit == FrustumHit::Inside)
        {
			// ��尡 ����ü �ȿ� ������ ���ԵǸ�?
            PushNodeObjects_NoFrustum(node);
            continue;
        }

        // ��尡 ����ü ��迡 �����ִٸ�?
        if (node.iObjCount > 0)
        {
			// �� ��带 ó��
            PushNodeObjects_WithFrustum(node);
        }
        if (!node.bIsLeaf)
        {
			// �ڽ� ���� ���� ��� Ž��
            for (_uint c = 0; c < node.iChildCount; ++c)
                m_TempNodeStack.push_back(node.iFirstChild + c);
        }
    }
}

void COctoTree_Manager::PushNodeObjects_NoFrustum(const Node& node)
{
    // node�� m_Nodes ���� ���° �ε������� Ȯ��
    const _uint startId = static_cast<_uint>(&node - &m_Nodes[0]);

	// ���� ���ÿ� ���� ��� ID�� �ִ´�
    vector<_uint> localStack;
	localStack.reserve(64);
    localStack.push_back(startId);

    // ���� ����
    while (!localStack.empty())
    {
        const _uint id = localStack.back();
        localStack.pop_back();


        // �ش� ��忡 ����ִ� ������Ʈ���� m_CulledStaticObjects �� �߰��Ѵ�.
        const Node& n = m_Nodes[id];
        for (_uint i = 0; i < n.iObjCount; ++i)
        {
            // �ش� ����� ������Ʈ �ε����� ���� �� �ߺ����� ��ü �߰�
            const _uint objIdx = m_ObjectIndices[n.iFirstObj + i];
            PushIfNotSeenThisFrame(objIdx, m_CulledStaticObjects);
        }


		//���� ������ ���� ��尡 �ƴ϶�� �ڽĵ� Ž���Ѵ�.
        if (!n.bIsLeaf)
        {
            for (_uint c = 0; c < n.iChildCount; ++c)
                localStack.push_back(n.iFirstChild + c);
        }
    }
}
void COctoTree_Manager::PushNodeObjects_WithFrustum(const Node& node)
{
    //������ ����ε� ����� ������Ʈ ������ ����ü�� �� �� �߰�
    for (_uint i = 0; i < node.iObjCount; ++i)
    {
        const _uint objIdx = m_ObjectIndices[node.iFirstObj + i];
        const AABBBOX& a = m_StaticObjectBounds[objIdx];

        const FrustumHit oh = m_Frustum.OctoIsInAABB(a.vMin, a.vMax);
        if (oh != FrustumHit::Outside)
        {
            // ���� �ƴ϶�� �߰�
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
	// ������Ʈ ���̵� Ȥ�ó� ������ ����� ��������(���)
    if (objIdx >= m_LastSeenFrame.size()) 
        m_LastSeenFrame.resize(objIdx + 1, 0);


	// FrameId �� ��� �����ϹǷ�, �ش� ������Ʈ�� ���� �����ӿ� ó�� ���̴� ���̶�� out �� �߰�
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
        // �������� ���(������) �� �ٸ���
        FrustumHit hit = m_Frustum.OctoIsInAABB(n.AABBBounds.vMin, n.AABBBounds.vMax);
        XMFLOAT4 col =
            (hit == FrustumHit::Inside) ? XMFLOAT4(0, 1, 0, 1) :
            (hit == FrustumHit::Intersect) ? XMFLOAT4(1, 1, 0, 1) :
            XMFLOAT4(0.4f, 0.4f, 0.4f, 1);

        // �湮 ��常 ���� ������ ���� �߰�:
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

    // �ε��� ����(���� �迭) ���� üũ
    if (iExpected != iGiven)
    {
#ifdef _DEBUG
        OutputDebugStringW(L"[OctoTree] SetObjectType: size mismatch (Bounds vs Types)\n");
#endif
        return E_FAIL;
    }

    // �״�� ����(���� �ε��� ���� ����)
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