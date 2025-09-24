// HLSL (Compute Shader)

float Random(uint iSeed, float fMin, float fMax);

/* [ 파티클 개별 속성 통합본 ] */
struct ParticleParam
{
    float4      Right;
    float4      Up;
    float4      Look;
    float4      Translation;
    
    float3      vInitOffset;
    uint        bFirstLoopDiscard;
    
    float4      Direction;  // normalized dir (w=unused)
    float4      VelocityDir; // 실제 이동한 방향 벡터, w = length
    
    float2      LifeTime;   // x=max, y=acc
    float       Speed;
    float       fTileIdx;

    float       OrbitSpeed; // degrees/sec
    float       fAccel;     // 가속도 (+면 가속, -면 감속)
    float       fMaxSpeed;  // 최대 속도 (옵션)
    float       fMinSpeed;  // 최소 속도 (옵션, 감속 시 멈춤 방지)
    
    float2      vTileOffset;
    float       RotationSpeed; // degrees/sec
    float       RotationAngle;
};

/* [ Constant Buffer ] */
cbuffer ParticleCB : register(b0)
{
    float DeltaTime; // dt (tool이면 무시)
    float TrackTime; // tool 절대시간(초) = curTrackPos/60.f
    uint ParticleType; // 0:SPREAD, 1:DIRECTIONAL
    uint NumInstances;

    uint isTool; // uint == bool
    uint isLoop;
    uint UseGravity;
    uint UseSpin;

    uint UseOrbit;
    uint isFirst;
    uint isTileLoop;
    float fGravity; // e.g. 9.8

    float2 vTileCnt;
    float fTileTickPerSec;
    float fAccTime;

    float3 Pivot; // vPivot
    uint EffectSeed;
    
    float3 Center; // vCenter
    float ShrinkThreshold;
    
    float3 OrbitAxis; // normalized
    uint LoopInSet;

    float3 Range; // vRange
    float LoopInSet_LoopDelay;
    
    float3 CircleNormal;
    uint isCircleRange;

    float4 vSocketRot;

    row_major float4x4 g_CombinedMatrix;
};

// t는 SRV, u는 UAV, b는 Constant Buffer
// 뒤의 숫자는 몇번째로 바인딩 했는지를 뜻함
// RWStructuredBuffer - Read / Write
// StructuredBuffer - Read only.
StructuredBuffer<ParticleParam> gInitInst : register(t0); 
RWStructuredBuffer<ParticleParam> gInst : register(u0);

struct AnimMeshVertex
{
    float3 vPosition : POSITION;
    float3 vNormal : NORMAL;
    float3 vTangent : TANGENT;
    float2 vTexcoord : TEXCOORD0;
    uint4 vBlendIndices : BLENDINDEX;
    float4 vBlendWeights : BLENDWEIGHT;
};

struct MeshInfo
{
    uint vertexOffset; // g_AllVertices에서의 시작 위치
    uint indexOffset; // g_AllIndices에서의 시작 위치
    uint cdfOffset; // g_AllCDFs에서의 시작 위치
    uint triangleCount;
};

cbuffer SkinningCB : register(b1)
{
    row_major matrix g_BoneMatrices[512];
    uint g_NumMeshes;
};

// 모델의 월드 행렬 (모델 자체의 변환)
cbuffer ModelTransforms : register(b2)
{
    matrix g_WorldMatrix;
};

StructuredBuffer<AnimMeshVertex> g_AllVertices : register(t1);
StructuredBuffer<uint> g_AllIndices : register(t2);
StructuredBuffer<float> g_AllCDFs : register(t3);
StructuredBuffer<float> g_MeshAreaCDF : register(t4);
StructuredBuffer<MeshInfo> g_MeshInfo : register(t5);

/************************************************************************************************/

// --- 2. CDF 탐색을 위한 이진 탐색 함수 ---

uint BinarySearch(StructuredBuffer<float> cdf, uint offset, uint count, float value)
{
    uint low = 0;
    uint high = count - 1;
    uint mid = 0;

    while (low <= high)
    {
        mid = low + (high - low) / 2;
        float cdfVal = cdf[offset + mid];
        float prevCdfVal = (mid == 0) ? 0.0f : cdf[offset + mid - 1];
        
        if (value > prevCdfVal && value <= cdfVal)
        {
            return mid;
        }
        else if (value > cdfVal)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return mid; // 보통은 루프 안에서 찾아야 함
}


// --- 3. 메쉬에서 위치를 샘플링하는 메인 함수 ---

float3 SpawnOnMesh(uint particleID)
{
    // --- 1단계: 메쉬 선택 ---
    float r_mesh = Random(particleID, 0.f, 1.f);
    uint selectedMeshIndex = BinarySearch(g_MeshAreaCDF, 0, g_NumMeshes, r_mesh);
    MeshInfo meshInfo = g_MeshInfo[selectedMeshIndex];

    // --- 2단계: 삼각형 선택 ---
    float r_tri = Random(particleID + 1, 0.f, 1.f);
    uint selectedTriIndex = BinarySearch(g_AllCDFs, meshInfo.cdfOffset, meshInfo.triangleCount, r_tri);

    // --- 3단계: 삼각형 내 위치 샘플링 (바리센트릭) ---
    uint i0 = g_AllIndices[meshInfo.indexOffset + selectedTriIndex * 3 + 0];
    uint i1 = g_AllIndices[meshInfo.indexOffset + selectedTriIndex * 3 + 1];
    uint i2 = g_AllIndices[meshInfo.indexOffset + selectedTriIndex * 3 + 2];

    AnimMeshVertex v0 = g_AllVertices[meshInfo.vertexOffset + i0];
    AnimMeshVertex v1 = g_AllVertices[meshInfo.vertexOffset + i1];
    AnimMeshVertex v2 = g_AllVertices[meshInfo.vertexOffset + i2];
    
    float r1 = Random(particleID + 2, 0.f, 1.f);
    float r2 = Random(particleID + 3, 0.f, 1.f);
    if (r1 + r2 > 1.0f)
    {
        r1 = 1.0f - r1;
        r2 = 1.0f - r2;
    }
    float r0 = 1.0f - r1 - r2;

    // 위치, 가중치 등 보간
    float3 localPos = v0.Pos * r0 + v1.Pos * r1 + v2.Pos * r2;
    float4 weights = v0.Weights * r0 + v1.Weights * r1 + v2.Weights * r2;
    // 뼈 ID는 가장 가중치가 높은 버텍스의 것을 따르는 것이 간단함
    uint4 boneIDs = v0.BoneIDs;

    // --- 4단계: 스키닝 ---
    float fWeightW = 1.f - (vertex.vBlendWeights.x + vertex.vBlendWeights.y + vertex.vBlendWeights.z);
    
    matrix BoneMatrix = g_BoneMatrices[vertex.vBlendIndices.x] * vertex.vBlendWeights.x +
        g_BoneMatrices[vertex.vBlendIndices.y] * vertex.vBlendWeights.y +
        g_BoneMatrices[vertex.vBlendIndices.z] * vertex.vBlendWeights.z +
        g_BoneMatrices[vertex.vBlendIndices.w] * fWeightW;

    vector vPosition = mul(vector(vertex.vPosition, 1.f), BoneMatrix);


    // --- 5단계: 월드 변환 ---
    return mul(vPosition, g_CombinedMatrix).xyz;
}



/************************************************************************************************/
// 공전
float3 rotateAroundAxis(float3 v, float3 axis, float angleRad)
{
    float s = sin(angleRad), c = cos(angleRad);
    return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1.0f - c);
}
// 벡터 * 쿼터니언
float3 RotateByQuat(float3 v, float4 q)
{
    // q * v * q^-1
    float3 t = 2.0f * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}
// 랜덤용 해시 추출 함수
float Hash(uint n)
{
    n = (n << 13u) ^ n;
    return ((n * (n * n * 15731u + 789221u) + 1376312589u) & 0x7fffffffu)
           / 2147483648.0; // 2^31
}
// iSeed + EffectSeed + 시간(fAccTime)을 섞은 랜덤
float Random(uint iSeed, float fMin, float fMax)
{
    // 시간은 float → uint 변환 과정에서 aliasing될 수 있으니 스케일링해서 정수화
    uint timeSeed = asuint(fAccTime * 1000.0f); // 밀리초 단위 정수화

    // 세 가지를 XOR/곱 조합으로 섞음
    uint mixedSeed = iSeed ^ (EffectSeed * 1664525u) ^ (timeSeed * 1013904223u);

    float rnd = Hash(mixedSeed); // [0,1)

    return lerp(fMin, fMax, rnd);
}
// 원하는 방향의 평면 위 원 둘레 랜덤 좌표                  (영웅)
// EllipseScale을 곱해서 타원처럼. range z에 넣어뒀으니까 기본값을 1로 주기...
float4 RandomOnCircle(uint iSeed, float3 Center, float Radius, float EllipseScale /*, float3 Normal*/)
{
    // --- 안전한 법선 ---
    float3 Normal = normalize(CircleNormal);

    // --- 법선과 직교하는 벡터 하나 얻기 ---
    float3 ref = (abs(Normal.y) < 0.999f) ? float3(0, 1, 0) : float3(1, 0, 0);
    float3 tangent = normalize(cross(Normal, ref));

    // --- 두 번째 직교 벡터 ---
    float3 bitangent = cross(Normal, tangent);

    // --- 랜덤 각도 ---
    float angle = Random(iSeed, 0.0f, 6.2831853f);
    float c = cos(angle) * Radius;
    float s = sin(angle) * Radius * EllipseScale;

    // --- 평면 위 점 ---
    float3 pos = Center + tangent * c + bitangent * s;

    return float4(pos, 1.f);
}
// 기존 Range 내 랜덤 좌표
float4 RandomInRange(uint iSeed, float3 Center, float3 Range)
{
    return float4(
        Random(iSeed, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
        Random(iSeed + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
        Random(iSeed + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f),
        1.f );
}

[numthreads(128, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint i = dispatchThreadId.x;
    if (i >= NumInstances)
        return;

    ParticleParam pp = gInst[i];
    float3 prevPos = pp.Translation.xyz;
    const bool TOOL = (isTool != 0);

    if (TOOL)
    {
        // --- 툴 모드: 절대시간으로 "초기값 기준" 재구성 ---
        pp.bFirstLoopDiscard = 0;
        float t = TrackTime / 60.f;
        if (isLoop != 0)
            t = fmod(t, pp.LifeTime.x);
        pp.fTileIdx = TrackTime / 60.f / fTileTickPerSec;
        if (isTileLoop != 0)
            pp.fTileIdx = fmod(pp.fTileIdx, vTileCnt.x * vTileCnt.y);
        uint iTileIdx = (uint) pp.fTileIdx;
        float fTileSizeX = 1.0f / float(vTileCnt.x);
        float fTileSizeY = 1.0f / float(vTileCnt.y);
        pp.vTileOffset = float2((iTileIdx % uint(vTileCnt.x)) * fTileSizeX, (iTileIdx / uint(vTileCnt.x)) * fTileSizeY);

        const ParticleParam init = gInitInst[i]; // 항상 초기값 기준
        float3 pos = init.Translation.xyz;
        
        float3 dir = normalize(pp.Direction.xyz);
        float v0 = init.Speed;
        float accel = pp.fAccel;
        float vMin = pp.fMinSpeed;
        float vMax = pp.fMaxSpeed;

        float travel = 0.0f;

        if (accel != 0.0f)
        {
            // vMin에 도달하는 시간
            float tStop = (vMin - v0) / accel;

            if ((accel < 0 && t < tStop) || (accel > 0 && v0 < vMin))
            {
            // 아직 vMin에 도달 안 함 → 가속/감속 공식
                travel = v0 * t + 0.5f * accel * t * t;
            }
            else
            {
            // vMin 이하로 내려갔거나, vMin 이상으로 올라간 뒤 → vMin 등속
                float stopDist = v0 * tStop + 0.5f * accel * tStop * tStop;
                float remain = max(t - tStop, 0.0f);
                travel = stopDist + vMin * remain;
            }
        }
        else
        {
            // 가속도 0이면 그냥 등속
            float curSpeed = clamp(v0, vMin, vMax);
            travel = curSpeed * t;
        }

        // 최종 위치
        pos += dir * travel;
        /*****/

        // 중력: 절대시간 적용 (s = 1/2 g t^2)
        if (UseGravity != 0)
            pos.y -= 0.5f * fGravity * t * t;

        // 자전: 초기 basis에 "절대 각도" 적용
        else
        {
            pp.Right = init.Right;
            pp.Up = init.Up;
            pp.Look = init.Look;
        }

        // 공전: (pos - Center)를 "절대 각도"만큼 회전
        if (UseOrbit != 0)  
        {
            float3 axis = normalize(OrbitAxis);
            float angle = radians(pp.OrbitSpeed) * t;
            float3 offset = pos - Center;
            offset = rotateAroundAxis(offset, axis, angle);
            pos = Center + offset;
        }

        pp.Translation = float4(pos, 1.0f);
        pp.LifeTime.y = t; // 툴은 절대시간을 그대로 보관
        float3 velocity = pos - init.Translation.xyz;
        pp.VelocityDir = float4(normalize(velocity), length(velocity));
        // 툴모드 이전프레임 계산 할 수는 있는데 일단 임시로 안함 초기위치랑만 비교
        gInst[i] = pp;
        return;
    }
    else
    {
        // --- 런타임 모드: 증분(Δt) 업데이트 ---
        pp.LifeTime.y += DeltaTime;
        
        if (isFirst == 1)
        {
            /* 이펙트 랜덤 생성 없을 시(bake 하고 고정 생성) */
            //pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);
            //
            //float3 localDir = gInitInst[i].Direction.xyz;
            //float3 worldDir = RotateByQuat(localDir, vSocketRot);
            //pp.Direction.xyz = normalize(worldDir);

            /******************************************************************************/

            /* 이펙트 랜덤 생성 시 (bake 안 하고) */

            float3 vCurPivot = Center + Pivot;
            
            if (isCircleRange == 0)
            {
                pp.Translation = RandomInRange(i * 3, Center.xyz, Range.xyz);
            }
            else
            {
                pp.Translation = RandomOnCircle(i * 3, Center.xyz, Range.x * 0.5f, Range.z);
            }
            
            if (ParticleType == 0) // spread
            {
                pp.Direction = float4(normalize(pp.Translation.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 1) // directional
            {
                pp.Direction = float4(normalize(Center.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 2) // random
            {
                pp.Direction = float4(
                    Random(i * 3    , -1.f, 1.f), 
                    Random(i * 3 + 1, -1.f, 1.f),
                    Random(i * 3 + 2, -1.f, 1.f),
                    0.f
                );
            }
            else if (ParticleType == 3) // to pivot
            {
                pp.Direction = float4(normalize(vCurPivot.xyz - pp.Translation.xyz), 0.f);
            }
            
            
            //pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);
            float3 worldDir = RotateByQuat(pp.Direction.xyz, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);
            
            pp.vInitOffset = float3(
			    pp.Translation.x,
			    pp.Translation.y,
			    pp.Translation.z
		    );
            
            
            if (pp.RotationSpeed < 0.0001f)
                pp.RotationAngle = 0.f;
            else
                pp.RotationAngle = Random(pp.RotationSpeed, 0.f, 3.14f);
            // MeshEmitter 사용 시 함수 호출 하라는데
            // Spawn();
        }
        
        float t = pp.LifeTime.y;
        float tPrev = max(t - DeltaTime, 0.0f);

        float3 pos = pp.Translation.xyz;
        float3 dir = normalize(pp.Direction.xyz);

        //// 이동: Δs = v * Δt
        //pos += dir * pp.Speed * DeltaTime;
        
        // 가속 추가 버전
        // 속도 갱신: v = v + a*Δt (최소/최대 속도 제한 포함)
        pp.Speed = clamp(pp.Speed + pp.fAccel * DeltaTime, pp.fMinSpeed, pp.fMaxSpeed);

        // 이동: Δs = v * Δt 
        pos += dir * pp.Speed * DeltaTime;

        // 중력: Δs = 1/2 g (t^2 - (t-Δt)^2)  = g*(t-Δt/2)*Δt
        if (UseGravity != 0)
        {
            float gravNow = 0.5f * fGravity * t * t;
            float gravPrev = 0.5f * fGravity * tPrev * tPrev;
            pos.y -= (gravNow - gravPrev);
        }

        // 자전: 직전 basis에 Δ각도만큼 회전
        if (UseSpin != 0)
        {
            float angle = radians(pp.RotationSpeed) * DeltaTime;
        }

        // 공전: Δ각도만큼 오프셋 회전
        if (UseOrbit != 0)
        {
            float3 axis = normalize(OrbitAxis);
            float angle = radians(pp.OrbitSpeed) * DeltaTime;
            float3 offset = pos - Center;
            offset = rotateAroundAxis(offset, axis, angle);
            pos = Center + offset;
        }
        
        pp.fTileIdx += DeltaTime * fTileTickPerSec;
        
        float3 velocity = pos - prevPos;
        if (isFirst)
            pp.VelocityDir = float4(0.f, 0.f, 0.f, 0.f);
        else
            pp.VelocityDir = float4(normalize(velocity), length(velocity));

        const bool firstLoop = (pp.bFirstLoopDiscard != 0);
        const bool lifeEnded = pp.LifeTime.y >= pp.LifeTime.x;
        const bool tileEnded = pp.fTileIdx >= vTileCnt.x * vTileCnt.y;
        const bool SetLifeEnded = pp.LifeTime.y >= LoopInSet_LoopDelay;

        //bool needReset = firstLoop ? lifeEnded 
        //                : ((isTileLoop != 0) ? tileEnded
        //                                        : ((isLoop != 0) && lifeEnded));
        //// 위의 삼항삼항연산자와 동일함
        //bool needReset = false;
        //if (firstLoop)
        //    needReset = lifeEnded;
        //else if (isTileLoop != 0)
        //    needReset = tileEnded; // TileLoop 우선
        //else if (isLoop != 0)
        //    needReset = lifeEnded;
        
        bool needReset = false;

        if (isTileLoop != 0) // true
        {
            if (firstLoop)
                needReset = lifeEnded; // 첫 루프는 lifeTime으로만 끊어줌
            else
                needReset = tileEnded; // 이후부터는 tileLoop만 체크
        }
        else
        {
            if (firstLoop)
                needReset = lifeEnded; // 원래 로직 그대로
            else if (isLoop != 0)
                needReset = lifeEnded;
        }
        if (LoopInSet != 0) // true
        {
            needReset = SetLifeEnded;
        }
        if (ParticleType == 3)
        {
            float3 vCurPivot = Center + Pivot;
            //float fDist = length(vCurPivot - pos);
            float fPivotRadius = 0.01f;
            float3 prevPos = pp.Translation.xyz;
            
            // === 선분-구체 교차 체크 ===
            float3 seg = pos - prevPos;
            float3 toPivot = vCurPivot - prevPos;

            // t = [0,1] 사이면 선분 위 교차
            float t = dot(toPivot, seg) / dot(seg, seg);
            t = saturate(t); // 0~1 clamp

            // 최근접 점과 pivot 거리
            float3 closest = prevPos + seg * t;
            float fDist = length(vCurPivot - closest);

            if (fDist < ShrinkThreshold)
            {
                if (isLoop != 0)
                    needReset = true;
                else
                    pp.LifeTime.y = pp.LifeTime.x;
            }
        }

        if (needReset)
        {
            pp.LifeTime.y = 0.0f;
            pp.Speed = gInitInst[i].Speed; // 속도 초기화는 유지
            pp.bFirstLoopDiscard = 0;
            pp.fTileIdx = 0.f;
            
            // initoffset을 사용해서 초기 모양 그대로 리셋할건지..
            //float3 offset0 = gInitInst[i].vInitOffset;
            //pos = Center + offset0; // << 변경점
            
            //// === 뼈 회전 적용된 방향으로 다시 세팅 ===
            //float3 localDir = gInitInst[i].Direction.xyz;
            //float3 worldDir = RotateByQuat(localDir, vSocketRot);
            //pp.Direction.xyz = normalize(worldDir);


            float3 vCurPivot = Center + Pivot;

            // range를 다시 사용해서 새롭게 다시 만들 건지..
                        
            if (isCircleRange == 0)
            {
                pos = RandomInRange(i * 3, Center.xyz, Range.xyz).xyz;
            }
            else
            {
                pos = RandomOnCircle(i * 3, Center.xyz, Range.x * 0.5f, Range.z).xyz;
            }
            

            if (ParticleType == 0)
            {
                pp.Direction = float4(normalize(pos.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 1)
            {
                pp.Direction = float4(normalize(Center.xyz - vCurPivot.xyz), 0.f);
            }
            else if (ParticleType == 2) // random
            {
                pp.Direction = float4(
                    Random(i * 3, -1.f, 1.f),
                    Random(i * 3 + 1, -1.f, 1.f),
                    Random(i * 3 + 2, -1.f, 1.f),
                    0.f
                );
            }
             else if (ParticleType == 3) // to center
             {
                 pp.Direction = float4(normalize(vCurPivot.xyz - pos.xyz), 0.f);
             }
            
            pp.vInitOffset = float3(
			    pos.x - Center.x,
			    pos.y - Center.y,
			    pos.z - Center.z
		    );
            
            //float3 localDir = gInitInst[i].Direction.xyz;
            float3 worldDir = RotateByQuat(pp.Direction.xyz, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);
            pp.VelocityDir = float4(0.f, 0.f, 0.f, 0.f);
        }
        pp.RotationAngle += pp.RotationSpeed;

        if (pp.RotationAngle > 6.28318530718f)
            pp.RotationAngle -= 6.28318530718f;
        
        uint iTileIdx = (uint) pp.fTileIdx;
        float fTileSizeX = 1.0f / float(vTileCnt.x);
        float fTileSizeY = 1.0f / float(vTileCnt.y);
        pp.vTileOffset = float2((iTileIdx % uint(vTileCnt.x)) * fTileSizeX, (iTileIdx / uint(vTileCnt.x)) * fTileSizeY);

        pp.Translation = float4(pos, 1.0f);

        gInst[i] = pp;
        return;
    }
}
