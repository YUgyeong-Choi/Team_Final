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
	_uint iNumMesh = m_pModelCom->Get_NumMeshes();

	m_CDFPerMesh.resize(iNumMesh); // 메쉬마다 CDF 배열을 저장할 컨테이너

	for (_uint iMesh = 0; iMesh < iNumMesh; iMesh++)
	{
		auto* pMesh = m_pModelCom->Get_Mesh(iMesh); // 메쉬 객체 가져오기
		const auto& vertices = pMesh->Get_Vertices(); // 정점 배열
		const auto& indices = pMesh->Get_Indices();   // 인덱스 배열 (삼각형 기준)

		size_t triCount = (indices) / 3;

		std::vector<float> areas(triCount);
		std::vector<float> cdf(triCount);

		// 1. 각 삼각형의 면적 계산
		for (size_t t = 0; t < triCount; t++)
		{
			XMFLOAT3 v0 = vertices[indices[t * 3 + 0]].vPosition;
			XMFLOAT3 v1 = vertices[indices[t * 3 + 1]].vPosition;
			XMFLOAT3 v2 = vertices[indices[t * 3 + 2]].vPosition;

			XMVECTOR e1 = XMLoadFloat3(&v1) - XMLoadFloat3(&v0);
			XMVECTOR e2 = XMLoadFloat3(&v2) - XMLoadFloat3(&v0);
			float area = 0.5f * XMVectorGetX(XMVector3Length(XMVector3Cross(e1, e2)));

			areas[t] = area;
		}

		// 2. 누적합(CDF) 계산
		float accum = 0.0f;
		for (size_t t = 0; t < triCount; t++)
		{
			accum += areas[t];
			cdf[t] = accum;
		}

		// 3. 메쉬별로 저장
		m_CDFPerMesh[iMesh] = std::move(cdf);

		// 필요하다면 총 면적도 따로 저장 가능
		// m_TotalAreaPerMesh[iMesh] = accum;
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
