#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	typedef struct tagTransformDesc
	{
		_float			fSpeedPerSec;
		_float			fRotationPerSec;
	}TRANSFORM_DESC;

protected:
	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransform(const CTransform& Prototype);
	virtual ~CTransform() = default;

public:
	_vector Get_State(STATE eState) {
		return XMLoadFloat4x4(&m_WorldMatrix).r[ENUM_CLASS(eState)];
	}

	_float3 Get_Scaled();

	_matrix Get_WorldMatrix_Inverse() {
		return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));
	}

	const _float4x4* Get_WorldMatrix_Ptr() {
		return &m_WorldMatrix;
	}

	void Set_State(STATE eState, _fvector vState)
	{
		XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[ENUM_CLASS(eState)]), vState);
	}

	void Set_SpeedPreSec(_float _fSpeeed) { m_fSpeedPerSec = _fSpeeed;}
	void Set_RotationPreSec(_float _fSpeeed) { m_fRotationPerSec = _fSpeeed; }

	_float4x4& Get_World4x4() {
		return m_WorldMatrix;
	}
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);

public:
	void Scaling(_float fX = 1.f, _float fY = 1.f, _float fZ = 1.f);
	void Scaling(const _float3& vScale);
public:
	void Go_Straight(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	void Go_Backward(_float fTimeDelta);
	void Go_Right(_float fTimeDelta);
	void Go_Left(_float fTimeDelta);
	void Go_Target(_fvector vTarget, _float fTimeDelta, _float fMinDistance);
	void Turn(_fvector vAxis, _float fTimeDelta);
	void Rotation(_fvector vAxis, _float fRadian);
	void Rotation(_float fX, _float fY, _float fZ);

public:
	/* [ ������ (�׺�) ] */
	void Go_Front(_float fTimeDelta, CNavigation* pNavigation = nullptr);
	/* [ ������ ��ġ�� ������ ȸ�� �� �̵� ] */
	bool Go_FrontByPosition(_float fTimeDelta, _vector vPosition, CNavigation* pNavigation = nullptr);

public:
	/* [ ���� (���� ���޽� ����) ] */
	bool Go_UpCustom(_float fTimeDelta, _float fSpeed, _float fMaxHight);
	/* [ �Ʒ��� (���� ���޽� ����) ] */
	void Go_DownCustom(_float fTimeDelta, _float fSpeed);

public:
	/* [ �������� �̵� (�׺�) ] */
	void Go_Dir(const _vector& vMoveDir, _float fTimeDelta, CNavigation* pNavigation = nullptr);

public:
	/* [ ���ʵ��� ����������� �ŭ �̵��ҰŴ�? ] */
	bool Move_Special(_float fTimeDelta, _float fTime, _vector& vMoveDir, _float fDistance, CNavigation* pNavigation = nullptr);
	/* [ ���ʵ��� Ÿ�� �����ϸ�ŭ �����ϵȴ�. ] */
	bool Scale_Special(_float fTimeDelta, _float fTime, _vector vTargetScale);
	/* [ ���ʵ��� ����������� ������� �������� �ŭ ȸ���ҰŴ�? ] */
	bool Rotate_Special(_float fTimeDelta, _float fTime, _vector vAxis, _float fAngleDegree);
	/* [ ��ǥ�������� �󸶸�ŭ ���̷� �����Ѵ� ] */
	bool JumpToTarget(_float fTimeDelta, _vector vTargetPos, _float fJumpHeight, _float fJumpTime);

public:
	/* [ õõ�� ȸ���Ѵ� ] */
	void RotationTimeDelta(_float fTimeDelta, _fvector Axis, _float fSpeedPerSec);
	/* [ �ش� �������� ��� ȸ���Ѵ� ] */
	bool RotateToDirectionImmediately(const _fvector& vTargetDir);

public:
	/* [ Y���� �����ϰ� �ٶ󺻴� ] */
	void LookAtWithOutY(_fvector vAt);
	/* [ Y���� �����ϰ� �Ѿư��� (�׺�) ] */
	bool ChaseWithOutY(_vector& vTargetPos, _float fTimeDelta, _float fMinDistance, CNavigation* pNavigation = nullptr);
	/* [ �Ѿư��� ] */
	bool ChaseCustom(const _fvector vTargetPos, _float fTimeDelta, _float fMinDistance, _float fSpeed);

public:
	/* [ Y���� �����ϰ� ������ȴ� ] */
	void BillboardToCameraY(_fvector vCameraPos);
	/* [ ��ü�� �������Ѵ�. ] */
	void BillboardToCameraFull(_fvector vCameraPos);

public:
	/* [ �������� ����Ѵ� ] */
	_float3 Compute_Scaled();
	/* [ �������� �����Ѵ� ] */
	void SetUp_Scale(_float fScaleX, _float fScaleY, _float fScaleZ);
	/* [ �������� �����´� ] */
	_vector Get_Scale() const;


public:
	void LookAt(_fvector vAt);

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName);

private:
	_float4x4				m_WorldMatrix = {};
	_float					m_fSpeedPerSec = {};
	_float					m_fRotationPerSec = {};

private:
	// �̵� ���� ���������
	_float  m_fSpecialMoveElapsed = 0.f;
	_float  m_fSpecialMoveDuration = 0.f;
	_vector m_vSpecialMoveOffset = {};
	_vector m_fSpecialMoveStartPos = {};
	_bool	m_bSpecialMoving = {};

	// ȸ���� ��� ������
	_bool m_bSpecialRotating = false;
	_float m_fSpecialRotateElapsed = 0.f;
	_float m_fSpecialRotateDuration = 0.f;
	_float m_fSpecialRotateAngle = 0.f;
	_vector m_vSpecialRotateAxis = {};
	_matrix m_matSpecialRotateStart = {};

	_vector m_vOriginalRight = { 1.f, 0.f, 0.f };
	_vector m_vOriginalUp = { 0.f, 1.f, 0.f };
	_vector m_vOriginalLook = { 0.f, 0.f, 1.f };

	//���� ����
	_vector m_vSpecialMoveStartPos = {};
	_vector m_vSpecialMoveTargetPos = {};

	//������ ����
	_bool m_bScaling = false;
	_float m_fScaleElapsed = 0.f;
	_float m_fScaleDuration = 0.f;
	_vector m_vStartScale = {};
	_vector m_vTargetScale = {};

public:
	static CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END