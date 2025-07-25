#pragma once

/* ������ Ŭ���̾�Ʈ ���� ��ũ�� ��Ȱ�� �����Ѵ�. */
/* ���� ���� �����ϴ� ������ �̱��� Ŭ�����̴�. */
/* ���� �����ڰ� Ŭ�󰳓T�ڿ��� �����ְ���� �Լ��� ... */
#include "Prototype_Manager.h"
#include "Shadow.h"

NS_BEGIN(Engine)


class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance);

private:
	CGameInstance();
	virtual ~CGameInstance() = default;

public:
	HRESULT Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDeviceOut, ID3D11DeviceContext** ppContextOut);
	void Update_Engine(_float fTimeDelta);
	void Clear(_uint iLevelIndex);
	HRESULT Begin_Draw();
	HRESULT Draw();
	HRESULT End_Draw();

	_float Compute_Random_Normal();
	_float Compute_Random(_float fMin, _float fMax);

#pragma region LEVEL_MANAGER
public:
	HRESULT Change_Level(_uint iLevelIndex, class CLevel* pNewLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, class CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE ePrototypeType, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);
	const map<const _wstring, class CBase*>* Get_Prototypes();
	class CBase* Find_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);
#pragma endregion

#pragma region OBJECT_MANAGER
	HRESULT Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg = nullptr);
	HRESULT Add_GameObjectReturn(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, CGameObject** ppOut, void* pArg);
	CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iIndex = 0);
	CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, _uint iPartID, const _wstring& strComponentTag, _uint iIndex = 0);

	class CGameObject* Get_Object(_uint iLevelIndex, const _wstring& strLayerTag, _uint iIndex);
	class CGameObject* Get_LastObject(_uint iLevelIndex, const _wstring& strLayerTag);
	list<class CGameObject*>& Get_ObjectList(_uint iLevelIndex, const _wstring& strLayerTag);

	//�ش� �̸��� ������ ���̾���� �±׸� ��� �����´�.
	vector<wstring> Find_LayerNamesContaining(_uint iLevelIndex, const wstring& SubString);

	//�ش� ������ �ִ� ���̾ ì�ܿ´�.
	//const map<const _wstring, class CLayer*>& Get_Layers(_uint iLevelIndex) const;
#pragma endregion

#pragma region RENDERER
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
#ifdef _DEBUG
	_bool Get_RenderCollider();
	HRESULT Add_DebugComponent(class CComponent* pDebugCom);
#endif
#pragma endregion

#pragma region TIMER_MANAGER
	_float Get_TimeDelta(const _wstring& strTimerTag);
	HRESULT Add_Timer(const _wstring& strTimerTag);
	void Update_Timer(const _wstring& strTimerTag);
#pragma endregion


#pragma region 	PIPELINE
	void Set_Transform(D3DTS eState, _fmatrix TransformMatrix);
	const _float4x4* Get_Transform_Float4x4(D3DTS eState) const;
	_matrix Get_Transform_Matrix(D3DTS eState) const;
	const _float4x4* Get_Transform_Float4x4_Inv(D3DTS eState) const;
	_matrix Get_Transform_Matrix_Inv(D3DTS eState) const;
	const _float4* Get_CamPosition() const;
#pragma endregion

#pragma region INPUT_DEVICE
	_byte Get_DIKeyState(_ubyte byKeyID);
	_byte Get_DIMouseState(DIM eMouse);
	_long Get_DIMouseMove(DIMM eMouseState);
	_bool Key_Down(_byte byKeyID);
	_bool Key_Up(_byte byKeyID);
	_bool Key_Pressing(_byte byKeyID);
	_bool Mouse_Down(DIM eMouseBtn);
	_bool Mouse_Up(DIM eMouseBtn);
	_bool Mouse_Pressing(DIM eMouseBtn);
	_long Get_DIMouseWheelDelta();
#pragma endregion

#pragma region LIGHT_MANAGER
	const LIGHT_DESC* Get_Light(_uint iIndex);
	HRESULT Add_Light(const LIGHT_DESC& LightDesc);
	HRESULT Render_Lights(class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	void Draw_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), _float fScale = 1.f);
#pragma endregion

#pragma region TARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);	
	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool isTargetClear = true, _bool isDepthClear = false);
	HRESULT End_MRT();
	HRESULT Bind_RT_ShaderResource(const _wstring& strTargetTag, class CShader* pShader, const _char* pContantName);
	HRESULT Copy_RT_Resource(const _wstring& strTargetTag, ID3D11Texture2D* pDest);

#ifdef _DEBUG
	HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_MRT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif

#pragma endregion

#pragma region PICKING
	_bool Picking(_float4* pOut);
#pragma endregion

#pragma region SHADOW
	HRESULT Ready_Light_For_Shadow(const CShadow::SHADOW_DESC& Desc);
	const _float4x4* Get_Light_ViewMatrix();
	const _float4x4* Get_Light_ProjMatrix();
#pragma endregion

#pragma region FRUSTUM	
	void Transform_Frustum_ToLocalSpace(_fmatrix WorldMatrix);
	_bool isIn_Frustum_WorldSpace(_fvector vWorldPos, _float fRange = 0.f);
	_bool isIn_Frustum_LocalSpace(_fvector vLocalPos, _float fRange);
#pragma endregion

#pragma region PHYSX_MANAGER
	PxTriangleMeshGeometry CookTriangleMesh(const PxVec3* vertices, PxU32 vertexCount, const PxU32* indices, PxU32 triangleCount, PxMeshScale geomScale);
	PxConvexMeshGeometry CookConvexMesh(const PxVec3* vertices, PxU32 vertexCount, PxMeshScale geomScale);
	PxCapsuleGeometry CookCapsuleGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float geomScale = 1.f);
	PxCapsuleGeometry CookCapsuleGeometry(_float fRadius, _float fCapsuleHeight);
	PxSphereGeometry  CookSphereGeometry(_float fRadius);
	PxSphereGeometry CookSphereGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float fScale = 1.f);
	PxBoxGeometry CookBoxGeometry(const PxVec3& halfExtents);
	PxBoxGeometry CookBoxGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float fScale = 1.f);
	PxScene* Get_Scene();
	PxPhysics* GetPhysics();
	PxMaterial* GetMaterial(const wstring& name);
#pragma endregion

#pragma region SOUND_DEVICE
	// Out�� nullptr ������ �̱ۻ���� ��ϵ�
	HRESULT LoadSound(const string& Path, _bool is3D = FALSE, _bool loop = FALSE, _bool stream = FALSE, unordered_map<string, class CSound_Core*>* _Out_ pOut = nullptr);
	// �� ���巡�� �ø��Ŵ� �и��� ���ߴ� ���� �ø��� ������ ��Ű�ÿ�
	class CSound_Core* Get_Single_Sound(const string& strKey);
	// ������ ������ ����(������ �䱸�ؼ� Ʈ������ �޾���)
	void Set_Listener_Position(class CTransform* pTransform, const _float3& vel);
	// ��ü ���� ����
	void Set_Master_Volume(_float volume);
#pragma endregion

private:
	class CGraphic_Device*		m_pGraphic_Device = { nullptr };
	class CInput_Device*		m_pInput_Device = { nullptr };
	class CLevel_Manager*		m_pLevel_Manager = { nullptr };
	class CPrototype_Manager*	m_pPrototype_Manager = { nullptr };
	class CObject_Manager*		m_pObject_Manager = { nullptr };
	class CRenderer*			m_pRenderer = { nullptr };
	class CTimer_Manager*		m_pTimer_Manager = { nullptr };
	class CPipeLine*			m_pPipeLine = { nullptr };
	class CLight_Manager*		m_pLight_Manager = { nullptr };
	class CFont_Manager*		m_pFont_Manager = { nullptr };
	class CTarget_Manager*		m_pTarget_Manager = { nullptr };
	class CPicking*				m_pPicking = { nullptr };
	class CShadow*				m_pShadow = { nullptr };
	class CFrustum*				m_pFrustum = { nullptr };
	class CPhysX_Manager*		m_pPhysX_Manager = { nullptr };
	class CSound_Device*		m_pSound_Device = { nullptr };
public:
	void Release_Engine();
	virtual void Free() override;
};

NS_END