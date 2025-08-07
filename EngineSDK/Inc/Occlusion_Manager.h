#pragma once

#include "Base.h"

NS_BEGIN(Engine)
class CVIBuffer_Cube;
class CShader;
class CPhysXActor;
class CGameObject;

class COcclusion_Manager final : public CBase
{
public:
    struct OCCLUSION_INFO
    {
        ID3D11Query* pQuery = nullptr;
        _bool bVisible = true;
    };
private:
    COcclusion_Manager(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual ~COcclusion_Manager() = default;
public:
    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    void Begin_Occlusion(CGameObject* pObj, CPhysXActor* pPhysX);
    void End_Occlusion(CGameObject* pObj);
    _bool IsVisible(CGameObject* pObj) const;

private:
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;

    CVIBuffer_Cube* m_pCubeBuffer = nullptr;
    CShader* m_pShader = nullptr;

    map<CGameObject*, OCCLUSION_INFO> m_mapOcclusion;
private:
    HRESULT Ready_States(); // BlendState, DepthStencilState µî

    ID3D11DepthStencilState* m_pDepthState = nullptr;
    ID3D11BlendState* m_pBlendState = nullptr;

public:
    static COcclusion_Manager* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual void Free() override;
};

NS_END