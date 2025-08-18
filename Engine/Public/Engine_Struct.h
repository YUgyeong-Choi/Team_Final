#pragma once

namespace Engine
{
	typedef struct tagEngineDesc 
	{
		HINSTANCE		hInstance;
		HWND			hWnd;
		bool			isWindowed;
		unsigned int	iWinSizeX;
		unsigned int	iWinSizeY;
		unsigned int	iNumLevels;		
	}ENGINE_DESC;


	struct AnimationEvent
	{
		float fTime{ 0.f };
		string name;
	};

	struct Link
	{
		int iLinkId; // 링크 ID
		int iLinkStartID; // 시작 Pin ID
		int iLinkEndID; // 끝 Pin ID
	};

	struct Parameter
	{
		string name;
		ParamType type;
		bool	bTriggered = false; 
		bool	bValue = false; // bool 값
		int		iValue = 0; // int 값
		float	fValue = 0.f; // float 값
	};

	struct OverrideAnimController
	{
		string name;
		string controllerName; // 애니메이션 컨트롤러 이름
		struct OverrideState
		{
			string clipName; // 애니메이션 클립 이름
			string upperClipName; // 상체 애니메이션 이름
			string lowerClipName; // 하체 애니메이션 이름
			string maskBoneName; // 마스크 본 이름 (없으면 빈 문자열)
			float fBlendWeight = 1.f; // 블렌드 가중치 (0~1 사이)
			float fLowerStartTime = 0.f; // 하체 애니메이션 시작 시간
			float fUpperStartTime = 0.f; // 상체 애니메이션 시작 시간	
		};

		unordered_map<string, OverrideState> states; // 상태 이름, OverrideState
	};

	typedef struct tagKeyFrame
	{
		/* 행렬이 아닌이유? : 상태와 상태 사이를 보간해주기위해서. */
		XMFLOAT3		vScale;
		XMFLOAT4		vRotation;
		XMFLOAT3		vTranslation;

		/* 이 상태를 취해야하는 재생위치  */
		float			fTrackPosition;
	}KEYFRAME;

	typedef struct tagLightDesc
	{
		enum TYPE { TYPE_DIRECTIONAL, TYPE_SPOT, TYPE_POINT, TYPE_END };

		TYPE				eType;
		XMFLOAT4			vDirection;
		XMFLOAT4			vPosition;
		_float				fIntensity;
		_float				fRange;

		XMFLOAT4			vDiffuse;
		_float				fAmbient;
		XMFLOAT4			vSpecular;

		_float				fInnerCosAngle;
		_float				fOuterCosAngle;
		_float				fFalloff;

		_float				fFogDensity;
		_float				fFogCutoff;

		_bool				bIsVolumetric;
		_bool				bIsPlayerFar;
		_bool				bIsUse;

	}LIGHT_DESC;


	/* 화면에 그려야하는 최종적인 색 = Light.Diffuse * 재질.Diffuse */
	typedef struct ENGINE_DLL tagVertexPosition
	{
		XMFLOAT3		vPosition;		

		static const unsigned int					iNumElements = { 1 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXPOS;

	typedef struct ENGINE_DLL tagVertexPositionTexcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vTexcoord;

		static const unsigned int					iNumElements = {2};
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXPOSTEX;

	typedef struct ENGINE_DLL tagVertexCube
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vTexcoord;

		static const unsigned int					iNumElements = { 2 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXCUBE;

	typedef struct ENGINE_DLL tagVertexPositionNormalTexcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		static const unsigned int					iNumElements = { 3 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXNORTEX;

	typedef struct ENGINE_DLL tagVertexMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT3		vTangent;
		XMFLOAT2		vTexcoord;

		static const unsigned int					iNumElements = { 4 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXMESH;

	typedef struct ENGINE_DLL tagVertexAnimMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT3		vTangent;
		XMFLOAT2		vTexcoord;
		XMUINT4			vBlendIndices;
		XMFLOAT4		vBlendWeights;

		static const unsigned int					iNumElements = { 6 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXANIMMESH;

	typedef struct ENGINE_DLL tagVertexMeshInstance
	{
		XMFLOAT4		vRight;
		XMFLOAT4		vUp;
		XMFLOAT4		vLook;
		XMFLOAT4		vTranslation;

		static const unsigned int					iNumElements = { 8 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXMESH_INSTANCE;

	typedef struct ENGINE_DLL tagVertexRectParticleInstance
	{
		XMFLOAT4		vRight;
		XMFLOAT4		vUp;
		XMFLOAT4		vLook;
		XMFLOAT4		vTranslation;		
		XMFLOAT2		vLifeTime;		

		static const unsigned int					iNumElements = { 7 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXRECT_PARTICLE_INSTANCE;
	
	typedef struct ENGINE_DLL tagVertexPointParticleInstance
	{
		XMFLOAT4		vRight;
		XMFLOAT4		vUp;
		XMFLOAT4		vLook;
		XMFLOAT4		vTranslation;

		XMFLOAT2		vLifeTime;
		XMFLOAT2		_pad0;


		static const unsigned int					iNumElements = { 6 };	
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXPOS_PARTICLE_INSTANCE;

	typedef struct ENGINE_DLL tagVertexPointTrail
	{
		XMFLOAT3		vOuterPos;
		XMFLOAT3		vInnerPos;
		XMFLOAT2		vLifeTime;
		float			fVCoord;

		static const unsigned int					iNumElements = { 4 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXPOS_TRAIL;


	struct ENGINE_DLL AABBBOX {
		_float3 vMin;
		_float3 vMax;
	};

	inline XMFLOAT3 AABB_Center(const AABBBOX& b)
	{
		return XMFLOAT3(
			0.5f * (b.vMin.x + b.vMax.x),
			0.5f * (b.vMin.y + b.vMax.y),
			0.5f * (b.vMin.z + b.vMax.z));
	}

	inline XMFLOAT3 AABB_Extents(const AABBBOX& b) // 반측 길이(half-size)
	{
		return XMFLOAT3(
			0.5f * (b.vMax.x - b.vMin.x),
			0.5f * (b.vMax.y - b.vMin.y),
			0.5f * (b.vMax.z - b.vMin.z));
	}

	inline XMFLOAT3 AABB_Size(const AABBBOX& b) // 전체 길이
	{
		return XMFLOAT3(
			(b.vMax.x - b.vMin.x),
			(b.vMax.y - b.vMin.y),
			(b.vMax.z - b.vMin.z));
	}

	// 대각선의 절반 길이(LOD)
	inline float AABB_HalfDiagonal(const AABBBOX& b)
	{
		XMFLOAT3 e = AABB_Extents(b);
		return sqrtf(e.x * e.x + e.y * e.y + e.z * e.z);
	}

	inline void AABB_Inflate(AABBBOX& b, float eps) // 경계 떨림 완화 등
	{
		b.vMin.x -= eps; b.vMin.y -= eps; b.vMin.z -= eps;
		b.vMax.x += eps; b.vMax.y += eps; b.vMax.z += eps;
	}

	inline void AABB_ExpandByPoint(AABBBOX& b, const XMFLOAT3& p)
	{
		b.vMin.x = (std::min)(b.vMin.x, p.x);
		b.vMin.y = (std::min)(b.vMin.y, p.y);
		b.vMin.z = (std::min)(b.vMin.z, p.z);

		b.vMax.x = (std::max)(b.vMax.x, p.x);
		b.vMax.y = (std::max)(b.vMax.y, p.y);
		b.vMax.z = (std::max)(b.vMax.z, p.z);
	}

	inline void AABB_ExpandByAABB(AABBBOX& b, const AABBBOX& o)
	{
		b.vMin.x = (min)(b.vMin.x, o.vMin.x);
		b.vMin.y = (min)(b.vMin.y, o.vMin.y);
		b.vMin.z = (min)(b.vMin.z, o.vMin.z);

		b.vMax.x = (max)(b.vMax.x, o.vMax.x);
		b.vMax.y = (max)(b.vMax.y, o.vMax.y);		
		b.vMax.z = (max)(b.vMax.z, o.vMax.z);
	}

	inline AABBBOX MakeLightAABB_Point(const _float3& vCenter, _float fRange)
	{
		AABBBOX tBox{};
		tBox.vMin = { vCenter.x - fRange, vCenter.y - fRange, vCenter.z - fRange };
		tBox.vMax = { vCenter.x + fRange, vCenter.y + fRange, vCenter.z + fRange };
		return tBox;
	}

	/*---------------------------
		포함/교차 판정
	---------------------------*/
	inline _bool AABB_ContainsPoint(const AABBBOX& b, const XMFLOAT3& p, float eps = 0.0f)
	{
		return (p.x >= b.vMin.x - eps && p.x <= b.vMax.x + eps) &&
			(p.y >= b.vMin.y - eps && p.y <= b.vMax.y + eps) &&
			(p.z >= b.vMin.z - eps && p.z <= b.vMax.z + eps);
	}

	// a가 b를 완전히 포함?
	inline _bool AABB_ContainsAABB(const AABBBOX& a, const AABBBOX& b, float eps = 0.0f)
	{
		return (b.vMin.x >= a.vMin.x - eps) && (b.vMax.x <= a.vMax.x + eps) &&
			//(b.vMin.y >= a.vMin.y - eps) && (b.vMax.y <= a.vMax.y + eps) &&
			(b.vMin.z >= a.vMin.z - eps) && (b.vMax.z <= a.vMax.z + eps);
	}

	// 교차(겹침) 여부
	inline _bool AABB_IntersectsAABB(const AABBBOX& a, const AABBBOX& b)
	{
		if (a.vMax.x < b.vMin.x || a.vMin.x > b.vMax.x) return false;
		if (a.vMax.y < b.vMin.y || a.vMin.y > b.vMax.y) return false;
		if (a.vMax.z < b.vMin.z || a.vMin.z > b.vMax.z) return false;
		return true;
	}

	// 점과 상자 거리
	inline _float AABB_DistanceSqToPoint(const AABBBOX& b, const XMFLOAT3& p)
	{
		float dx = 0.f, dy = 0.f, dz = 0.f;

		if (p.x < b.vMin.x) dx = b.vMin.x - p.x;
		else if (p.x > b.vMax.x) dx = p.x - b.vMax.x;

		if (p.y < b.vMin.y) dy = b.vMin.y - p.y;
		else if (p.y > b.vMax.y) dy = p.y - b.vMax.y;

		if (p.z < b.vMin.z) dz = b.vMin.z - p.z;
		else if (p.z > b.vMax.z) dz = p.z - b.vMax.z;

		return dx * dx + dy * dy + dz * dz;
	}
	struct DebugLine { XMFLOAT3 a, b; XMFLOAT4 color; };
	static void PushAABBLines(const AABBBOX& b, const XMFLOAT4& col, vector<DebugLine>& out)
	{
		XMFLOAT3 v[8] = {
			{b.vMin.x,b.vMin.y,b.vMin.z},{b.vMax.x,b.vMin.y,b.vMin.z},
			{b.vMax.x,b.vMin.y,b.vMax.z},{b.vMin.x,b.vMin.y,b.vMax.z},
			{b.vMin.x,b.vMax.y,b.vMin.z},{b.vMax.x,b.vMax.y,b.vMin.z},
			{b.vMax.x,b.vMax.y,b.vMax.z},{b.vMin.x,b.vMax.y,b.vMax.z},
		};
		auto L = [&](int i, int j) { out.push_back({ v[i],v[j],col }); };
		// 아래 사각
		L(0, 1); L(1, 2); L(2, 3); L(3, 0);
		// 위 사각
		L(4, 5); L(5, 6); L(6, 7); L(7, 4);
		// 기둥
		L(0, 4); L(1, 5); L(2, 6); L(3, 7);
	}

	inline BoundingBox ToDXBox(const AABBBOX& aabb)
	{
		DirectX::BoundingBox box;
		box.Center.x = (aabb.vMin.x + aabb.vMax.x) * 0.5f;
		box.Center.y = (aabb.vMin.y + aabb.vMax.y) * 0.5f;
		box.Center.z = (aabb.vMin.z + aabb.vMax.z) * 0.5f;
		box.Extents.x = (aabb.vMax.x - aabb.vMin.x) * 0.5f;
		box.Extents.y = (aabb.vMax.y - aabb.vMin.y) * 0.5f;
		box.Extents.z = (aabb.vMax.z - aabb.vMin.z) * 0.5f;
		return box;
	}

	inline _float3 ExtractAABBWorldCorner(const _float3& vMin, const _float3& vMax, _int iIndex)
	{
		_float3 vCorner;

		// X
		vCorner.x = (iIndex & 1) ? vMax.x : vMin.x;
		// Y
		vCorner.y = (iIndex & 2) ? vMax.y : vMin.y;
		// Z
		vCorner.z = (iIndex & 4) ? vMax.z : vMin.z;

		return vCorner;
	}

	typedef struct ENGINE_DLL Area
	{
		enum class EAreaType : _int { ROOM, LOBBY, INDOOR, OUTDOOR, END};

		_int iAreaState;
		_int iAreaId;
		AABBBOX vBounds;
		vector<_uint> vecAdjacent;

		EAreaType eType;
		_int iPriority;

		bool ContainsPoint(const _float3& point) const
		{
			return (point.x >= vBounds.vMin.x && point.x <= vBounds.vMax.x &&
				point.y >= vBounds.vMin.y && point.y <= vBounds.vMax.y &&
				point.z >= vBounds.vMin.z && point.z <= vBounds.vMax.z);
		}
	}AREA;

	typedef struct ENGINE_DLL Handle
	{
		unsigned int id; // 오브젝트를 구별할 번호
		bool operator==(const Handle& other) const noexcept
		{
			return id == other.id;
		}
		bool operator<(const Handle& other) const noexcept {
			return id < other.id;
		}
	}HANDLEID;

#pragma region Model_Binary

	typedef struct tagBoneData
	{
		string		strBoneName;
		/* XMMatrixTranspose 후 저장할 것 !!! */
		_float4x4	TransformMatrix = {};
		_int		iParentBoneIndex = { -2 };
	}FBX_BONEDATA;

	typedef struct tagChannelData
	{
		_uint iNumKeyFrames = {};
		vector<KEYFRAME> vecKeyFrames;
		_uint iBoneIndex = {};
	}FBX_CHANNELDATA;

	typedef struct tagAnimationData
	{
		_uint	iNumChannels = {};
		_float	fDuration = {};
		_float	fTicksPerSecond = {};
		vector<FBX_CHANNELDATA> vecChannels;
	}FBX_ANIMDATA;

	typedef struct tagMaterialData
	{
		aiTextureType eTexType;
		string strTexturePath;
	}FBX_MATDATA;

	typedef struct tagAnimMeshData
	{
		_uint iMaterialIndex = {};
		_uint iNumVertices = {};
		_uint iNumIndices = {};
		_uint iNumBones = {};

		vector<_uint> vecBoneIndices;
		vector<VTXANIMMESH> vecVertices;
	}FBX_ANIMMESHDATA;


	typedef struct tagMeshData
	{
		string strMeshName;
		_uint iMaterialIndex = {};
		_uint iNumVertices = {};
		_uint iNumIndices = {};
		vector<_uint>	vecIndices;
		vector<VTXMESH> vecVertices;
	}FBX_MESHDATA;

	typedef struct tagFBXData
	{
		_uint iNumMeshes = {};
		_uint iNumMaterials = {};
		_uint iNumAnimations = {};
		string strFBXName;
	}FBX_MODELDATA;

#pragma endregion

	typedef struct tagDebugRay
	{
		PxVec3 vStartPos = {};
		PxVec3 vDirection = {};
		_float fRayLength = {};
		_bool bIsHit = false;
		PxVec3 vHitPos = {};
	}DEBUGRAY_DATA;


	/* [ 여기 수정할 땐 꼭 CS hlsl도 동시에 수정해주기 바람.... ] */
	//typedef struct tagParticleDesc {
	//	_float4		vDirection = {};

	//	_float		fSpeed = {};
	//	_float		fRotationSpeed = {}; // 자전 속도
	//	_float		fOrbitSpeed = {}; // 공전 속도
	//	_float		fAccel;        // 가속도 (+면 가속, -면 감속)

	//	_float		fMaxSpeed;     // 최대 속도 (옵션)
	//	_float		fMinSpeed;     // 최소 속도 (옵션, 감속 시 멈춤 방지)
	//	_float2		_pad0;
	//}PARTICLEDESC;

	/* PARTICLEDESC + VTXPOSINSTANCE */
	typedef struct tagParticleParamDesc{
		_float4		vRight;
		_float4		vUp;
		_float4		vLook;
		_float4		vTranslation; // WorldMatrix
	
		_float4		vDirection = {};
	
		_float2		vLifeTime;
		_float		fSpeed = {};
		_float		fRotationSpeed = {}; // 자전 속도
	
		_float		fOrbitSpeed = {}; // 공전 속도
		_float		fAccel;        // 가속도 (+면 가속, -면 감속
		_float		fMaxSpeed;     // 최대 속도 (옵션)
		_float		fMinSpeed;     // 최소 속도 (옵션, 감속 시 멈춤 방지)

	}PPDESC;

	typedef struct tagParticleCBuffer {
		_float		fDeltaTime;       // dt (tool이면 무시)
		_float		fTrackTime;       // tool 절대시간(초) = curTrackPos/60.f
		_uint		iParticleType;    // 0:SPREAD, 1:DIRECTIONAL
		_uint		iNumInstances;

		_uint		bIsTool;			// uint == bool
		_uint		bIsLoop;
		_uint		bUseGravity;
		_uint		bUseSpin;

		_uint		bUseOrbit;
		_float		fGravity;        // e.g. 9.8
		_float2		_pad0;

		_float3		vPivot;          // vPivot
		_float		_pad1;

		_float3		vCenter;         // vCenter
		_float		_pad2;

		_float3		vOrbitAxis;      // normalized
		_float		_pad3;

		_float3		vRotationAxis;   // normalized
		_float		_pad4;
	}PARTICLECBUFFER;

} 