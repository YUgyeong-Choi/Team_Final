#pragma once

/* 엔진과 클라이언트 간의 링크의 역활을 수행한다. */
/* 엔진 내에 존재하는 유일한 싱글톤 클래스이다. */
/* 엔진 개발자가 클라개밫자에게 보여주고싶은 함수를 ... */
#include "Prototype_Manager.h"
#include "Shadow.h"
NS_BEGIN(Engine)


class ENGINE_DLL CGameInstance final : public CBase
{
	DECLARE_SINGLETON(CGameInstance)

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

	void Set_IsChangeLevel(_bool _b) { g_bSceneChanging = _b; }
	void Set_GameTimeScale(_float ftimeScale) { m_fTimeScale = ftimeScale; }

#pragma region LEVEL_MANAGER
public:
	HRESULT Change_Level(_uint iLevelIndex, class CLevel* pNewLevel);
#pragma endregion

#pragma region PROTOTYPE_MANAGER
	HRESULT Add_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, class CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE ePrototypeType, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg = nullptr);
	const map<const _wstring, class CBase*>* Get_Prototypes();
	class CBase* Find_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);
	// 프로토타입 삭제
	HRESULT Delete_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag);
	// 프로토타입 이미 있었다면 삭제 후 재생성
	HRESULT Replace_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, class CBase* pPrototype);
#pragma endregion

#pragma region OBJECT_MANAGER
	HRESULT Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg = nullptr);
	HRESULT Add_GameObjectReturn(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, CGameObject** ppOut, void* pArg);
	CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iIndex = 0);
	CComponent* Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, _uint iPartID, const _wstring& strComponentTag, _uint iIndex = 0);

	class CGameObject* Get_Object(_uint iLevelIndex, const _wstring& strLayerTag, _uint iIndex);
	class CGameObject* Get_LastObject(_uint iLevelIndex, const _wstring& strLayerTag);
	list<class CGameObject*>& Get_ObjectList(_uint iLevelIndex, const _wstring& strLayerTag);

	//해당 이름을 포함한 레이어들의 태그를 모두 가져온다.
	vector<wstring> Find_LayerNamesContaining(_uint iLevelIndex, const wstring& SubString);

	//해당 레벨에 있는 레이어를 챙겨온다.
	const map<const _wstring, class CLayer*>& Get_Layers(_uint iLevelIndex) const;
#pragma endregion

#pragma region RENDERER
	HRESULT Add_RenderGroup(RENDERGROUP eRenderGroup, class CGameObject* pRenderObject);
	void ClearRenderObjects();
#ifdef _DEBUG
	_bool Get_RenderCollider();
	_bool Get_RenderMapCollider();
	HRESULT Add_DebugComponent(class CComponent* pDebugCom);
#endif
#pragma endregion

#pragma region TIMER_MANAGER
	_float Get_TimeDelta(const _wstring& strTimerTag);
	_float Get_ScaledTimeDelta(const wstring& timerTag);
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
	HRESULT Render_PBR_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer, _uint Level);
	HRESULT Render_Volumetric_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer, _uint Level);
	HRESULT Add_LevelLightData(_uint iLevelIndex, const LIGHT_DESC& LightDesc);
	HRESULT Add_LevelLightDataReturn(_uint iLevelIndex, const LIGHT_DESC& LightDesc, class CLight** ppOut);
	HRESULT Remove_NoLevelLight();
	HRESULT Remove_Light(_uint iLevelIndex, class CLight* pLight);
	HRESULT RemoveAll_Light(_uint iLevelIndex);
	_uint Get_LightCount(_uint TYPE, _uint iLevel) const;
#pragma endregion

#pragma region FONT_MANAGER
	HRESULT Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath);
	void Draw_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), _float fScale = 1.f, _float fOffset = 0.f);
	void Draw_Font_Centered(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), _float fScale = 1.f,  _float fOffset = 0.f);
	void Draw_Font_Righted(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f), _float fRotation = 0.f, const _float2& vOrigin = _float2(0.f, 0.f), _float fScale = 1.f, _float fOffset = 0.f);
	_float2 Calc_Draw_Range(const _wstring& strFontTag, const _tchar* pText);
#pragma endregion

#pragma region TARGET_MANAGER
	HRESULT Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor);
	HRESULT Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag);	
	HRESULT Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV = nullptr, _bool isTargetClear = true, _bool isDepthClear = false);
	HRESULT End_MRT();
	HRESULT Bind_RT_ShaderResource(const _wstring& strTargetTag, class CShader* pShader, const _char* pContantName);
	HRESULT Copy_RT_Resource(const _wstring& strTargetTag, ID3D11Texture2D* pDest);
	//렌더 타겟을 제거한다.
	HRESULT Delete_RenderTarget(const _wstring& strTargetTag);
	//멀티 렌더타겟을 제거한다.
	HRESULT Delete_MRT(const _wstring& strMRTTag);
	//모델 미리보기 그리기위해 렌더타겟 찾는거 퍼블릭으로 올림
	class CRenderTarget* Find_RenderTarget(const _wstring& strTargetTag);

#ifdef _DEBUG
	HRESULT Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY);
	HRESULT Render_MRT_Debug(const _wstring& strMRTTag, class CShader* pShader, class CVIBuffer_Rect* pVIBuffer);
#endif

#pragma endregion

#pragma region PICKING
	//피킹된 위치를 가져옴
	_bool Picking(_float4* pOut);

	//피킹한 오브젝트의 아이디를 가져옴
	_bool PickByClick(_int* pOut);

	//드래그한 만큼의 범위에서 오브젝트의 아이디를 가져옴
	_bool PickInRect(const _float2& vStart, const _float2& vEnd, set<_int>* pOut);


	_bool Picking_ToolMesh(_int* pOut);
#pragma endregion

#pragma region SHADOW
	HRESULT Ready_Light_For_Shadow(const CShadow::SHADOW_DESC& Desc, SHADOW eShadow);
	const _float4x4* Get_Light_ViewMatrix(SHADOW eShadow);
	const _float4x4* Get_Light_ProjMatrix(SHADOW eShadow);
#pragma endregion

#pragma region FRUSTUM	
	void Transform_Frustum_ToLocalSpace(_fmatrix WorldMatrix);
	_bool isIn_Frustum_WorldSpace(_fvector vWorldPos, _float fRange = 0.f);
	_bool isIn_Frustum_LocalSpace(_fvector vLocalPos, _float fRange);
	_bool isIn_PhysXAABB(class CPhysXActor* pPhysXActor);
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
	PxControllerManager* Get_ControllerManager();
	PxPhysics* GetPhysics();
	PxMaterial* GetMaterial(const wstring& name);
#pragma endregion

#pragma region SOUND_DEVICE
	// Out에 nullptr 넣으면 싱글사운드로 등록됨
	HRESULT LoadSound(const string& Path, _bool is3D = FALSE, _bool loop = FALSE, _bool stream = FALSE, unordered_map<string, class CSound_Core*>* _Out_ pOut = nullptr);
	// 난 에드래퍼 올릴거다 분명히 말했다 래퍼 올린다 릴리즈 시키시오
	class CSound_Core* Get_Single_Sound(const string& strKey);
	// 리스너 포지션 제어(업룩포 요구해서 트랜스폼 받았음)
	void Set_Listener_Position(class CTransform* pTransform, const _float3& vel);
	// 전체 볼륨 제어
	void Set_Master_Volume(_float volume);
#pragma endregion

#pragma region LEVEL_SET
	void SetCurrentLevelIndex(_uint iLevelIndex) { m_iCurrentLevelIndex = iLevelIndex; }
	_uint GetCurrentLevelIndex() const { return m_iCurrentLevelIndex; }
#pragma endregion

#pragma region OBSERVER_MANAGER
	HRESULT Add_Observer(const _wstring strTag, class CObserver* pObserver);
	HRESULT Remove_Observer(const _wstring strTag);
	// pull push 둘다
	void Notify(const _wstring& strTag, const _wstring& eventType, void* pData = nullptr);
	// 값 올리는거만 필요하면
	void Notify_Pull(const _wstring& strTag, const _wstring& eventType, void* pData);
	// 값 받아오는거만 필요하면
	void Notify_Push(const _wstring& strTag, const _wstring& eventType, void* pData);

	class CObserver* Find_Observer(const _wstring& strTag);
	void Register_PullCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback);
	void Register_PushCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback);
	void Reset(const _wstring& strTag);
	void Reset_All();
#pragma endregion


#pragma region OCTOTREE_MANAGER
	HRESULT Ready_OctoTree(const vector<AABBBOX>& staticBounds, const map<Handle, _uint>& handleToIndex);
	void InitIndexToHandle(const map<Handle, _uint>& handleToIndex, size_t count);
	void BeginQueryFrame(const XMMATRIX& view, const XMMATRIX& proj);
	HRESULT SetObjectType(const vector<OCTOTREEOBJECTTYPE>& vTypes);
	const vector<OCTOTREEOBJECTTYPE>& GetObjectType();
	vector<class CGameObject*> GetIndexToObj() const;
	void PushBackIndexToObj(class CGameObject* vec);
	vector<_uint> GetCulledStaticObjects() const;
	Handle StaticIndexToHandle(_uint idx) const;
	void ToggleDebugOctoTree();
	void ClearIndexToObj();
	void QueryVisible();
#pragma endregion

#pragma region AREA_MANAGER
	_bool AddArea_AABB(_int iAreaId, const _float3& vMin, const _float3& vMax, const vector<_uint>& vecAdjacentIds, AREA::EAreaType eType, _int iPriority);
	void GetActiveAreaBounds(vector<AABBBOX>& vecOutBounds, _float fPad = 0.f) const;
	HRESULT FinalizePartition();
	_int FindAreaContainingPoint();
	HRESULT Reset_Parm();
	void SetPlayerPosition(const _vector& vPos);
	void ToggleDebugArea();
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
	class CObserver_Manager*	m_pObserver_Manager = { nullptr };
	class COctoTree_Manager*	m_pOctoTree_Manager = { nullptr };
	class CArea_Manager*		m_pArea_Manager = { nullptr };

private:
	_uint					m_iCurrentLevelIndex = 0;
	float m_fTimeScale = 1.f; // 업데이트 속도
public:
	void Release_Engine();
	virtual void Free() override;
};

NS_END