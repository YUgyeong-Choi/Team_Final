#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CShadow final : public CBase
{
public:
	typedef struct tagShadowDesc
	{
		_float4			vEye, vAt;
		_float			fFovy, fNear, fFar;
	}SHADOW_DESC;

private:
	CShadow(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CShadow() = default;

public:
	// View Matrix
	const _float4x4* Get_LightViewMatrix(SHADOW eShadow) const {
		return &m_LightViewMatrix[ENUM_CLASS(eShadow)];
	}
	// Proj Matrix
	const _float4x4* Get_LightProjMatrix(SHADOW eShadow) const {
		return &m_LightProjMatrix[ENUM_CLASS(eShadow)];
	}

public:
	HRESULT Ready_Light_For_Shadow(const SHADOW_DESC& Desc, SHADOW eShadow);

private:
	ID3D11Device*					m_pDevice = { nullptr };
	ID3D11DeviceContext*			m_pContext = { nullptr };

private:
	_float4x4						m_LightViewMatrix[ENUM_CLASS(SHADOW::SHADOW_END)] = {};
	_float4x4						m_LightProjMatrix[ENUM_CLASS(SHADOW::SHADOW_END)] = {};

private:
	_float m_AspectRatio = {};

public:
	static CShadow* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free();

};

NS_END