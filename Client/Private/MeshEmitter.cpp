#include "MeshEmitter.h"

#include "GameInstance.h"

CMeshEmitter::CMeshEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CParticleEffect{ pDevice, pContext }
{

}

CMeshEmitter::CMeshEmitter(const CMeshEmitter& Prototype)
	: CParticleEffect(Prototype)
{

}


HRESULT CMeshEmitter::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CMeshEmitter::Initialize(void* pArg)
{
	DESC* pDesc = static_cast<DESC*>(pArg);

	m_pModelCom = static_cast<CModel*>(pDesc->pOwner->Get_Component(TEXT("Com_Model")));
	//pDesc->pOwner;
	
	Create_CDF();

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

void CMeshEmitter::Priority_Update(_float fTimeDelta)
{
	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
	for (_uint i = 0; i < iNumMesh; i++)
	{
		//m_pShaderCom->Bind_RawValue("g_fEmissiveIntensity", &m_fEmissive, sizeof(_float));

		m_pModelCom->Bind_Bone_Matrices(m_pShaderCom, "g_BoneMatrices", i);

	}
}

void CMeshEmitter::Update(_float fTimeDelta)
{
}

void CMeshEmitter::Late_Update(_float fTimeDelta)
{
}

HRESULT CMeshEmitter::Render()
{
	return S_OK;
}

_float CMeshEmitter::Ready_Death()
{
	return _float();
}

void CMeshEmitter::Spawn_Particles()
{

}

HRESULT CMeshEmitter::Ready_Components()
{
	return S_OK;
}

HRESULT CMeshEmitter::Bind_ShaderResources()
{
	return S_OK;
}

//void CMeshEmitter::Create_CDF()
//{
//	_uint		iNumMesh = m_pModelCom->Get_NumMeshes();
//	for (_uint i = 0; i < iNumMesh; i++)
//	{
//		//CalcTriangleArea()
//	}
//}
//
//void CMeshEmitter::Prepare_EmitterData()
//{
//    // 이전 계산 결과 초기화
//    m_AllVertices.clear();
//    m_AllIndices.clear();
//    m_AllCDFs.clear();
//    m_MeshAreaCDF.clear();
//    m_MeshInfos.clear();
//
//    const uint32_t iNumMesh = m_pModelCom->Get_NumMeshes();
//    m_MeshInfos.resize(iNumMesh);
//
//    std::vector<float> meshTotalAreas;
//    meshTotalAreas.resize(iNumMesh);
//
//    float modelTotalArea = 0.0f;
//
//    // --- 메쉬별 데이터 처리 ---
//    for (uint32_t iMesh = 0; iMesh < iNumMesh; ++iMesh)
//    {
//        auto* pMesh = m_pModelCom->Get_Mesh(iMesh);
//        const _uint iNumVertices = pMesh->Get_NumVertices();
//        const _uint iNumIndices = pMesh->Get_NumIndices();
//        const _uint triCount = iNumIndices / 3;
//
//        const _float3* vertices = pMesh->Get_Vertices(); // _float3* 배열
//        const _uint* indices = pMesh->Get_Indices();  // _uint* 배열
//
//        // --- 오프셋 기록 ---
//        m_MeshInfos[iMesh].vertexOffset = static_cast<_uint>(m_AllVertices.size());
//        m_MeshInfos[iMesh].indexOffset = static_cast<_uint>(m_AllIndices.size());
//        m_MeshInfos[iMesh].cdfOffset = static_cast<_uint>(m_AllCDFs.size());
//        m_MeshInfos[iMesh].triangleCount = triCount;
//
//        // --- 버텍스 복사 ---
//        for (_uint i = 0; i < iNumVertices; ++i)
//        {
//            m_AllVertices.push_back(vertices[i]);
//        }
//
//        // --- 인덱스 복사 (글로벌 인덱스로 변환) ---
//        for (_uint i = 0; i < iNumIndices; ++i)
//        {
//            _uint localIndex = indices[i];
//            _uint globalIndex = m_MeshInfos[iMesh].vertexOffset + localIndex;
//            m_AllIndices.push_back(globalIndex);
//        }
//
//        // --- 개별 메쉬 CDF 계산 ---
//        float meshArea = 0.0f;
//        if (triCount > 0)
//        {
//            std::vector<float> tempCDF(triCount);
//            for (uint32_t t = 0; t < triCount; ++t)
//            {
//                // 삼각형 버텍스
//                _float3 v0 = vertices[indices[t * 3 + 0]];
//                _float3 v1 = vertices[indices[t * 3 + 1]];
//                _float3 v2 = vertices[indices[t * 3 + 2]];
//
//                // 면적 계산
//                _vector e1 = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);
//                _vector e2 = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
//                float area = 0.5f * XMVectorGetX(XMVector3Length(XMVector3Cross(e1, e2)));
//
//                meshArea += area;
//                tempCDF[t] = meshArea;
//            }
//
//            // CDF 정규화
//            if (meshArea > 1e-6f)
//            {
//                for (uint32_t t = 0; t < triCount; ++t)
//                    tempCDF[t] /= meshArea;
//            }
//
//            // 전체 CDF 배열에 추가
//            m_AllCDFs.insert(m_AllCDFs.end(), tempCDF.begin(), tempCDF.end());
//        }
//
//        meshTotalAreas[iMesh] = meshArea;
//        modelTotalArea += meshArea;
//    }
//
//    // --- 메쉬 선택용 CDF 생성 ---
//    float accum = 0.0f;
//    for (uint32_t iMesh = 0; iMesh < iNumMesh; ++iMesh)
//    {
//        accum += meshTotalAreas[iMesh];
//        m_MeshAreaCDF.push_back(accum);
//    }
//
//    // --- 메쉬 선택용 CDF 정규화 ---
//    if (modelTotalArea > 1e-6f)
//    {
//        for (float& val : m_MeshAreaCDF)
//            val /= modelTotalArea;
//    }
//}

void CMeshEmitter::Prepare_EmitterData(_uint totalParticles)
{
    m_EmitInfos.clear();

    const _uint iNumMesh = m_pModelCom->Get_NumMeshes();
    m_EmitInfos.resize(iNumMesh);

    _float modelTotalArea = 0.0f;

    // 1. 메쉬별 삼각형 개수와 면적 합 계산
    for (_uint iMesh = 0; iMesh < iNumMesh; ++iMesh)
    {
        auto* pMesh = m_pModelCom->Get_Mesh(iMesh);
        const _uint iNumIndices = pMesh->Get_NumIndices();
        const _uint triCount = iNumIndices / 3;

        const _uint* indices = pMesh->Get_Indices();
        const _float3* vertices = pMesh->Get_Vertices();

        float meshArea = 0.0f;

        for (_uint t = 0; t < triCount; ++t)
        {
            _float3 v0 = vertices[indices[t * 3 + 0]];
            _float3 v1 = vertices[indices[t * 3 + 1]];
            _float3 v2 = vertices[indices[t * 3 + 2]];

            _vector e1 = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);
            _vector e2 = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
            float area = 0.5f * XMVectorGetX(XMVector3Length(XMVector3Cross(e1, e2)));

            meshArea += area;
        }

        m_EmitInfos[iMesh].triangleCount = triCount;
        m_EmitInfos[iMesh].meshArea = meshArea;

        modelTotalArea += meshArea;
    }

    // 2. 메쉬별 파티클 수 비례 배분 (면적 기반)
    _uint sumAssigned = 0;
    for (_uint i = 0; i < iNumMesh; ++i)
    {
        if (modelTotalArea > 1e-6f)
        {
            float ratio = m_EmitInfos[i].meshArea / modelTotalArea;
            m_EmitInfos[i].assignedCount = static_cast<_uint>(ratio * totalParticles);
        }
        else
        {
            m_EmitInfos[i].assignedCount = 0;
        }
        sumAssigned += m_EmitInfos[i].assignedCount;
    }

    // 3. 파티클 수 합이 totalParticles와 안 맞으면 보정
    while (sumAssigned < totalParticles)
    {
        // 면적이 제일 큰 메쉬에 하나씩 더 분배
        auto it = std::max_element(
            m_EmitInfos.begin(), m_EmitInfos.end(),
            [](const EMITINFO& a, const EMITINFO& b) { return a.meshArea < b.meshArea; });

        it->assignedCount++;
        sumAssigned++;
    }
}

_float CMeshEmitter::CalcTriangleArea(const _float3& v0, const _float3& v1, const _float3& v2)
{
	_vector e1 = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);
	_vector e2 = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
	return 0.5f * XMVectorGetX(XMVector3Length(XMVector3Cross(e1, e2)));
}

CMeshEmitter* CMeshEmitter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CMeshEmitter* pInstance = new CMeshEmitter(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CMeshEmitter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CMeshEmitter::Clone(void* pArg)
{
	CMeshEmitter* pInstance = new CMeshEmitter(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CMeshEmitter");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMeshEmitter::Free()
{
	__super::Free();

	Safe_Release(m_pModelCom);
}

json CMeshEmitter::Serialize()
{
	return json();
}

void CMeshEmitter::Deserialize(const json& j)
{
}
