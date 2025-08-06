#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_VolumeMesh final : public CVIBuffer
{
private:
	CVIBuffer_VolumeMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_VolumeMesh(const CVIBuffer_VolumeMesh& Prototype);
	virtual ~CVIBuffer_VolumeMesh() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

public:
	static CVIBuffer_VolumeMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END