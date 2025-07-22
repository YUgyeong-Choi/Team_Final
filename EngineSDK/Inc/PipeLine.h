#pragma once

#include "Base.h"

/* ������ ������������ ��ȯ�� ���� ��, ��������� �����Ѵ�. */
/* ������ ������������ ��ȯ�� ���� ��, ��������� ������� ���, �����Ѵ�. */
/* ī�޶� ��ġ�� �����Ѥ�,���� ./*/

NS_BEGIN(Engine)

class CPipeLine final : public CBase
{
private:
	CPipeLine();
	virtual ~CPipeLine() = default;

public:
	void Set_Transform(D3DTS eState, _fmatrix TransformMatrix);
	const _float4x4* Get_Transform_Float4x4(D3DTS eState) const;
	_matrix Get_Transform_Matrix(D3DTS eState) const;

	const _float4x4* Get_Transform_Float4x4_Inv(D3DTS eState) const;
	_matrix Get_Transform_Matrix_Inv(D3DTS eState) const;

	const _float4* Get_CamPosition() const;


public:
	void Update();

private:
	_float4x4				m_TransformationMatrices[ENUM_CLASS(D3DTS::END)] = {};
	_float4x4				m_TransformationMatrixInverse[ENUM_CLASS(D3DTS::END)] = {};
	_float4					m_vCamPosition = {};

public:
	static CPipeLine* Create();
	virtual void Free() override;
};

NS_END