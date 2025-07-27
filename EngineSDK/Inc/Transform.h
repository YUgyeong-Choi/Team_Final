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

	_matrix Get_WorldMatrix() {
		return XMLoadFloat4x4(&m_WorldMatrix);
	}
	
	//월드매트릭스 대입하는 것(맵 오브젝트 배치 할 때 필요 해서 넣음)
	void Set_WorldMatrix(const _float4x4& WorldMatrix) {
		m_WorldMatrix = WorldMatrix;
	}
	void Set_WorldMatrix(const _fmatrix& WorldMatrix) {
		XMStoreFloat4x4(&m_WorldMatrix, WorldMatrix);
	}

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
	/* [ 방향으로 이동 ] */
	void Move(const _vector& vDirectionVector);
	/* [ 앞으로 (네비) ] */
	void Go_Front(_float fTimeDelta, CNavigation* pNavigation = nullptr);
	/* [ 지정된 위치로 앞으로 회전 후 이동 ] */
	bool Go_FrontByPosition(_float fTimeDelta, _vector vPosition, CNavigation* pNavigation = nullptr);

public:
	/* [ 위로 (고점 도달시 정지) ] */
	bool Go_UpCustom(_float fTimeDelta, _float fSpeed, _float fMaxHight);
	/* [ 아래로 (저점 도달시 정지) ] */
	void Go_DownCustom(_float fTimeDelta, _float fSpeed);

public:
	/* [ 방향으로 이동 (네비) ] */
	void Go_Dir(const _vector& vMoveDir, _float fTimeDelta, CNavigation* pNavigation = nullptr);

public:
	/* [ 몇초동안 어느방향으로 몇만큼 이동할거니? ] */
	bool Move_Special(_float fTimeDelta, _float fTime, _vector& vMoveDir, _float fDistance, CNavigation* pNavigation = nullptr);
	/* [ 몇초동안 타겟 스케일만큼 스케일된다. ] */
	bool Scale_Special(_float fTimeDelta, _float fTime, _vector vTargetScale);
	/* [ 몇초동안 어느방향으로 어느축을 기준으로 몇만큼 회전할거니? ] */
	bool Rotate_Special(_float fTimeDelta, _float fTime, _vector vAxis, _float fAngleDegree);
	/* [ 목표지점까지 얼마만큼 높이로 점프한다 ] */
	bool JumpToTarget(_float fTimeDelta, _vector vTargetPos, _float fJumpHeight, _float fJumpTime);

public:
	/* [ 천천히 회전한다 ] */
	void RotationTimeDelta(_float fTimeDelta, _fvector Axis, _float fSpeedPerSec);
	/* [ 해당 방향으로 즉시 회전한다 ] */
	bool RotateToDirectionImmediately(const _fvector& vTargetDir);
	/* [ 쿼터니언 회전 ] */
	void Quaternion_Turn(const _vector& vAngle);

public:
	/* [ Y축을 제외하고 바라본다 ] */
	void LookAtWithOutY(_fvector vAt);
	/* [ Y축을 제외하고 쫓아간다 (네비) ] */
	bool ChaseWithOutY(_vector& vTargetPos, _float fTimeDelta, _float fMinDistance, CNavigation* pNavigation = nullptr);
	/* [ 쫓아간다 ] */
	bool ChaseCustom(const _fvector vTargetPos, _float fTimeDelta, _float fMinDistance, _float fSpeed);

public:
	/* [ Y축을 고정하고 빌보드된다 ] */
	void BillboardToCameraY(_fvector vCameraPos);
	/* [ 전체를 빌보드한다. ] */
	void BillboardToCameraFull(_fvector vCameraPos);

public:
	/* [ 스케일을 계산한다 ] */
	_float3 Compute_Scaled();
	/* [ 스케일을 적용한다 ] */
	void SetUp_Scale(_float fScaleX, _float fScaleY, _float fScaleZ);
	/* [ 스케일을 가져온다 ] */
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
	// 이동 전용 멤버변수들
	_float  m_fSpecialMoveElapsed = 0.f;
	_float  m_fSpecialMoveDuration = 0.f;
	_vector m_vSpecialMoveOffset = {};
	_vector m_fSpecialMoveStartPos = {};
	_bool	m_bSpecialMoving = {};

	// 회전용 멤버 변수들
	_bool m_bSpecialRotating = false;
	_float m_fSpecialRotateElapsed = 0.f;
	_float m_fSpecialRotateDuration = 0.f;
	_float m_fSpecialRotateAngle = 0.f;
	_vector m_vSpecialRotateAxis = {};
	_matrix m_matSpecialRotateStart = {};

	_vector m_vOriginalRight = { 1.f, 0.f, 0.f };
	_vector m_vOriginalUp = { 0.f, 1.f, 0.f };
	_vector m_vOriginalLook = { 0.f, 0.f, 1.f };

	//점프 전용
	_vector m_vSpecialMoveStartPos = {};
	_vector m_vSpecialMoveTargetPos = {};

	//스케일 전용
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