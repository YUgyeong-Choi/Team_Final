#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Trail final : public CVIBuffer
{
public:
	typedef struct tagTrailBufferDesc
	{
		_uint	iMaxNodeCount = { 300 };
		_uint	Subdivisions = { 4 };
		_float	fLifeDuration = { 0.5f };
		_float	fNodeInterval = { 0.0166f };
	}DESC;

private:
	CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Trail(const CVIBuffer_Trail& Prototype);
	virtual ~CVIBuffer_Trail() = default;

public:
	virtual HRESULT Initialize_Prototype(const DESC* pDesc);
	virtual HRESULT Initialize(void* pArg);
	void Update_Trail(const _float3& vInnerPos, const _float3& vOuterPos, _float fTimeDelta);
	HRESULT Update_Buffers();

	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

public:
	void Set_TrailActive(_bool bActive) { m_bTrailActive = bActive; if (bActive == true) m_TrailNodes.clear();}

#ifdef USE_IMGUI
	void Set_MaxNodeCount(_uint iCnt) { m_iMaxNodeCount = iCnt; }
	void Set_LifeDuration(_float fLifeDuration) { m_fLifeDuration = fLifeDuration; }
#endif

private:
	HRESULT Interpolate_TrailNodes();

private:
	vector<VTXPOS_TRAIL>	m_TrailNodes;
	_uint					m_iMaxNodeCount = { 300 }; // 임시로 개수 지정함
	_float					m_fLifeDuration = { 0.5f };
	_bool					m_bTrailActive = true;
	_float					m_fNodeAccTime = { 0.f };
	_float					m_fNodeInterval = { 0.0166f }; // 60 FPS 기준, 1초에 60번 노드 추가
	_int					m_Subdivisions = 4; // 캣멀롬 보간을 위한 세분화 단계


public:
	static CVIBuffer_Trail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const DESC* pDesc = nullptr);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END