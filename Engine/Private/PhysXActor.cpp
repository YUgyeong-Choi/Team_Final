#include "PhysXActor.h"
#include "DebugDraw.h"
#include "GameInstance.h"
CPhysXActor::CPhysXActor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent{ pDevice, pContext }
{
}

CPhysXActor::CPhysXActor(const CPhysXActor& Prototype)
    : CComponent(Prototype )
    , m_pBatch{ Prototype.m_pBatch }
    , m_pEffect{ Prototype.m_pEffect }
#ifdef _DEBUG
    , m_pInputLayout{ Prototype.m_pInputLayout }
#endif
{
#ifdef _DEBUG
    Safe_AddRef(m_pInputLayout);
#endif

}

void CPhysXActor::Set_ShapeFlag(_bool bSimulation, _bool bTrigger, _bool bQuery)
{
    m_pShape->setFlag(PxShapeFlag::eSIMULATION_SHAPE, bSimulation); // OnEnter, OnStay, OnExit 활성화
    m_pShape->setFlag(PxShapeFlag::eTRIGGER_SHAPE, bTrigger); //OnTriger 활성화
    m_pShape->setFlag(PxShapeFlag::eSCENE_QUERY_SHAPE, bQuery); // Ray충돌 활성화 
}

void CPhysXActor::Set_SimulationFilterData(PxFilterData _data)
{
    m_pShape->setSimulationFilterData(_data);
}

void CPhysXActor::Set_QueryFilterData(PxFilterData _data)
{
    m_pShape->setQueryFilterData(_data);
}


void CPhysXActor::On_Enter(CPhysXActor* pOther)
{
    if (m_pOwner && pOther->Get_Owner())
    {
        pOther->Get_Owner()->On_CollisionEnter(m_pOwner);
    }
}

void CPhysXActor::On_Stay(CPhysXActor* pOther)
{
    if (m_pOwner && pOther->Get_Owner())
    {
        pOther->Get_Owner()->On_CollisionStay(m_pOwner);
    }
}

void CPhysXActor::On_Exit(CPhysXActor* pOther)
{
    if (m_pOwner && pOther->Get_Owner())
    {
        pOther->Get_Owner()->On_CollisionExit(m_pOwner);
    }
}


void CPhysXActor::On_Trigger(CPhysXActor* pOther)
{
    if (m_pOwner && pOther->Get_Owner())
    {
        pOther->Get_Owner()->On_Trigger();
    }
}

HRESULT CPhysXActor::ReadyForDebugDraw(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
#ifdef _DEBUG
    m_pBatch = new PrimitiveBatch<VertexPositionColor>(pContext);
    m_pEffect = new BasicEffect(pDevice);

    const void* pShaderByteCode = { nullptr };
    size_t		iShaderByteCodeLength = {  };

    m_pEffect->SetVertexColorEnabled(true);

    m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

    if (FAILED(pDevice->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount,
        pShaderByteCode, iShaderByteCodeLength, &m_pInputLayout)))
        return E_FAIL;
#endif
    return S_OK;
}


HRESULT CPhysXActor::Render()
{
    DebugRender(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));

    for (auto& Ray : m_RenderRay) {
        DrawRay(m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW), m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ), Ray.vStartPos, Ray.vDirection, Ray.fRayLength, Ray.bIsHit, Ray.vHitPos);
    }
    m_RenderRay.clear();

    return S_OK;
}

void CPhysXActor::Add_RenderRay(DEBUGRAY_DATA _data)
{
    m_RenderRay.push_back(_data);
}

void CPhysXActor::DebugRender(const _matrix& view,const _matrix& proj, _float offSet)
{  
    PxTransform pose = PxShapeExt::getGlobalPose(*m_pShape, *m_pActor); 
    PxGeometryHolder geom = m_pShape->getGeometry();

    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(view);
    m_pEffect->SetProjection(proj);

    m_pContext->IASetInputLayout(m_pInputLayout);

    m_pEffect->Apply(m_pContext);


    m_pBatch->Begin();

    switch (geom.getType())
    {
    case PxGeometryType::eBOX:
    {
        BoundingOrientedBox obb;
        obb.Center = { pose.p.x, pose.p.y, pose.p.z };
        obb.Extents = { geom.box().halfExtents.x, geom.box().halfExtents.y, geom.box().halfExtents.z };
        obb.Orientation = { pose.q.x, pose.q.y, pose.q.z, pose.q.w };

        DX::Draw(m_pBatch, obb, Colors::Yellow);
        break;
    }

    case PxGeometryType::eSPHERE:
    {
        BoundingSphere sphere;
        sphere.Center = { pose.p.x, pose.p.y, pose.p.z };
        sphere.Radius = geom.sphere().radius;

        DX::Draw(m_pBatch, sphere, Colors::Cyan);
        break;
    }

    case PxGeometryType::eCAPSULE:
    {
        const PxCapsuleGeometry& capsule = geom.capsule();
        pose.p.y += offSet;
        DrawDebugCapsule(m_pBatch, pose, capsule.radius, capsule.halfHeight, Colors::Green);
        break;
    }
    case PxGeometryType::eTRIANGLEMESH:
    {
        DrawTriangleMesh(pose, geom);
        break;
    }
    case PxGeometryType::eCONVEXMESH:
    {
        DrawConvexMesh(pose, geom);
        break;
    }
    default:
        break;
    }

    m_pBatch->End();
}

void CPhysXActor::DrawDebugCapsule(PrimitiveBatch<VertexPositionColor>* pBatch, const PxTransform& pose, float radius, float halfHeight, FXMVECTOR color)
{

    constexpr int segmentCount = 24;
    const float step = XM_2PI / segmentCount;

    XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&pose.q)));
    XMVECTOR origin = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&pose.p));

    // ───────────────────────────────
   // 위 반구
    for (int i = 0; i < segmentCount / 2; ++i)
    {
        float theta0 = XM_PI * i / (segmentCount / 2);
        float theta1 = XM_PI * (i + 1) / (segmentCount / 2);

        float y0 = radius * sinf(theta0);
        float r0 = radius * cosf(theta0);
        float y1 = radius * sinf(theta1);
        float r1 = radius * cosf(theta1);

        // Z 방향 세로 단면
        XMVECTOR p0 = XMVectorSet(0, halfHeight + y0, r0, 1);
        XMVECTOR p1 = XMVectorSet(0, halfHeight + y1, r1, 1);

        p0 = XMVector3Transform(p0, rot) + origin;
        p1 = XMVector3Transform(p1, rot) + origin;
        pBatch->DrawLine(VertexPositionColor(p0, color), VertexPositionColor(p1, color));
    }

    //  X 방향 세로 단면 (위 반구, XY 평면)
    for (int i = 0; i < segmentCount / 2; ++i)
    {
        float theta0 = XM_PI * i / (segmentCount / 2);
        float theta1 = XM_PI * (i + 1) / (segmentCount / 2);

        float y0 = radius * sinf(theta0);
        float r0 = radius * cosf(theta0);
        float y1 = radius * sinf(theta1);
        float r1 = radius * cosf(theta1);

        XMVECTOR p0 = XMVectorSet(r0, halfHeight + y0, 0, 1);
        XMVECTOR p1 = XMVectorSet(r1, halfHeight + y1, 0, 1);

        p0 = XMVector3Transform(p0, rot) + origin;
        p1 = XMVector3Transform(p1, rot) + origin;
        pBatch->DrawLine(VertexPositionColor(p0, color), VertexPositionColor(p1, color));
    }

    // 아래 반구
    for (int i = 0; i < segmentCount / 2; ++i)
    {
        float theta0 = XM_PI * i / (segmentCount / 2);
        float theta1 = XM_PI * (i + 1) / (segmentCount / 2);

        float y0 = radius * sinf(theta0);
        float r0 = radius * cosf(theta0);
        float y1 = radius * sinf(theta1);
        float r1 = radius * cosf(theta1);

        // Z 방향 세로 단면
        XMVECTOR p0 = XMVectorSet(0, -halfHeight - y0, r0, 1);
        XMVECTOR p1 = XMVectorSet(0, -halfHeight - y1, r1, 1);

        p0 = XMVector3Transform(p0, rot) + origin;
        p1 = XMVector3Transform(p1, rot) + origin;
        pBatch->DrawLine(VertexPositionColor(p0, color), VertexPositionColor(p1, color));
    }

    //  X 방향 세로 단면 (아래 반구, XY 평면)
    for (int i = 0; i < segmentCount / 2; ++i)
    {
        float theta0 = XM_PI * i / (segmentCount / 2);
        float theta1 = XM_PI * (i + 1) / (segmentCount / 2);

        float y0 = radius * sinf(theta0);
        float r0 = radius * cosf(theta0);
        float y1 = radius * sinf(theta1);
        float r1 = radius * cosf(theta1);

        XMVECTOR p0 = XMVectorSet(r0, -halfHeight - y0, 0, 1);
        XMVECTOR p1 = XMVectorSet(r1, -halfHeight - y1, 0, 1);

        p0 = XMVector3Transform(p0, rot) + origin;
        p1 = XMVector3Transform(p1, rot) + origin;
        pBatch->DrawLine(VertexPositionColor(p0, color), VertexPositionColor(p1, color));
    }

    // 3. 수평 링 (위/아래 XZ 평면)
    for (int i = 0; i < segmentCount; ++i)
    {
        float theta0 = step * i;
        float theta1 = step * (i + 1);

        float x0 = radius * cosf(theta0);
        float z0 = radius * sinf(theta0);
        float x1 = radius * cosf(theta1);
        float z1 = radius * sinf(theta1);

        XMVECTOR pt0 = XMVectorSet(x0, +halfHeight, z0, 1);
        XMVECTOR pt1 = XMVectorSet(x1, +halfHeight, z1, 1);
        XMVECTOR pb0 = XMVectorSet(x0, -halfHeight, z0, 1);
        XMVECTOR pb1 = XMVectorSet(x1, -halfHeight, z1, 1);

        pt0 = XMVector3Transform(pt0, rot) + origin;
        pt1 = XMVector3Transform(pt1, rot) + origin;
        pb0 = XMVector3Transform(pb0, rot) + origin;
        pb1 = XMVector3Transform(pb1, rot) + origin;

        pBatch->DrawLine(VertexPositionColor(pt0, color), VertexPositionColor(pt1, color));
        pBatch->DrawLine(VertexPositionColor(pb0, color), VertexPositionColor(pb1, color));
    }

    //  세로 라인 4개 추가
    for (int i = 0; i < 4; ++i)
    {
        float angle = XM_PIDIV2 * i; // 0, 90, 180, 270
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);

        XMVECTOR top = XMVectorSet(x, +halfHeight, z, 1.f);
        XMVECTOR bottom = XMVectorSet(x, -halfHeight, z, 1.f);

        top = XMVector3Transform(top, rot) + origin;
        bottom = XMVector3Transform(bottom, rot) + origin;

        pBatch->DrawLine(VertexPositionColor(top, color), VertexPositionColor(bottom, color));
    }

}

void CPhysXActor::DrawTriangleMesh(PxTransform pose, PxGeometryHolder geom)
{
    const PxTriangleMeshGeometry& meshGeom = geom.triangleMesh();
    const PxTriangleMesh* mesh = meshGeom.triangleMesh;
    if (!mesh)
        return;

    const PxVec3* verts = mesh->getVertices();
    PxU32 numVerts = mesh->getNbVertices();

    const void* tris = mesh->getTriangles();
    PxU32 numTris = mesh->getNbTriangles();

    const bool use16bit = mesh->getTriangleMeshFlags().isSet(PxTriangleMeshFlag::e16_BIT_INDICES);

    // pose를 월드 행렬로 변환
    const PxVec3& scale = meshGeom.scale.scale; // 여기가 핵심!
    XMMATRIX matPose = XMMatrixAffineTransformation(
        XMVectorSet(scale.x, scale.y, scale.z, 0.f),  // 실제 스케일 반영
        XMVectorZero(),
        XMVectorSet(pose.q.x, pose.q.y, pose.q.z, pose.q.w),
        XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.f));

    for (PxU32 i = 0; i < numTris; ++i)
    {
        PxU32 i0, i1, i2;
        if (use16bit)
        {
            const PxU16* indices = reinterpret_cast<const PxU16*>(tris);
            i0 = indices[i * 3 + 0];
            i1 = indices[i * 3 + 1];
            i2 = indices[i * 3 + 2];
        }
        else
        {
            const PxU32* indices = reinterpret_cast<const PxU32*>(tris);
            i0 = indices[i * 3 + 0];
            i1 = indices[i * 3 + 1];
            i2 = indices[i * 3 + 2];
        }

        if (i0 >= numVerts || i1 >= numVerts || i2 >= numVerts)
            continue; // 방어 코드

        // 정점  XMVECTOR  pose 적용
        XMVECTOR v0 = XMVector3TransformCoord(XMLoadFloat3((XMFLOAT3*)&verts[i0]), matPose);
        XMVECTOR v1 = XMVector3TransformCoord(XMLoadFloat3((XMFLOAT3*)&verts[i1]), matPose);
        XMVECTOR v2 = XMVector3TransformCoord(XMLoadFloat3((XMFLOAT3*)&verts[i2]), matPose);

        // 삼각형 테두리 선으로 렌더
        m_pBatch->DrawLine(VertexPositionColor(v0, Colors::Red), VertexPositionColor(v1, Colors::Red));
        m_pBatch->DrawLine(VertexPositionColor(v1, Colors::Red), VertexPositionColor(v2, Colors::Red));
        m_pBatch->DrawLine(VertexPositionColor(v2, Colors::Red), VertexPositionColor(v0, Colors::Red));
    }


    // === AABB 시각화 추가 ===
    PxBounds3 bounds = PxShapeExt::getWorldBounds(*m_pShape, *m_pActor);
    BoundingBox aabb;
    aabb.Center = XMFLOAT3(
        (bounds.minimum.x + bounds.maximum.x) * 0.5f,
        (bounds.minimum.y + bounds.maximum.y) * 0.5f,
        (bounds.minimum.z + bounds.maximum.z) * 0.5f
    );
    aabb.Extents = XMFLOAT3(
        (bounds.maximum.x - bounds.minimum.x) * 0.5f,
        (bounds.maximum.y - bounds.minimum.y) * 0.5f,
        (bounds.maximum.z - bounds.minimum.z) * 0.5f
    );
    DX::Draw(m_pBatch, aabb, Colors::Yellow); // AABB는 노란색으로

}

void CPhysXActor::DrawConvexMesh(PxTransform pose, PxGeometryHolder geom)
{
    const PxConvexMeshGeometry& convexGeom = geom.convexMesh();
    const PxConvexMesh* convex = convexGeom.convexMesh;
    if (!convex)
        return;

    const PxVec3* verts = convex->getVertices();
    const PxU32 numVerts = convex->getNbVertices();
    const PxU8* indices = convex->getIndexBuffer();
    const PxU32 numPolys = convex->getNbPolygons();

    PxHullPolygon poly;

    // 월드 변환 행렬 생성
    const PxVec3& scale = convexGeom.scale.scale;
    XMMATRIX matPose = XMMatrixAffineTransformation(
        XMVectorSet(scale.x, scale.y, scale.z, 0.f),
        XMVectorZero(),
        XMVectorSet(pose.q.x, pose.q.y, pose.q.z, pose.q.w),
        XMVectorSet(pose.p.x, pose.p.y, pose.p.z, 1.f));

    for (PxU32 i = 0; i < numPolys; ++i)
    {
        if (!convex->getPolygonData(i, poly))
            continue;

        const PxU32 polyVertCount = poly.mNbVerts;
        const PxU8* polyIndices = indices + poly.mIndexBase;

        for (PxU32 j = 1; j + 1 < polyVertCount; ++j)
        {
            PxU32 i0 = polyIndices[0];
            PxU32 i1 = polyIndices[j];
            PxU32 i2 = polyIndices[j + 1];

            if (i0 >= numVerts || i1 >= numVerts || i2 >= numVerts)
                continue;

            XMVECTOR v0 = XMVector3TransformCoord(XMLoadFloat3((XMFLOAT3*)&verts[i0]), matPose);
            XMVECTOR v1 = XMVector3TransformCoord(XMLoadFloat3((XMFLOAT3*)&verts[i1]), matPose);
            XMVECTOR v2 = XMVector3TransformCoord(XMLoadFloat3((XMFLOAT3*)&verts[i2]), matPose);

            m_pBatch->DrawLine(VertexPositionColor(v0, Colors::Orange), VertexPositionColor(v1, Colors::Orange));
            m_pBatch->DrawLine(VertexPositionColor(v1, Colors::Orange), VertexPositionColor(v2, Colors::Orange));
            m_pBatch->DrawLine(VertexPositionColor(v2, Colors::Orange), VertexPositionColor(v0, Colors::Orange));
        }
    }
}

void CPhysXActor::DrawRay(const _matrix& view, const _matrix& proj, const PxVec3& origin, const PxVec3& dir, float length, _bool drawHitBox, PxVec3 hitPos)
{
    if (!m_pBatch || !m_pEffect || !m_pContext)
        return;

    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(view);
    m_pEffect->SetProjection(proj);

    m_pContext->IASetInputLayout(m_pInputLayout);
    m_pEffect->Apply(m_pContext);
    m_pBatch->Begin();

    XMVECTOR start = XMVectorSet(origin.x, origin.y, origin.z, 1.f);
    XMVECTOR end = XMVectorSet(origin.x + dir.x * length, origin.y + dir.y * length, origin.z + dir.z * length, 1.f);

    m_pBatch->DrawLine(VertexPositionColor(start, Colors::Cyan), VertexPositionColor(end, Colors::Cyan));

    if (drawHitBox)
    {
        // 간단한 박스형태 선들로 hit 표시
        float size = 0.1f;
        float x = hitPos.x;
        float y = hitPos.y;
        float z = hitPos.z;

        XMVECTOR p1 = XMVectorSet(x - size, y, z - size, 1.f);
        XMVECTOR p2 = XMVectorSet(x + size, y, z - size, 1.f);
        XMVECTOR p3 = XMVectorSet(x + size, y, z + size, 1.f);
        XMVECTOR p4 = XMVectorSet(x - size, y, z + size, 1.f);

        m_pBatch->DrawLine(VertexPositionColor(p1, Colors::Red), VertexPositionColor(p2, Colors::Red));
        m_pBatch->DrawLine(VertexPositionColor(p2, Colors::Red), VertexPositionColor(p3, Colors::Red));
        m_pBatch->DrawLine(VertexPositionColor(p3, Colors::Red), VertexPositionColor(p4, Colors::Red));
        m_pBatch->DrawLine(VertexPositionColor(p4, Colors::Red), VertexPositionColor(p1, Colors::Red));
    }

    m_pBatch->End();
}


void CPhysXActor::Free()
{
#ifdef _DEBUG
    Safe_Release(m_pInputLayout);
    if (false == m_isCloned)
    {
        Safe_Delete(m_pBatch);
        Safe_Delete(m_pEffect);
    }
#endif

    __super::Free();
}
