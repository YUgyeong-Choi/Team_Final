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

void CMeshEmitter::Create_CDF()
{
    m_iNumMesh = m_pModelCom->Get_NumMeshes();
    m_CDFPerMesh.resize(m_iNumMesh); // 메쉬마다 CDF 배열을 저장할 컨테이너

    for (_uint iMesh = 0; iMesh < m_iNumMesh; ++iMesh)
    {
        auto* pMesh = m_pModelCom->Get_Mesh(iMesh);
        const auto& vertices = pMesh->Get_Vertices();
        const auto& indices = pMesh->Get_Indices();
        const _uint iNumIndices = pMesh->Get_NumIndices();

        const _uint triCount = iNumIndices / 3;
        if (triCount == 0) // 삼각형이 없는 메쉬는 건너뛰기
            continue;

        // CDF 벡터를 필요한 크기로 미리 할당
        vector<_float> cdf(triCount);
        _float totalArea = 0.0f;

        // 1. 면적 계산과 누적합(CDF) 계산을 한 번의 루프로 통합
        for (_uint t = 0; t < triCount; ++t)
        {
            // 인덱스로부터 버텍스 위치 가져오기
            _float3 v0 = vertices[indices[t * 3 + 0]];
            _float3 v1 = vertices[indices[t * 3 + 1]];
            _float3 v2 = vertices[indices[t * 3 + 2]];

            // 면적 계산
            _vector e1 = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);
            _vector e2 = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
            _float area = 0.5f * XMVectorGetX(XMVector3Length(XMVector3Cross(e1, e2)));

            // 누적합 계산
            totalArea += area;
            cdf[t] = totalArea;
        }

        // 2. CDF 정규화 (0.0 ~ 1.0 사이 값으로)
        if (totalArea > 1e-6f) // 총 면적이 0에 가까운 경우(예: 모든 삼각형이 퇴화) 나누기 방지
        {
            for (_uint t = 0; t < triCount; ++t)
            {
                cdf[t] /= totalArea;
            }
        }

        // 3. 메쉬별로 저장
        m_CDFPerMesh[iMesh] = std::move(cdf);
    }
}

_float CMeshEmitter::CalcTriangleArea(const _float3& v0, const _float3& v1, const _float3& v2)
{
	//_vector e1 = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);
	//_vector e2 = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
	//return 0.5f * XMVectorGetX(XMVector3Length(XMVector3Cross(e1, e2)));
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
