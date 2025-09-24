#pragma once

#include "Client_Defines.h"
#include "ParticleEffect.h"

NS_BEGIN(Engine)
class CModel;
NS_END

NS_BEGIN(Client)

class CMeshEmitter final : public CParticleEffect
{
public:
	typedef struct tagMeshEmitterDesc : public CParticleEffect::DESC
	{
		class CGameObject* pOwner = { nullptr };
	}DESC;

	//typedef struct tagMeshInfo
	//{
	//	_uint vertexOffset;
	//	_uint indexOffset;
	//	_uint cdfOffset;
	//	_uint triangleCount;
	//}EM_MESHINFO;

	typedef struct tagMeshEmitInfo
	{
		_uint   triangleCount = 0;		// 메쉬 삼각형 개수
		_float  meshArea = 0.0f;		// 메쉬 전체 면적
		_uint   assignedCount = 0;		// 배정된 파티클 수
	}EMITINFO;

private:
	CMeshEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMeshEmitter(const CMeshEmitter& Prototype);
	virtual ~CMeshEmitter() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual _float Ready_Death();
	virtual void Pause() { m_isActive = false; }

public:
	void Spawn_Particles();
	//void 

private:
	CModel*					m_pModelCom = { nullptr };
	vector<vector<_float>>	m_CDFPerMesh;
	//vector<_float>			m_
	_uint					m_iNumMesh = {};


private:
	vector<VTXANIMMESH>		m_AllVertices;       // 모든 메쉬의 버텍스 통합본
	vector<_uint>			m_AllIndices;        // 모든 메쉬의 인덱스 통합본
	vector<_float>			m_AllCDFs;           // 모든 메쉬의 CDF 통합본
	vector<_float>			m_MeshAreaCDF;       // 메쉬 선택용 CDF
	//vector<EM_MESHINFO>		m_MeshInfos;         // 메쉬별 오프셋 정보
	vector<EMITINFO>		m_EmitInfos;         // 메쉬별 오프셋 정보

private:
	virtual HRESULT Ready_Components() override;
	virtual HRESULT Bind_ShaderResources() override;

private:
	void Prepare_EmitterData(_uint totalParticles);
	void Create_CDF();
	_float CalcTriangleArea(const _float3& v0, const _float3& v1, const _float3& v2);

public:
	static CMeshEmitter* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize()override;
	virtual void Deserialize(const json& j)override;
};

NS_END
