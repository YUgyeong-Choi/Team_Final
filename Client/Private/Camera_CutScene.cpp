#include "Camera_CutScene.h"
#include "GameInstance.h"
#include "Client_Calculation.h"
#include "BossUnit.h"
#include "Camera_Manager.h"
#include "EffectContainer.h"
#include "Effect_Manager.h"
#include "Player.h"

#pragma region help
// ===== Speed-curve helpers =====
// preset: Linear/EaseIn/EaseOut/EaseInOut 의 "속도 s(t)" (F(t)의 도함수)
inline float SpeedLinear(float t) { return 1.0f; }                // F=t
inline float SpeedEaseIn(float t) { return 2.0f * t; }            // F=t^2
inline float SpeedEaseOut(float t) { return 2.0f * (1.0f - t); }   // F=1-(1-t)^2
inline float SpeedEaseInOut(float t) { return (t < 0.5f) ? 12.0f * t * t : 12.0f * (1.0f - t) * (1.0f - t); } // F=4t^3 / 1-4(1-t)^3

// 균등 노드 Hermite (네가 쓰던 것 그대로)
inline float Hermite01(float y0, float y1, float y2, float y3, float u) {
	const float m1 = 0.5f * (y2 - y0), m2 = 0.5f * (y3 - y1);
	const float u2 = u * u, u3 = u2 * u;
	return (2 * u3 - 3 * u2 + 1) * y1 + (u3 - 2 * u2 + u) * m1 + (-2 * u3 + 3 * u2) * y2 + (u3 - u2) * m2;
}

// Custom5를 "속도 s(t)"로 샘플링 (음수 방지)
inline float SampleCustom5Speed(const float y[5], float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	constexpr float xs[5] = { 0.f, 0.25f, 0.5f, 0.75f, 1.f };
	int i = 0; while (i < 4 && t > xs[i + 1]) ++i;
	const float x0 = xs[i], x1 = xs[i + 1];
	const float u = (x1 > x0) ? (t - x0) / (x1 - x0) : 0.0f;

	const int i0 = max(0, i - 1), i1 = i, i2 = i + 1, i3 = std::min(4, i + 2);
	float s = Hermite01(y[i0], y[i1], y[i2], y[i3], u);
	return (s < 0.0f) ? 0.0f : s; // 속도는 음수 금지
}

// 0..t까지 속도 적분 / 0..1까지 속도 적분  → t'  (Trapezoidal integration)
inline float RemapBySpeed(int curveType, const float curveY[5], float t) {
	t = std::clamp(t, 0.0f, 1.0f);

	auto speed = [&](float u)->float {
		switch (curveType) {
		case 0:  return SpeedLinear(u);
		case 1:  return SpeedEaseIn(u);
		case 2:  return SpeedEaseOut(u);
		case 3:  return SpeedEaseInOut(u);
		case 4:  return SampleCustom5Speed(curveY, u);
		default: return 1.0f;
		}
		};

	auto integrate = [&](float a, float b)->float {
		if (b <= a) return 0.0f;
		const int N = 64; // 분할 수(필요시 올려도 됨)
		float sum = 0.0f;
		float prevX = a, prevY = speed(a);
		for (int k = 1; k <= N; ++k) {
			float x = a + (b - a) * (k / float(N));
			float y = speed(x);
			sum += (prevY + y) * 0.5f * (x - prevX);
			prevX = x; prevY = y;
		}
		return sum;
		};

	const float total = integrate(0.0f, 1.0f);
	if (total <= 1e-6f) return t;              // 안전 폴백
	const float part = integrate(0.0f, t);
	return std::clamp(part / total, 0.0f, 1.0f);
}

static inline bool IsNearlyZero3(FXMVECTOR v, float eps = 1e-8f) {
	return XMVectorGetX(XMVector3LengthSq(v)) < eps;
}
#pragma endregion


CCamera_CutScene::CCamera_CutScene(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CCamera{ pDevice, pContext }
{
}

CCamera_CutScene::CCamera_CutScene(const CCamera_CutScene& Prototype)
	: CCamera(Prototype)
{
}

HRESULT CCamera_CutScene::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCamera_CutScene::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	CAMERA_CutScene_DESC* pDesc = static_cast<CAMERA_CutScene_DESC*>(pArg);
	m_fMouseSensor = pDesc->fMouseSensor;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(InitDatas()))
		return E_FAIL;

	m_pTransformCom->Set_WorldMatrix(m_CutSceneDatas[CUTSCENE_TYPE::WAKEUP].vecWorldMatrixData.front().WorldMatrix);
	m_pGameInstance->Set_Transform(D3DTS::VIEW, m_pTransformCom->Get_WorldMatrix_Inverse());
	m_pGameInstance->Set_Transform(D3DTS::PROJ, XMMatrixPerspectiveFovLH(m_fFov, m_fAspect, m_fNear, m_fFar));

	return S_OK;
}

void CCamera_CutScene::Priority_Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	if (KEY_DOWN(DIK_C))
	{
		m_bStopCamera = !m_bStopCamera;
	}
	
	if (m_bStopCamera)
		fTimeDelta = 0.f;
	
	if (m_bActive)
	{
		if (!m_bOrbitalToSetOrbital)
		{
			_bool bFinish = ReadyToOrbitalWorldMatrix(fTimeDelta);
			m_bOrbitalToSetOrbital = bFinish;
		}

		if (m_bOrbitalToSetOrbital && !m_bReadyCutScene)
		{
			_bool bCheck = false;
			if (m_CameraDatas.vecTargetData.size() == 0)
			{
				// 목표 행렬
				_matrix targetMat = m_CameraDatas.vecWorldMatrixData.front().WorldMatrix;
				// 현재 행렬
				_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

				_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);
				m_bReadyCutScene = bFinish;
				bCheck = true;
			}

			if (m_CameraDatas.vecWorldMatrixData.size() == 0)
			{
				m_bReadyCutScene = true;
				bCheck = true;
			}

			if (!bCheck && (m_CameraDatas.vecWorldMatrixData.front().iKeyFrame > m_CameraDatas.vecTargetData.front().iKeyFrame))
			{
				m_bReadyCutScene = true;
			}

			if (!bCheck && (m_CameraDatas.vecWorldMatrixData.front().iKeyFrame < m_CameraDatas.vecTargetData.front().iKeyFrame))
			{
				// 목표 행렬
				_matrix targetMat = m_CameraDatas.vecWorldMatrixData.front().WorldMatrix;
				// 현재 행렬
				_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

				_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);
				m_bReadyCutScene = bFinish;
			}
		}

		if (m_bReadyCutScene && !m_bReadyCutSceneOrbital)
		{
			m_fElapsedTime += fTimeDelta;

			// 시간 누적 → 프레임 단위 변환
			_int iNewFrame = static_cast<_int>(m_fElapsedTime * m_fFrameSpeed);

			if (iNewFrame != m_iCurrentFrame)
			{ 
				m_iCurrentFrame = iNewFrame;

				//printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzPlszzzzzzzzzzzzzzzzzzzzzzzzz\n");
				//printf("zzzzzzzzzzzzzzzzzzzzzzzzzzzPlszzzzzzzzzzzzzzzzzzzzzzzzz\n");
				//printf("%d CurrentFrame\n", m_iCurrentFrame);

				Interp_WorldMatrixOnly(m_iCurrentFrame);
				Interp_Fov(m_iCurrentFrame);
				Interp_OffsetRot(m_iCurrentFrame);
				Interp_OffsetPos(m_iCurrentFrame);
				Interp_Target(m_iCurrentFrame);

				Event();
				
				// 종료 조건
				if (m_iCurrentFrame > m_CameraDatas.iEndFrame)
				{
					m_bReadyCutSceneOrbital = true;
				}
			}
		}

		if (m_bReadyCutSceneOrbital)
		{
			m_initOrbitalMatrix = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_OrbitalWorldMatrix(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
			// 목표 행렬
			_matrix targetMat = m_initOrbitalMatrix;
			// 현재 행렬
			_matrix currentMat = m_pTransformCom->Get_WorldMatrix();

			_bool bFinish = Camera_Blending(fTimeDelta, targetMat, currentMat);

			if (bFinish)
			{
				m_bActive = false;
				m_bOrbitalToSetOrbital = false;
				m_bReadyCutScene = false;
				m_bReadyCutSceneOrbital = false;
				m_fElapsedTime = 0.f;
				m_iCurrentFrame = -1;
				m_initOrbitalMatrix = {};
				CCamera_Manager::Get_Instance()->GetOrbitalCam()->Set_InitCam(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
				CCamera_Manager::Get_Instance()->SetOrbitalCam();
			}
		}
	}

	// Tool 용
	if (m_bShowSpecial)
	{
		m_fElapsedTime += fTimeDelta;

		const bool useRange = (m_iStaratFrame >= 0 && m_iEndFrame >= 0 && m_iStaratFrame <= m_iEndFrame);
		const _int baseFrame = useRange ? m_iStaratFrame : 0;
		const _int endBound = useRange ? m_iEndFrame : m_CameraDatas.iEndFrame;

		// 경과시간 -> "베이스 기준" 프레임
		const _int relFrame = static_cast<_int>(m_fElapsedTime * m_fFrameSpeed); // 0,1,2...
		const _int iNewFrame = baseFrame + relFrame;

		if (iNewFrame != m_iCurrentFrame)
		{
			m_iCurrentFrame = iNewFrame;

			// 클램프 & 종료 처리 (큰 dt로 end를 건너뛰는 경우 대비)
			if (m_iCurrentFrame > endBound)
			{
				m_iCurrentFrame = endBound;
				// 재생 종료
				m_bShowSpecial = false;           // 더 이상 업데이트 안 하도록
				m_iCurrentFrame = -1;
				m_fElapsedTime = 0;
				CCamera_Manager::Get_Instance()->SetOrbitalCam();
				return;
			}

			// 보간 적용
			Interp_WorldMatrixOnly(m_iCurrentFrame);
			Interp_Fov(m_iCurrentFrame);
			Interp_OffsetRot(m_iCurrentFrame);
			Interp_OffsetPos(m_iCurrentFrame);
			Interp_Target(m_iCurrentFrame);

			Event();
		}
	}
}

void CCamera_CutScene::Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;

	if (KEY_DOWN(DIK_CAPSLOCK))
		PrintMatrix("CutScene CameraWold", XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr()));

	if (m_bStopCamera)
		fTimeDelta = 0.f;

	__super::Update(fTimeDelta);
}

void CCamera_CutScene::Late_Update(_float fTimeDelta)
{
	if (CCamera_Manager::Get_Instance()->GetCurCam() != this)
		return;
}

HRESULT CCamera_CutScene::Render()
{
	return S_OK;
}

void CCamera_CutScene::Set_CameraFrame(CUTSCENE_TYPE cutSceneType, const CAMERA_FRAMEDATA CameraFrameData, _int start, _int end)
{
	m_CameraDatas = CameraFrameData;

	m_eCurrentCutScene = cutSceneType;
	m_iStaratFrame = start;
	m_iEndFrame = end;
	m_bShowSpecial = true;
}

void CCamera_CutScene::Set_CutSceneData(CUTSCENE_TYPE cutSceneType)
{
	m_CameraDatas = m_CutSceneDatas[cutSceneType];
	m_bOrbitalToSetOrbital = m_CameraDatas.bOrbitalToSetOrbital;
	if (m_bOrbitalToSetOrbital)
		m_pTransformCom->Set_WorldMatrix(m_CameraDatas.vecWorldMatrixData.front().WorldMatrix);
	m_bReadyCutSceneOrbital = m_CameraDatas.bReadyCutSceneOrbital;
	m_bReadyCutScene = false;

	m_eCurrentCutScene = cutSceneType;

	m_initOrbitalMatrix = CCamera_Manager::Get_Instance()->GetOrbitalCam()->Get_OrbitalWorldMatrix(m_CameraDatas.fPitch, m_CameraDatas.fYaw);
	m_iCurrentFrame = -1;
	m_fElapsedTime = 0.f;
}

void CCamera_CutScene::Interp_WorldMatrixOnly(_int curFrame)
{
	auto& vec = m_CameraDatas.vecWorldMatrixData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const int span = max(1, b.iKeyFrame - a.iKeyFrame);
			float t = float(curFrame - a.iKeyFrame) / float(span); // 0..1

			float tRemap = RemapBySpeed(a.curveType, a.curveY, t);

			const INTERPOLATION_CAMERA interp = a.interpMatrixPos;

			// Decompose A
			XMVECTOR sA, rA, tA;
			XMMatrixDecompose(&sA, &rA, &tA, a.WorldMatrix);

			// Decompose B
			XMVECTOR sB, rB, tB;
			XMMatrixDecompose(&sB, &rB, &tB, b.WorldMatrix);

			if (IsNearlyZero3(tA))
			{
				a.WorldMatrix = m_pTransformCom->Get_WorldMatrix();
				XMMatrixDecompose(&sA, &rA, &tA, a.WorldMatrix);
			}

			XMVECTOR finalT = tA;
			XMVECTOR finalR = rA;
			XMVECTOR finalS = sA;

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				finalT = tA;
				finalR = rA;
				finalS = sA;
				break;

			case INTERPOLATION_CAMERA::LERP:
				finalT = XMVectorLerp(tA, tB, tRemap);
				finalR = XMQuaternionSlerp(rA, rB, tRemap);
				finalS = XMVectorLerp(sA, sB, tRemap);
				break;

			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				XMVECTOR t0 = (i == 0) ? tA : XMMatrixDecompose_T(vec[i - 1].WorldMatrix);
				XMVECTOR t1 = tA;
				XMVECTOR t2 = tB;
				XMVECTOR t3 = (i + 2 < vec.size()) ? XMMatrixDecompose_T(vec[i + 2].WorldMatrix) : t2;

				finalT = XMVectorCatmullRom(t0, t1, t2, t3, tRemap);
				finalR = XMQuaternionSlerp(rA, rB, tRemap); // 회전은 안정성을 위해 그대로 Slerp
				finalS = XMVectorLerp(sA, sB, tRemap);      // 스케일은 단순 LERP
				break;
			}
			}

			// 다시 합성
			_matrix result = XMMatrixAffineTransformation(finalS, XMVectorZero(), finalR, finalT);
			m_pTransformCom->Set_WorldMatrix(result);
			return;
		}
	}

	// 범위 밖이면 고정
	//const _matrix& m = (curFrame <= vec.front().iKeyFrame) ? vec.front().WorldMatrix : vec.back().WorldMatrix;
	//m_pTransformCom->Set_WorldMatrix(m);
}

void CCamera_CutScene::Interp_Fov(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecFovData;
	if (vec.empty())
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpFov;
			const int span = max(1, b.iKeyFrame - a.iKeyFrame);
			float t = float(curFrame - a.iKeyFrame) / float(span); // 0..1

			float tRemap = RemapBySpeed(a.curveType, a.curveY, t);

			_float result = a.fFov;

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				result = a.fFov;
				break;

			case INTERPOLATION_CAMERA::LERP:
			default:
				result = a.fFov + (b.fFov - a.fFov) * tRemap;
				break;

			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				_float p0 = (i == 0) ? a.fFov : vec[i - 1].fFov;
				_float p1 = a.fFov;
				_float p2 = b.fFov;
				_float p3 = (i + 2 < vec.size()) ? vec[i + 2].fFov : p2;

				result = CatmullRom(p0, p1, p2, p3, tRemap);
				break;
			}
			}

			Set_FOV(XMConvertToRadians(result)); // ← 라디안 변환 후 적용
			return;
		}
	}

	// 경계 외에서는 처음 또는 마지막 값 고정
	if (curFrame <= vec.front().iKeyFrame)
		Set_FOV(XMConvertToRadians(vec.front().fFov));
	else if (curFrame >= vec.back().iKeyFrame)
		Set_FOV(XMConvertToRadians(vec.back().fFov));
}

void CCamera_CutScene::Interp_OffsetRot(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecOffSetRotData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpOffSetRot;
			const int span = max(1, b.iKeyFrame - a.iKeyFrame);
			float t = float(curFrame - a.iKeyFrame) / float(span); // 0..1
			float tRemap = RemapBySpeed(a.curveType, a.curveY, t);

			XMVECTOR rotA = XMLoadFloat3(&a.offSetRot);
			XMVECTOR rotB = XMLoadFloat3(&b.offSetRot);
			XMVECTOR result = {};

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
				break;
			case INTERPOLATION_CAMERA::LERP:
			default:
				result = XMVectorLerp(rotA, rotB, tRemap);
				break;
			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				XMVECTOR p1 = rotA;
				XMVECTOR p2 = rotB;
				XMVECTOR p0 = (i == 0) ? p1 : XMLoadFloat3(&vec[i - 1].offSetRot);
				XMVECTOR p3 = (i + 2 < vec.size()) ? XMLoadFloat3(&vec[i + 2].offSetRot) : p2;
				result = XMVectorCatmullRom(p0, p1, p2, p3, tRemap);
				break;
			}
			}

			// 오프셋 회전 저장 (최종 회전은 update에서 적용)
			m_vCurrentShakeRot = result;
			return;
		}
	}

	// 범위 바깥이면 시작/끝값
	if (curFrame <= vec.front().iKeyFrame)
		m_vCurrentShakeRot = XMLoadFloat3(&vec.front().offSetRot);
	else if (curFrame >= vec.back().iKeyFrame)
		if (vec.back().interpOffSetRot == INTERPOLATION_CAMERA::NONE)
			m_vCurrentShakeRot = { 0.f, 0.f, 0.f, 0.f };
		else
			m_vCurrentShakeRot = XMLoadFloat3(&vec.back().offSetRot);
}

void CCamera_CutScene::Interp_OffsetPos(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecOffSetPosData;
	if (vec.size() < 1)
		return;

	for (size_t i = 0; i < vec.size() - 1; ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];

		if (curFrame >= a.iKeyFrame && curFrame <= b.iKeyFrame)
		{
			const INTERPOLATION_CAMERA interp = a.interpOffSetPos;
			const int span = max(1, b.iKeyFrame - a.iKeyFrame);
			float t = float(curFrame - a.iKeyFrame) / float(span); // 0..1
			float tRemap = RemapBySpeed(a.curveType, a.curveY, t);

			XMVECTOR rotA = XMLoadFloat3(&a.offSetPos);
			XMVECTOR rotB = XMLoadFloat3(&b.offSetPos);
			XMVECTOR result = {};

			switch (interp)
			{
			case INTERPOLATION_CAMERA::NONE:
				m_vCurrentShakePos = { 0.f, 0.f, 0.f, 0.f };
				break;
			case INTERPOLATION_CAMERA::LERP:
			default:
				result = XMVectorLerp(rotA, rotB, tRemap);
				break;
			case INTERPOLATION_CAMERA::CATMULLROM:
			{
				XMVECTOR p1 = rotA;
				XMVECTOR p2 = rotB;
				XMVECTOR p0 = (i == 0) ? p1 : XMLoadFloat3(&vec[i - 1].offSetPos);
				XMVECTOR p3 = (i + 2 < vec.size()) ? XMLoadFloat3(&vec[i + 2].offSetPos) : p2;
				result = XMVectorCatmullRom(p0, p1, p2, p3, tRemap);
				break;
			}
			}

			// 오프셋 위치 저장 (최종 위치는 update에서 적용)
			m_vCurrentShakePos = result;
			return;
		}
	}

	// 범위 바깥이면 시작/끝값
	if (curFrame <= vec.front().iKeyFrame)
		m_vCurrentShakePos = XMLoadFloat3(&vec.front().offSetPos);
	else if (curFrame >= vec.back().iKeyFrame)
		if (vec.back().interpOffSetPos == INTERPOLATION_CAMERA::NONE)
			m_vCurrentShakePos = { 0.f, 0.f, 0.f, 0.f };
		else
			m_vCurrentShakePos = XMLoadFloat3(&vec.back().offSetPos);
}

// 각도 유틸(라디안)
inline _float WrapPi(_float a) {
	a = fmodf(a + XM_PI, XM_2PI);
	if (a <= 0) a += XM_2PI;
	return a - XM_PI;
}
inline _float ShortestDelta(_float from, _float to) {
	return WrapPi(to - from);
}
inline _float LerpAngle(_float from, _float to, _float t) {
	return from + ShortestDelta(from, to) * t;
}

void CCamera_CutScene::Interp_Target(_int curFrame)
{
	const auto& vec = m_CameraDatas.vecTargetData;
	if (vec.size() < 2) return;

	for (size_t i = 0; i + 1 < vec.size(); ++i)
	{
		const auto& a = vec[i];
		const auto& b = vec[i + 1];
		if (curFrame < a.iKeyFrame || curFrame > b.iKeyFrame)
			continue;

		if (a.eTarget == TARGET_CAMERA::NONE)
			return; // 필요시 continue

		CGameObject* pTargetObj = nullptr;
		switch (a.eTarget)
		{
		case TARGET_CAMERA::PLAYER:
			pTargetObj = m_pGameInstance->Get_LastObject(
				m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_Player"));
			break;
		default: break;
		}
		if (!pTargetObj) return;

		// 진행률 → 속도 커브 기반 t'
		const int   span = max(1, b.iKeyFrame - a.iKeyFrame);
		const float t = float(curFrame - a.iKeyFrame) / float(span);   // 0..1
		const float tp = RemapBySpeed(a.curveType, a.curveY, t);       

		// 스칼라 보간 (Yaw는 최단 경로)
		const float fPitch = LerpFloat(a.fPitch, b.fPitch, tp);
		const float fYaw = LerpAngle(a.fYaw, b.fYaw, tp);
		const float fDistance = max(0.f, LerpFloat(a.fDistance, b.fDistance, tp));

		// 타깃 피벗(머리 위 + 살짝 뒤)
		_vector pivot = pTargetObj->Get_TransfomCom()->Get_State(STATE::POSITION);
		pivot += XMVectorSet(0.f, 1.7f, 0.f, 0.f);
		pivot += XMVector3Normalize(pTargetObj->Get_TransfomCom()->Get_State(STATE::LOOK)) * -0.15f;

		// 구면좌표 → 카메라 오프셋
		const float x = fDistance * cosf(fPitch) * sinf(fYaw);
		const float y = fDistance * sinf(fPitch);
		const float z = fDistance * cosf(fPitch) * cosf(fYaw);
		const _vector offset = XMVectorSet(x, y, z, 0.f);

		const _vector desiredPos = pivot + offset;

		m_pTransformCom->Set_State(STATE::POSITION, desiredPos);
		m_pTransformCom->LookAt(pivot);
		return;
	}
}

XMVECTOR CCamera_CutScene::XMMatrixDecompose_T(const _matrix& m)
{
	XMVECTOR scale, rot, trans;
	XMMatrixDecompose(&scale, &rot, &trans, m);
	return trans;
}

HRESULT CCamera_CutScene::InitDatas()
{
	m_CutSceneDatas.clear();

	ifstream inFile("../Bin/Save/CutScene/WakeUp.json");
	if (inFile.is_open())
	{
		json j;
		inFile >> j;
		inFile.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::WAKEUP, LoadCameraFrameData(j)));
	}

	ifstream inFile2("../Bin/Save/CutScene/TutorialDoor.json");
	if (inFile2.is_open())
	{
		json j;
		inFile2 >> j;
		inFile2.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::TUTORIALDOOR, LoadCameraFrameData(j)));
	}

	ifstream inFile3("../Bin/Save/CutScene/OutDoor.json");
	if (inFile3.is_open())
	{
		json j;
		inFile3 >> j;
		inFile3.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::OUTDOOOR, LoadCameraFrameData(j)));
	}

	ifstream inFile4("../Bin/Save/CutScene/FuocoDoor.json");
	if (inFile4.is_open())
	{
		json j;
		inFile4 >> j;
		inFile4.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::FUOCO, LoadCameraFrameData(j)));
	}

	ifstream inFile5("../Bin/Save/CutScene/FestivalDoor.json");
	if (inFile5.is_open())
	{
		json j;
		inFile5 >> j;
		inFile5.close();

		m_CutSceneDatas.emplace(make_pair(CUTSCENE_TYPE::FESTIVAL, LoadCameraFrameData(j)));
	}

	return S_OK;
}

CAMERA_FRAMEDATA CCamera_CutScene::LoadCameraFrameData(const json& j)
{
	CAMERA_FRAMEDATA data;

	data.iEndFrame = j.value("iEndFrame", 0);
	data.bOrbitalToSetOrbital = j.value("bStartBlend", false);
	data.bReadyCutSceneOrbital = j.value("bEndBlend", false);
	data.fPitch = j["Pitch"].get<float>();
	data.fYaw = j["Yaw"].get<float>();

	// 1. vecMatrixPosData
	if (j.contains("vecMatrixPosData"))
	{
		for (const auto& worldJson : j["vecMatrixPosData"])
		{
			CAMERA_WORLDFRAME worldFrame;
			worldFrame.iKeyFrame = worldJson["keyFrame"];
			worldFrame.interpMatrixPos = worldJson["interpMatrixPosition"];

			const std::vector<float>& matValues = worldJson["worldMatrix"];
			XMFLOAT4X4 mat;
			memcpy(&mat, matValues.data(), sizeof(float) * 16);
			worldFrame.WorldMatrix = XMLoadFloat4x4(&mat);

			worldFrame.curveType = worldJson.value("curveType", 0); // 0=Linear

			if (worldJson.contains("curveY") && worldJson["curveY"].is_array())
			{
				auto arr = worldJson["curveY"];
				const int n = std::min<int>(5, (int)arr.size());
				for (int k = 0; k < n; ++k)
					worldFrame.curveY[k] = arr[k].get<float>();
			}

			data.vecWorldMatrixData.push_back(worldFrame);
		}
	}

	// 2. vecPosData
	if (j.contains("vecPosData"))
	{
		for (const auto& posJson : j["vecPosData"])
		{
			CAMERA_POSFRAME posFrame;
			posFrame.iKeyFrame = posJson["keyFrame"];
			posFrame.offSetPos = XMFLOAT3(
				posJson["position"][0],
				posJson["position"][1],
				posJson["position"][2]
			);
			posFrame.interpOffSetPos = posJson["interpPosition"];


			posFrame.curveType = posJson.value("curveType", 0); // 0=Linear

			if (posJson.contains("curveY") && posJson["curveY"].is_array())
			{
				auto arr = posJson["curveY"];
				const int n = std::min<int>(5, (int)arr.size());
				for (int k = 0; k < n; ++k)
					posFrame.curveY[k] = arr[k].get<float>();
			}

			data.vecOffSetPosData.push_back(posFrame);
		}
	}

	// 3. vecRotData
	if (j.contains("vecRotData"))
	{
		for (const auto& rotJson : j["vecRotData"])
		{
			CAMERA_ROTFRAME rotFrame;
			rotFrame.iKeyFrame = rotJson["keyFrame"];
			rotFrame.offSetRot = XMFLOAT3(
				rotJson["rotation"][0],
				rotJson["rotation"][1],
				rotJson["rotation"][2]
			);
			rotFrame.interpOffSetRot = rotJson["interpRotation"];

			rotFrame.curveType = rotJson.value("curveType", 0); // 0=Linear

			if (rotJson.contains("curveY") && rotJson["curveY"].is_array())
			{
				auto arr = rotJson["curveY"];
				const int n = std::min<int>(5, (int)arr.size());
				for (int k = 0; k < n; ++k)
					rotFrame.curveY[k] = arr[k].get<float>();
			}

			data.vecOffSetRotData.push_back(rotFrame);
		}
	}

	// 4. vecFovData
	if (j.contains("vecFovData"))
	{
		for (const auto& fovJson : j["vecFovData"])
		{
			CAMERA_FOVFRAME fovFrame;
			fovFrame.iKeyFrame = fovJson["keyFrame"];
			fovFrame.fFov = fovJson["fFov"];
			fovFrame.interpFov = fovJson["interpFov"];

			fovFrame.curveType = fovJson.value("curveType", 0); // 0=Linear

			if (fovJson.contains("curveY") && fovJson["curveY"].is_array())
			{
				auto arr = fovJson["curveY"];
				const int n = std::min<int>(5, (int)arr.size());
				for (int k = 0; k < n; ++k)
					fovFrame.curveY[k] = arr[k].get<float>();
			}

			data.vecFovData.push_back(fovFrame);
		}
	}

	// 5. vecTargetData
	if (j.contains("vecTargetData"))
	{
		for (const auto& targetJson : j["vecTargetData"])
		{
			CAMERA_TARGETFRAME targetFrame;
			targetFrame.iKeyFrame = targetJson["keyFrame"];
			targetFrame.eTarget = static_cast<TARGET_CAMERA>(targetJson["targetType"]);
			targetFrame.fPitch = targetJson["pitch"];
			targetFrame.fYaw = targetJson["yaw"];
			targetFrame.fDistance = targetJson["distance"];

			targetFrame.curveType = targetJson.value("curveType", 0); // 0=Linear

			if (targetJson.contains("curveY") && targetJson["curveY"].is_array())
			{
				auto arr = targetJson["curveY"];
				const int n = std::min<int>(5, (int)arr.size());
				for (int k = 0; k < n; ++k)
					targetFrame.curveY[k] = arr[k].get<float>();
			}

			data.vecTargetData.push_back(targetFrame);
		}
	}
	return data;
}

void CCamera_CutScene::Event()
{
	switch (m_eCurrentCutScene)
	{
	case Client::CUTSCENE_TYPE::WAKEUP:
		break;
	case Client::CUTSCENE_TYPE::TUTORIALDOOR:
	{
		if (m_iCurrentFrame == 20)
		{
			GET_PLAYER(m_pGameInstance->GetCurrentLevelIndex())->Create_LeftArm_Lightning();
		}
	}
		break;
	case Client::CUTSCENE_TYPE::OUTDOOOR:
		break;
	case Client::CUTSCENE_TYPE::FUOCO:
	{
		if (m_iCurrentFrame == 860)
		{
			CBossUnit* unit = static_cast<CBossUnit*>(m_pGameInstance->Get_LastObject(m_pGameInstance->GetCurrentLevelIndex(), TEXT("Layer_FireEater")));
			unit->EnterCutScene();
		}
		break;
	}
	case Client::CUTSCENE_TYPE::FESTIVAL:
		break;
	default:
		break;
	}
}

_bool CCamera_CutScene::ReadyToOrbitalWorldMatrix(_float fTimeDelta)
{
	_matrix currentMat = m_pTransformCom->Get_WorldMatrix();
	_matrix targetMat = m_initOrbitalMatrix;

	XMVECTOR curScale, curRot, curTrans;
	XMVECTOR tgtScale, tgtRot, tgtTrans;

	XMMatrixDecompose(&curScale, &curRot, &curTrans, currentMat);
	XMMatrixDecompose(&tgtScale, &tgtRot, &tgtTrans, targetMat);

	_float blendSpeed = 5.f;
	_float t = fTimeDelta * blendSpeed;

	// 보간
	XMVECTOR lerpPos = XMVectorLerp(curTrans, tgtTrans, t);
	XMVECTOR slerpRot = XMQuaternionSlerp(curRot, tgtRot, t);
	XMVECTOR lerpScale = XMVectorLerp(curScale, tgtScale, t);

	_matrix blendedMat = XMMatrixScalingFromVector(lerpScale)
		* XMMatrixRotationQuaternion(slerpRot)
		* XMMatrixTranslationFromVector(lerpPos);

	m_pTransformCom->Set_WorldMatrix(blendedMat);

	XMVECTOR diff = XMVector3LengthSq(tgtTrans - lerpPos);
	if (XMVectorGetX(diff) < 0.0001f)
	{
		return true;
	}

	return false;
}

_bool CCamera_CutScene::Camera_Blending(_float fTimeDelta, _matrix targetMat, _matrix currentMat)
{
	XMVECTOR curScale, curRotQuat, curTrans;
	XMVECTOR tgtScale, tgtRotQuat, tgtTrans;
	XMMatrixDecompose(&curScale, &curRotQuat, &curTrans, currentMat);
	XMMatrixDecompose(&tgtScale, &tgtRotQuat, &tgtTrans, targetMat);

	_float blendSpeed = 5.f;
	_float t = fTimeDelta * blendSpeed;
	t = min(t, 1.0f); // overshoot 방지

	XMVECTOR lerpPos = XMVectorLerp(curTrans, tgtTrans, t);
	XMVECTOR slerpRot = XMQuaternionSlerp(curRotQuat, tgtRotQuat, t);
	XMVECTOR lerpScale = XMVectorLerp(curScale, tgtScale, t);

	// 재합성
	_matrix blendedMat = XMMatrixScalingFromVector(lerpScale)
		* XMMatrixRotationQuaternion(slerpRot)
		* XMMatrixTranslationFromVector(lerpPos);

	m_pTransformCom->Set_WorldMatrix(blendedMat);

	// 차이 계산
	XMVECTOR diff = XMVector3LengthSq(tgtTrans - lerpPos);
	if (XMVectorGetX(diff) < 0.00001f)
	{
		return true; 
	}
	return false;
}

CCamera_CutScene* CCamera_CutScene::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CCamera_CutScene* pGameInstance = new CCamera_CutScene(pDevice, pContext);

	if (FAILED(pGameInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CCamera_CutScene");
		Safe_Release(pGameInstance);
	}
	return pGameInstance;
}

CGameObject* CCamera_CutScene::Clone(void* pArg)
{
	CCamera_CutScene* pGameInstance = new CCamera_CutScene(*this);

	if (FAILED(pGameInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CCamera_CutScene");
		Safe_Release(pGameInstance);
	}

	return pGameInstance;
}

void CCamera_CutScene::Free()
{
	__super::Free();

}
