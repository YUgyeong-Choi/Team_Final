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

	struct TransitionResult
	{
		_bool bTransition = false; // 전환 여부
		_bool bBlendFullbody = true; // 블렌드 여부
		class CAnimation* pFromAnim = nullptr;
		class CAnimation* pToAnim = nullptr;
		_float fDuration = 0.f; // 전환 시간
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
		float				fIntensity;
		float				fRange;

		XMFLOAT4			vDiffuse;
		float				fAmbient;
		XMFLOAT4			vSpecular;
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


		static const unsigned int					iNumElements = { 6 };
		static const D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements];
	}VTXPOS_PARTICLE_INSTANCE;

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

}