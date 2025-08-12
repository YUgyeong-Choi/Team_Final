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
		int iLinkId; // ��ũ ID
		int iLinkStartID; // ���� Pin ID
		int iLinkEndID; // �� Pin ID
	};

	struct Parameter
	{
		string name;
		ParamType type;
		bool	bTriggered = false; 
		bool	bValue = false; // bool ��
		int		iValue = 0; // int ��
		float	fValue = 0.f; // float ��
	};

	struct OverrideAnimController
	{
		string name;
		string controllerName; // �ִϸ��̼� ��Ʈ�ѷ� �̸�
		struct OverrideState
		{
			string clipName; // �ִϸ��̼� Ŭ�� �̸�
			string upperClipName; // ��ü �ִϸ��̼� �̸�
			string lowerClipName; // ��ü �ִϸ��̼� �̸�
			string maskBoneName; // ����ũ �� �̸� (������ �� ���ڿ�)
			float fBlendWeight = 1.f; // ���� ����ġ (0~1 ����)
			float fLowerStartTime = 0.f; // ��ü �ִϸ��̼� ���� �ð�
			float fUpperStartTime = 0.f; // ��ü �ִϸ��̼� ���� �ð�
		};

		unordered_map<string, OverrideState> states; // ���� �̸�, OverrideState
	};

	typedef struct tagKeyFrame
	{
		/* ����� �ƴ�����? : ���¿� ���� ���̸� �������ֱ����ؼ�. */
		XMFLOAT3		vScale;
		XMFLOAT4		vRotation;
		XMFLOAT3		vTranslation;

		/* �� ���¸� ���ؾ��ϴ� �����ġ  */
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
	}LIGHT_DESC;


	/* ȭ�鿡 �׷����ϴ� �������� �� = Light.Diffuse * ����.Diffuse */
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
		//XMFLOAT4		vDirection;			// CS�� �ű� ��
		

		static const unsigned int					iNumElements = { 6 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXPOS_PARTICLE_INSTANCE;

	typedef struct ENGINE_DLL tagVertexPointTrail
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vPSize;
		XMFLOAT2		vLifeTime;


		static const unsigned int					iNumElements = { 3 };
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

	inline XMFLOAT3 AABB_Extents(const AABBBOX& b) // ���� ����(half-size)
	{
		return XMFLOAT3(
			0.5f * (b.vMax.x - b.vMin.x),
			0.5f * (b.vMax.y - b.vMin.y),
			0.5f * (b.vMax.z - b.vMin.z));
	}

	inline XMFLOAT3 AABB_Size(const AABBBOX& b) // ��ü ����
	{
		return XMFLOAT3(
			(b.vMax.x - b.vMin.x),
			(b.vMax.y - b.vMin.y),
			(b.vMax.z - b.vMin.z));
	}

	// �밢���� ���� ����(LOD)
	inline float AABB_HalfDiagonal(const AABBBOX& b)
	{
		XMFLOAT3 e = AABB_Extents(b);
		return sqrtf(e.x * e.x + e.y * e.y + e.z * e.z);
	}

	inline void AABB_Inflate(AABBBOX& b, float eps) // ��� ���� ��ȭ ��
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

	/*---------------------------
		����/���� ����
	---------------------------*/
	inline bool AABB_ContainsPoint(const AABBBOX& b, const XMFLOAT3& p, float eps = 0.0f)
	{
		return (p.x >= b.vMin.x - eps && p.x <= b.vMax.x + eps) &&
			(p.y >= b.vMin.y - eps && p.y <= b.vMax.y + eps) &&
			(p.z >= b.vMin.z - eps && p.z <= b.vMax.z + eps);
	}

	// a�� b�� ������ ����?
	inline bool AABB_ContainsAABB(const AABBBOX& a, const AABBBOX& b, float eps = 0.0f)
	{
		return (b.vMin.x >= a.vMin.x - eps) && (b.vMax.x <= a.vMax.x + eps) &&
			//(b.vMin.y >= a.vMin.y - eps) && (b.vMax.y <= a.vMax.y + eps) &&
			(b.vMin.z >= a.vMin.z - eps) && (b.vMax.z <= a.vMax.z + eps);
	}

	// ����(��ħ) ����
	inline bool AABB_IntersectsAABB(const AABBBOX& a, const AABBBOX& b)
	{
		if (a.vMax.x < b.vMin.x || a.vMin.x > b.vMax.x) return false;
		if (a.vMax.y < b.vMin.y || a.vMin.y > b.vMax.y) return false;
		if (a.vMax.z < b.vMin.z || a.vMin.z > b.vMax.z) return false;
		return true;
	}

	// ���� ���� �Ÿ�
	inline float AABB_DistanceSqToPoint(const AABBBOX& b, const XMFLOAT3& p)
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
		// �Ʒ� �簢
		L(0, 1); L(1, 2); L(2, 3); L(3, 0);
		// �� �簢
		L(4, 5); L(5, 6); L(6, 7); L(7, 4);
		// ���
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


	typedef struct ENGINE_DLL Handle
	{
		unsigned int id; // ������Ʈ�� ������ ��ȣ
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
		/* XMMatrixTranspose �� ������ �� !!! */
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

}