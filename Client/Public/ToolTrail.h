#pragma once

#include "Client_Defines.h"
#include "SwordTrailEffect.h"

NS_BEGIN(Client)

class CToolTrail final : public CSwordTrailEffect
{
public:
	typedef struct tagToolTEDesc : public CSwordTrailEffect::DESC
	{
		_bool bLoadingInTool = { false };
	}DESC;
private:
	CToolTrail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CToolTrail(const CToolTrail& Prototype);
	virtual ~CToolTrail() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	virtual void Update_Tool(_float fTimeDelta, _float fCurFrame);
	void Change_TrailBuffer(void* pArg);
	_uint* Get_MaxNodeCount_Ptr() ;
	_float* Get_LifeDuration_Ptr();
	_int* Get_Subdivisions_Ptr() ;
	_float* Get_NodeInterval_Ptr();

private:
	virtual HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	_bool m_bLoadingInTool = { false };

	// 버퍼용 변수들
	_float					m_fLifeDuration = { 0.5f };
	_float					m_fNodeInterval = { 0.0166f }; // 60 FPS 기준, 1초에 60번 노드 추가
	_int					m_Subdivisions = 4; // 캣멀롬 보간을 위한 세분화 단계

public:
	static CToolTrail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

public:
	virtual json Serialize()override;
	virtual void Deserialize(const json& j)override;
};

NS_END