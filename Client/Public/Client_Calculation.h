#pragma once
#include "Client_Defines.h"
#include "Base.h"
#include "GameInstance.h"
#include <chrono>
#include <random>

/* [ 캣멀룸 보간 ] */
// 첫 번째와 마지막 점은 곡선의 시작과 끝을 정의하고, 두 번째와 세 번째 점은 곡선의 방향을 정의합니다.
// t 값은 0에서 1 사이의 값을 가지며, 곡선의 위치를 결정합니다.
static _vector CatmullRom(const _vector& p0, const _vector& p1, const _vector& p2, const _vector& p3, _float t)
{
    return
        0.5f * (
            (2 * p1) +
            (-p0 + p2) * t +
            (2 * p0 - 5 * p1 + 4 * p2 - p3) * t * t +
            (-p0 + 3 * p1 - 3 * p2 + p3) * t * t * t
            );
}
static _float3 CatmullRom(_float3 p0, _float3 p1, _float3 p2, _float3 p3, _float t)
{
    _float t2 = t * t;
    _float t3 = t2 * t;

    _float3 result;
    result.x = 0.5f * ((2 * p1.x) +
        (-p0.x + p2.x) * t +
        (2 * p0.x - 5 * p1.x + 4 * p2.x - p3.x) * t2 +
        (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t3);
    result.y = 0.5f * ((2 * p1.y) +
        (-p0.y + p2.y) * t +
        (2 * p0.y - 5 * p1.y + 4 * p2.y - p3.y) * t2 +
        (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t3);
    result.z = 0.5f * ((2 * p1.z) +
        (-p0.z + p2.z) * t +
        (2 * p0.z - 5 * p1.z + 4 * p2.z - p3.z) * t2 +
        (-p0.z + 3 * p1.z - 3 * p2.z + p3.z) * t3);
    return result;
}
static _float CatmullRom(_float p0, _float p1, _float p2, _float p3, _float t)
{
    return
        0.5f * (
            (2.f * p1) +
            (-p0 + p2) * t +
            (2.f * p0 - 5.f * p1 + 4.f * p2 - p3) * t * t +
            (-p0 + 3.f * p1 - 3.f * p2 + p3) * t * t * t
            );
}

/* [ 랜덤 난수 뽑기 ] */
static float GetRandomFloat(float min, float max)
{
    static std::mt19937 gen(std::random_device{}());
    if (min > max) std::swap(min, max);
    uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}
static int GetRandomInt(int min, int max)
{
    static std::mt19937 gen(std::random_device{}());
    if (min > max) std::swap(min, max);
    std::uniform_int_distribution<int> dist(min, max); // min 이상 max 이하 정수
    return dist(gen);
}

/* [ 감속 보간(빠른 시작) ] */
static _float EaseOutBack(_float t)
{
    _float c1 = 1.70158f;
    _float c3 = c1 + 1.f;

    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}
static _float EaseOutCubic(_float t)
{
    return 1 - powf(1 - t, 3);
}

/* [ 가속 보간(느린 시작) ] */
static _float EaseInQuad(_float t)
{
    return t * t;
}
static _float EaseOutQuad(_float t)
{
    return 1 - (1 - t) * (1 - t);
}

/* [ 삼각형 넓이 계산 ] */
static _float Compute_TriangleArea(const _float3& A, const _float3& B, const _float3& C)
{
    _vector vA = XMLoadFloat3(&A);
    _vector vB = XMLoadFloat3(&B);
    _vector vC = XMLoadFloat3(&C);

    _vector ab = XMVectorSubtract(vB, vA);
    _vector ac = XMVectorSubtract(vC, vA);
    _vector cross = XMVector3Cross(ab, ac);

    _vector len = XMVector3Length(cross);
    return 0.5f * XMVectorGetX(len);
}

/* [ 쿼터니언을 xyz로 ] */
static _float3 QuaternionToEuler(_fvector q)
{
    _matrix mat = XMMatrixRotationQuaternion(q);

    _float4x4 fmat;
    XMStoreFloat4x4(&fmat, mat);

    _float pitch = asinf(-fmat._32);
    _float yaw = atan2f(fmat._31, fmat._33);
    _float roll = atan2f(fmat._12, fmat._22);

    return _float3(pitch, yaw, roll); // radian 기준
}

static _matrix Compute_Billboard(_fmatrix SrcMatrix)
{
    _float4x4 m_Return = {};
    XMStoreFloat4x4(&m_Return, XMMatrixIdentity());

    _float3	vScaled = _float3(XMVectorGetX(XMVector3Length(SrcMatrix.r[0])),
        XMVectorGetX(XMVector3Length(SrcMatrix.r[1])),
        XMVectorGetX(XMVector3Length(SrcMatrix.r[2])));

    _vector	vPosition = SrcMatrix.r[3];

    _matrix matCamWorld = CGameInstance::Get_Instance()->Get_Transform_Matrix_Inv(D3DTS::VIEW);

    _vector vRight = matCamWorld.r[0]; // X axis
    _vector vUp = matCamWorld.r[1]; // Y axis
    _vector vLook = matCamWorld.r[2]; // Z axis

    XMStoreFloat3((_float3*)&m_Return.m[0][0], XMVector3Normalize(vRight) * vScaled.x);
    XMStoreFloat3((_float3*)&m_Return.m[1][0], XMVector3Normalize(vUp) * vScaled.y);
    XMStoreFloat3((_float3*)&m_Return.m[2][0], XMVector3Normalize(vLook) * vScaled.z);
    XMStoreFloat3((_float3*)&m_Return.m[3][0], vPosition);
    m_Return._44 = 1.f;

    return XMLoadFloat4x4(&m_Return);
}

static _matrix Turn_Billboard(_fmatrix SrcMatrix, _float fRotationPerSec, _float fTimeDelta)
{
    _matrix m_Return = XMMatrixIdentity();

    _matrix	RotationMatrix = XMMatrixRotationAxis(SrcMatrix.r[2], fRotationPerSec * fTimeDelta);

    //Set_State(STATE::RIGHT, XMVector4Transform(SrcMatrix.r[0], RotationMatrix));
    //Set_State(STATE::UP, XMVector4Transform(SrcMatrix.r[1], RotationMatrix));
    //Set_State(STATE::LOOK, XMVector4Transform(SrcMatrix.r[2], RotationMatrix));

    m_Return.r[0] = XMVector4Transform(SrcMatrix.r[0], RotationMatrix);
    m_Return.r[1] = XMVector4Transform(SrcMatrix.r[1], RotationMatrix);
    m_Return.r[2] = XMVector4Transform(SrcMatrix.r[2], RotationMatrix);
    m_Return.r[3] = SrcMatrix.r[3];

    return m_Return;
}