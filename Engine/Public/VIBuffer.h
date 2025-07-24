#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer abstract : public CComponent
{
protected:
	CVIBuffer(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CVIBuffer(const CVIBuffer& Prototype);
	virtual ~CVIBuffer() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

	virtual HRESULT Bind_Buffers();
	virtual HRESULT Render();

	const _float3* Get_Vertices() const { return m_pVertexPositions; }
	const _uint* Get_Indices() const { return m_pIndices; }
	_uint Get_NumVertices() const { return m_iNumVertices; }
	_uint Get_NumIndices() const { return m_iNumIndices; }

protected:
	ID3D11Buffer* m_pVB = { nullptr };
	ID3D11Buffer* m_pIB = { nullptr };
protected:
	_float3* m_pVertexPositions = {};
	_uint* m_pIndices = {};

protected:
	_uint						m_iNumVertices = {};
	_uint						m_iVertexStride = {};
	_uint						m_iNumIndices = {};
	_uint						m_iIndexStride = {};
	_uint						m_iNumVertexBuffers = {};
	DXGI_FORMAT					m_eIndexFormat = {};
	D3D11_PRIMITIVE_TOPOLOGY	m_ePrimitiveTopology = {};

public:
	virtual CComponent* Clone(void* pArg) = 0;
	virtual void Free() override;

};

NS_END