#include "Occlusion_Manager.h"

#include "GameInstance.h"
#include "VIBuffer_Cube.h"
#include "Shader.h"
#include "PhysXActor.h"

COcclusion_Manager::COcclusion_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice{ pDevice }
    , m_pContext{ pContext }
{
    Safe_AddRef(m_pDevice);
    Safe_AddRef(m_pContext);
}

HRESULT COcclusion_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pCubeBuffer = CVIBuffer_Cube::Create(m_pDevice, m_pContext);
    if (FAILED(Ready_States()))
        return E_FAIL;

    // 컬링용 전용 쉐이더 로딩
    m_pShader = CShader::Create( m_pDevice,m_pContext, TEXT("../Bin/ShaderFiles/Shader_Occlusion.hlsl"), VTXCUBE::Elements, VTXCUBE::iNumElements);

    if (nullptr == m_pShader)
        return E_FAIL;

    return S_OK;
}

void COcclusion_Manager::Begin_Occlusion(CGameObject* pObj, CPhysXActor* pPhysX)
{
    if (!pObj || !pPhysX) return;

    auto& info = m_mapOcclusion[pObj];

    if (info.pQuery == nullptr)
    {
        D3D11_QUERY_DESC desc = { D3D11_QUERY_OCCLUSION, 0 };
        if (FAILED(m_pDevice->CreateQuery(&desc, &info.pQuery)))
            return;
    }

    PxBounds3 bounds = pPhysX->Get_Actor()->getWorldBounds();
    _float3 vCenter = { (bounds.minimum.x + bounds.maximum.x) * 0.5f,
                        (bounds.minimum.y + bounds.maximum.y) * 0.5f,
                        (bounds.minimum.z + bounds.maximum.z) * 0.5f };
    _float3 vScale = { (bounds.maximum.x - bounds.minimum.x),
                       (bounds.maximum.y - bounds.minimum.y),
                       (bounds.maximum.z - bounds.minimum.z) };

    // Matrix 세팅
    _matrix matWorld = XMMatrixScaling(vScale.x, vScale.y, vScale.z) * XMMatrixTranslation(vCenter.x, vCenter.y, vCenter.z);

    m_pContext->OMSetDepthStencilState(m_pDepthState, 0);
    m_pContext->OMSetBlendState(m_pBlendState, nullptr, 0xffffffff);

    m_pContext->Begin(info.pQuery);

    // 쉐이더 매트릭스 세팅
    m_pShader->Bind_RawValue("g_WorldMatrix", &matWorld, sizeof(_matrix));
    //m_pShader->Bind_RawValue("g_ViewMatrix", &CPipeLine::Get_Instance()->Get_TransformMatrix(CPipeLine::D3DTS_VIEW), sizeof(_matrix));
    //m_pShader->Bind_RawValue("g_ProjMatrix", &CPipeLine::Get_Instance()->Get_TransformMatrix(CPipeLine::D3DTS_PROJ), sizeof(_matrix));

    m_pShader->Begin(0); // PS는 안 써도 됨
    m_pCubeBuffer->Render();

    m_pContext->End(info.pQuery);
}

void COcclusion_Manager::End_Occlusion(CGameObject* pObj)
{
    if (!pObj) return;

    auto iter = m_mapOcclusion.find(pObj);
    if (iter == m_mapOcclusion.end()) return;

    UINT64 visiblePixels = 0;
    while (S_FALSE == m_pContext->GetData(iter->second.pQuery, &visiblePixels, sizeof(UINT64), 0));

    iter->second.bVisible = (visiblePixels > 0);
}

_bool COcclusion_Manager::IsVisible(CGameObject* pObj) const
{
    auto iter = m_mapOcclusion.find(pObj);
    if (iter == m_mapOcclusion.end()) return true;
    return iter->second.bVisible;
}

HRESULT COcclusion_Manager::Ready_States()
{
    D3D11_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
    if (FAILED(m_pDevice->CreateBlendState(&blendDesc, &m_pBlendState)))
        return E_FAIL;

    D3D11_DEPTH_STENCIL_DESC dsDesc{};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    if (FAILED(m_pDevice->CreateDepthStencilState(&dsDesc, &m_pDepthState)))
        return E_FAIL;

    return S_OK;
}
COcclusion_Manager* COcclusion_Manager::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new COcclusion_Manager(pDevice, pContext);
}

void COcclusion_Manager::Free()
{
    __super::Free();

    // 쿼리 해제
    for (auto& pair : m_mapOcclusion)
    {
        if (pair.second.pQuery)
            pair.second.pQuery->Release();
    }
    m_mapOcclusion.clear();

    // 쉐이더 해제
    Safe_Release(m_pShader);

    // 큐브 버퍼 해제
    Safe_Release(m_pCubeBuffer);

    // 상태 객체 해제
    Safe_Release(m_pBlendState);
    Safe_Release(m_pDepthState);

    // 디바이스/컨텍스트 해제
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);
}
