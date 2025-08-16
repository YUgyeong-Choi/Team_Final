#include "Area_Manager.h"

#include "GameInstance.h"

CArea_Manager::CArea_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
    , m_pGameInstance{ CGameInstance::Get_Instance() }
{
    Safe_AddRef(m_pGameInstance);
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT CArea_Manager::Initialize()
{
    Reset_Parm();

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

HRESULT CArea_Manager::Reset_Parm()
{
    /* [ ��å �⺻�� ] */
    m_iWarmNeighbors = 3;
    m_fEnterGrace = 0.f;
    m_fExitDelay = 0.f;
    m_bUseHysteresis = false;

    /* [ ��Ÿ�� �⺻�� ] */
    m_iCurrentAreaId = static_cast<_int>(-1);
    m_iPrevAreaId = static_cast<_int>(-1);
    m_iFrameId = 0;
    m_bDebugDraw = false;

    /* [ �����̳� ���� ] */
    m_vecAreas.clear();
    m_vecActiveAreaIds.clear();
    m_vecWarmAreaIds.clear();
    m_vecAreaHandles.clear();
    m_mapAreaIdToIndex.clear();


    return S_OK;
}

_bool CArea_Manager::AddArea_AABB( _int iAreaId, const _float3& vMin, const _float3& vMax, const vector<_uint>& vecAdjacentIds,
    AREA::EAreaType eType, _int iPriority)
{
    /* [ ��ȿ�� �˻� ] */
    if (vMin.x > vMax.x || vMin.y > vMax.y || vMin.z > vMax.z)
        return false;
    if (m_mapAreaIdToIndex.find(iAreaId) != m_mapAreaIdToIndex.end())
        return false;

	/* [ ���� �Ӽ� ] */
    AREA tArea{};
    tArea.iAreaId = iAreaId;
    tArea.iAreaState = 0;
    tArea.vBounds.vMin = vMin;
    tArea.vBounds.vMax = vMax;
    tArea.vecAdjacent = vecAdjacentIds;
	tArea.eType = eType;
	tArea.iPriority = iPriority;

    m_vecAreas.push_back(tArea);

    return true;
}

HRESULT CArea_Manager::FinalizePartition()
{
	/* [ �ε����� ����迭�� ��� ] */
    m_mapAreaIdToIndex.clear();
    m_mapAreaIdToIndex.reserve(m_vecAreas.size());

    for (_uint iIndex = 0; iIndex < static_cast<_uint>(m_vecAreas.size()); ++iIndex)
    {
        const _int iAreaId = m_vecAreas[iIndex].iAreaId;
        if (m_mapAreaIdToIndex.find(iAreaId) != m_mapAreaIdToIndex.end())
            return E_FAIL;

        m_mapAreaIdToIndex[iAreaId] = iIndex;
    }

    return S_OK;
}


_int CArea_Manager::FindAreaContainingPoint(const _float3& vPoint) const
{
    auto FnVolume = [](const AREA& a) -> _double 
        {
        const _double dx = static_cast<_double>(a.vBounds.vMax.x - a.vBounds.vMin.x);
        const _double dy = static_cast<_double>(a.vBounds.vMax.y - a.vBounds.vMin.y);
        const _double dz = static_cast<_double>(a.vBounds.vMax.z - a.vBounds.vMin.z);
        return dx * dy * dz;
        };
    auto FnIsBetter = [&](const AREA& cand, const AREA* pBest) -> _bool 
        {
        if (pBest == nullptr) return true;
        if (cand.iPriority != pBest->iPriority) return (cand.iPriority > pBest->iPriority);
        const _double vC = FnVolume(cand), vB = FnVolume(*pBest);
        if (vC != vB) return (vC < vB); 
        return (cand.iAreaId < pBest->iAreaId);
        };

    /* [ �ش� ��ġ�� ��� ������ ��ġ�ϴ��� Ž�� ] */
    if (m_iCurrentAreaId != static_cast<_int>(-1))
    {
        auto it = m_mapAreaIdToIndex.find(m_iCurrentAreaId);
        if (it != m_mapAreaIdToIndex.end())
        {
            // ���� �����ȿ� �ִ��� Ž�� (��ƼŰ �켱)
            const AREA& tCur = m_vecAreas[it->second];
            if (tCur.ContainsPoint(vPoint))
                return m_iCurrentAreaId;

            // ���� ������ �� '���ԵǴ�' �ĺ� �߿��� iPriority ���� ����Ʈ ����
            const AREA* pBestAdj = nullptr;
            _int iBestAdjId = static_cast<_int>(-1);

            for (const _uint iAdjId : tCur.vecAdjacent)
            {
                auto itAdj = m_mapAreaIdToIndex.find(static_cast<_int>(iAdjId));
                if (itAdj != m_mapAreaIdToIndex.end())
                {
                    const AREA& tAdj = m_vecAreas[itAdj->second];
                    if (tAdj.ContainsPoint(vPoint))
                    {
                        if (FnIsBetter(tAdj, pBestAdj))
                        {
                            pBestAdj = &tAdj;
                            iBestAdjId = static_cast<_int>(iAdjId);
                        }
                    }
                }
            }
            if (pBestAdj != nullptr)
                return iBestAdjId;
        }
    }

    // ���� ��ĵ������ 'ù ��ġ ��ȯ' ��� iPriority�� ����Ʈ ����
    const AREA* pBest = nullptr;
    _int iBestId = static_cast<_int>(-1);

    for (const AREA& tArea : m_vecAreas)
    {
        if (tArea.ContainsPoint(vPoint))
        {
            if (FnIsBetter(tArea, pBest))
            {
                pBest = &tArea;
                iBestId = tArea.iAreaId;
            }
        }
    }

    if (pBest != nullptr)
        return iBestId;

    // �ƿ� ���ٸ� -1 ��ȯ
    return static_cast<_int>(-1);
}

void CArea_Manager::DebugDrawCells()
{
    if (!m_DebugDrawCells)
        return;

    _matrix view = m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW);
    _matrix proj = m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ);
    
    vector<DebugLine> lines;
    lines.reserve(m_vecAreas.size() * 12);

    for (const AREA& tArea : m_vecAreas)
    {
        XMFLOAT3 vPos;
        XMStoreFloat3(&vPos, m_vPlayerPos);        

        const _bool bOverlapped =
            tArea.ContainsPoint(vPos) && (tArea.iAreaId != m_iCurrentAreaId);

        XMFLOAT4 col = GetAreaDebugColor(
            tArea,
            m_iCurrentAreaId,
            bOverlapped
        );

        PushAABBLines(tArea.vBounds, col, lines);
    }
    RenderDebugLines(lines);
}
void CArea_Manager::RenderDebugLines(const vector<DebugLine>& lines)
{
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

XMFLOAT4 CArea_Manager::BaseColorByType(AREA::EAreaType eType)
{
    switch (eType)
    {
    case AREA::EAreaType::ROOM:    return XMFLOAT4(0.20f, 0.80f, 1.00f, 1.0f);
    case AREA::EAreaType::INDOOR:  return XMFLOAT4(0.60f, 0.40f, 0.95f, 1.0f);
    case AREA::EAreaType::LOBBY:   return XMFLOAT4(1.00f, 0.55f, 0.10f, 1.0f);
    case AREA::EAreaType::OUTDOOR: return XMFLOAT4(0.15f, 0.85f, 0.35f, 1.0f);
    default:                       return XMFLOAT4(0.70f, 0.70f, 0.70f, 1.0f);
    }
}
XMFLOAT4 CArea_Manager::Lerp(const XMFLOAT4& a, const XMFLOAT4& b, _float t)
{
    return XMFLOAT4(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    );
}
XMFLOAT4 CArea_Manager::GetAreaDebugColor(const AREA& tArea, _int iCurrentAreaId, _bool bOverlapped)
{
    // 1) �÷��̾ ����ִ� '��ǥ' ���� �� �׻� �ʷ� (�ֿ켱)
    if (tArea.iAreaId == iCurrentAreaId)
        return XMFLOAT4(0.20f, 1.00f, 0.20f, 1.00f);

    // 2) �⺻���� �켱���� ���
    XMFLOAT4 tCol = BaseColorByType(tArea.eType);

    // 3) ��ġ�� ������ ��¦ ���̶���Ʈ(����: ��⸸ ��¦ �ø�)
    if (bOverlapped)
        return Lerp(tCol, XMFLOAT4(1.f, 1.f, 1.f, 1.f), 0.25f);

    // 4) �Ϲ� ������ ������ �״��
    return tCol;
}

CArea_Manager* CArea_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CArea_Manager* pInstance = new CArea_Manager(pDevice, pContext);

    if (FAILED(pInstance->Initialize()))
    {
        MSG_BOX("Failed to Created : CArea_Manager");
        Safe_Release(pInstance);
    }

    return pInstance;
}
void CArea_Manager::Free()
{
    __super::Free();

    Safe_Release(m_pContext);
    Safe_Release(m_pDevice);

    Safe_Release(m_pInputLayout);

    Safe_Delete(m_pBatch);
    Safe_Delete(m_pEffect);
    Safe_Release(m_pGameInstance);
}