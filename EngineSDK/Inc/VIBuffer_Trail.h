#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Trail final : public CVIBuffer
{
private:
	CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Trail(const CVIBuffer_Trail& Prototype);
	virtual ~CVIBuffer_Trail() = default;

public:
	virtual HRESULT Initialize_Prototype();
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
	vector<VTXPOS_TRAIL>	m_TrailNodes;
	_uint					m_iMaxNodeCount = { 256 }; // 임시로 개수 지정함
	_float					m_fLifeDuration = { 1.f };
	_bool					m_bTrailActive = true;

public:
	static CVIBuffer_Trail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END