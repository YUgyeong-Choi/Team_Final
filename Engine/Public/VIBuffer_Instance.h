#pragma once

#include "VIBuffer.h"

/*
�ν��Ͻ��� �ʿ��� ��� �𵨵��� �θ� �ȴ�.
*/

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Instance abstract : public CVIBuffer
{
public:
    typedef struct tagInstanceDesc
    {
        _uint            iNumInstance;

    }INSTANCE_DESC;
protected:
    CVIBuffer_Instance(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CVIBuffer_Instance(const CVIBuffer_Instance& Prototype);
    virtual ~CVIBuffer_Instance() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

    virtual HRESULT Bind_Buffers() override;
    virtual HRESULT Render() override;

protected:
    ID3D11Buffer*               m_pVBInstance = { nullptr };
    D3D11_BUFFER_DESC           m_VBInstanceDesc = {};
    D3D11_SUBRESOURCE_DATA      m_VBInstanceSubresourceData = {};

    _uint                    m_iNumIndexPerInstance = {};
    _uint                    m_iNumInstance = {};
    _uint                    m_iVertexInstanceStride = {};

public:
    virtual CComponent* Clone(void* pArg) = 0;
    virtual void Free() override;
};

NS_END;