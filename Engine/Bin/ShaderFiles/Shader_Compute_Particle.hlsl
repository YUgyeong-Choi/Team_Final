
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
    float _pad5;

    float3 Range; // vRange
    float _pad6;
    
    float4 vSocketRot;

    row_major float4x4 g_CombinedMatrix;
};

// t는 SRV, u는 UAV, b는 Constant Buffer
// 뒤의 숫자는 몇번째로 바인딩 했는지를 뜻함
// RWStructuredBuffer - Read / Write
// StructuredBuffer - Read only.
StructuredBuffer<ParticleParam> gInitInst : register(t0); 
RWStructuredBuffer<ParticleParam> gInst : register(u0);

//static const float PI = 3.14159265f;

float3 rotateAroundAxis(float3 v, float3 axis, float angleRad)
{
    float s = sin(angleRad), c = cos(angleRad);
    return v * c + cross(axis, v) * s + axis * dot(axis, v) * (1.0f - c);
}


float3 RotateByQuat(float3 v, float4 q)
{
    // q * v * q^-1
    float3 t = 2.0f * cross(q.xyz, v);
    return v + q.w * t + cross(q.xyz, t);
}
float Safe(float v)
{
    return (v == v) ? v : 0.0;
} // NaN이면 0으로
float Random_Normal(uint iSeed)
{
    float s = (float) iSeed * 12.9898 + (float) EffectSeed * 78.233 + fAccTime;
    //s = Safe(s); // 디버깅용
    float r = sin(s);
    //r = Safe(r);
    return frac(r * 43758.5453);
}

float Random(uint iSeed, float fMin, float fMax)
{
    return fMin + (fMax - fMin) * Random_Normal(iSeed);
}

float RandomEx(uint seed, float min1, float max1, float min2, float max2)
{
    // 두 구간 중 어떤 쪽을 쓸지 결정
    float pick = Random_Normal(seed); // 0~1
    if (pick < 0.5f)
        return Random(seed + 100, min1, max1);
    else
        return Random(seed + 200, min2, max2);
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
            pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);
            
            float3 localDir = gInitInst[i].Direction.xyz;
            float3 worldDir = RotateByQuat(localDir, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);

            /******************************************************************************/

            /* 이펙트 랜덤 생성 시 (bake 안 하고) */

           // float3 vCurPivot = Center + Pivot;
            
           // if (ParticleType == 0) // spread
           // {
           //     pp.Translation = float4(
			        //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
			        //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
			        //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f),
			        //1.f);
           //     pp.Direction = float4(normalize(pp.Translation.xyz - vCurPivot.xyz), 0.f);
           // }
           // else if (ParticleType == 1) // directional
           // {
           //     pp.Translation = float4(
			        //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
			        //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
			        //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f),
			        //1.f);
           //     pp.Direction = float4(normalize(Center.xyz - vCurPivot.xyz), 0.f);
           // }
           // else if (ParticleType == 2) // random
           // {
           //     pp.Translation = float4(
			        //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
			        //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
			        //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f),
			        //1.f);
           //     pp.Direction = float4(
           //         Random(i * 3    , -1.f, 1.f), 
           //         Random(i * 3 + 1, -1.f, 1.f),
           //         Random(i * 3 + 2, -1.f, 1.f),
           //         0.f
           //     );
           // }
           // else if (ParticleType == 3) // to pivot
           // {
           //     pp.Translation = float4(
			        //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
			        //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
			        //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f),
			        //1.f);
                
           //     pp.Direction = float4(normalize(vCurPivot.xyz - pp.Translation.xyz), 0.f);
           // }
            
            pp.vInitOffset = float3(
			    pp.Translation.x,
			    pp.Translation.y,
			    pp.Translation.z
		    );
            
            
            if (pp.RotationSpeed < 0.0001f)
                pp.RotationAngle = 0.f;
            else
                pp.RotationAngle = Random(pp.RotationSpeed, 0.f, 3.14f);
            
            //pp.Translation = float4(mul(float4(pp.Translation.xyz, 1.f), g_CombinedMatrix).xyz, 1.f);
            //float3 worldDir = RotateByQuat(pp.Direction.xyz, vSocketRot);
            //pp.Direction.xyz = normalize(worldDir);
            
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
        
        const bool firstLoop = (pp.bFirstLoopDiscard != 0);
        const bool lifeEnded = pp.LifeTime.y >= pp.LifeTime.x;
        const bool tileEnded = pp.fTileIdx >= vTileCnt.x * vTileCnt.y;
        
        float3 velocity = pos - prevPos;
        if (isFirst)
            pp.VelocityDir = float4(0.f, 0.f, 0.f, 0.f);
        else
            pp.VelocityDir = float4(normalize(velocity), length(velocity));
        
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

        if (isTileLoop != 0)
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
            // initoffset을 사용해서 초기 모양 그대로 리셋할건지..
            pp.LifeTime.y = 0.0f;
            pp.Speed = gInitInst[i].Speed; // 속도 초기화는 유지
            pp.bFirstLoopDiscard = 0;
            pp.fTileIdx = 0.f;
            float3 offset0 = gInitInst[i].vInitOffset;
            pos = Center + offset0; // << 변경점
            
            // === 뼈 회전 적용된 방향으로 다시 세팅 ===
            float3 localDir = gInitInst[i].Direction.xyz;
            float3 worldDir = RotateByQuat(localDir, vSocketRot);
            pp.Direction.xyz = normalize(worldDir);


            float3 vCurPivot = Center + Pivot;

            // range를 다시 사용해서 새롭게 다시 만들 건지..


        //     if (ParticleType == 0)
        //     {
        //         pos = float3(
		    	    //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
		    	    //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
		    	    //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f));
                 
        //         pp.Direction = float4(normalize(pos.xyz - vCurPivot.xyz), 0.f);
        //     }
        //     else if (ParticleType == 1)
        //     {
        //         pos = float3(
		    	    //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
		    	    //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
		    	    //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f));
                 
        //         pp.Direction = float4(normalize(Center.xyz - vCurPivot.xyz), 0.f);
        //     }
        //     else if (ParticleType == 2) // random
        //     {
        //         pos = float3(
		    	    //Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
		    	    //Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
		    	    //Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f));
                 
        //         pp.Direction = float4(
        //             Random(i * 3, -1.f, 1.f),
        //             Random(i * 3 + 1, -1.f, 1.f),
        //             Random(i * 3 + 2, -1.f, 1.f),
        //             0.f
        //         );
        //     }
        //     else if (ParticleType == 3) // to center
        //     {
        //         pos = float3(
		    	    //    Random(i * 3, Center.x - Range.x * 0.5f, Center.x + Range.x * 0.5f),
		    	    //    Random(i * 3 + 1, Center.y - Range.y * 0.5f, Center.y + Range.y * 0.5f),
		    	    //    Random(i * 3 + 2, Center.z - Range.z * 0.5f, Center.z + Range.z * 0.5f));
                 
        //         pp.Direction = float4(normalize(vCurPivot.xyz - pos.xyz), 0.f);
        //     }
            
            pp.vInitOffset = float3(
			    pos.x - Center.x,
			    pos.y - Center.y,
			    pos.z - Center.z
		    );
            
            //float3 localDir = gInitInst[i].Direction.xyz;
            //float3 worldDir = RotateByQuat(pp.Direction.xyz, vSocketRot);
            //pp.Direction.xyz = normalize(worldDir);
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
