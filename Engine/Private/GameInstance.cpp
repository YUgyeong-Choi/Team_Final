#include "GameInstance.h"

#include "Shadow.h"
#include "Frustum.h"
#include "Picking.h"
#include "Renderer.h"
#include "PipeLine.h"

#include "Input_Device.h"
#include "Font_Manager.h"
#include "Light_Manager.h"
#include "Level_Manager.h"
#include "Timer_Manager.h"
#include "Graphic_Device.h"
#include "Object_Manager.h"
#include "Target_Manager.h"
#include "Prototype_Manager.h"
#include "OctoTree_Manager.h"
#include "Area_Manager.h"

#include "PhysX_Manager.h"
#include "Sound_Device.h"
#include "Observer_Manager.h"

#include "ComputeShader.h"


IMPLEMENT_SINGLETON(CGameInstance);

static PxDefaultAllocator gAllocator;
static PxDefaultErrorCallback gErrorCallback;
namespace Engine
{
	_bool g_bSceneChanging = false;
}
//#ifdef _DEBUG
//void EnableConsole()
//{
//	AllocConsole();
//
//	FILE* fp;
//	freopen_s(&fp, "CONOUT$", "w", stdout);
//	freopen_s(&fp, "CONIN$", "r", stdin);
//	freopen_s(&fp, "CONOUT$", "w", stderr);
//
//	SetConsoleTitle(L"Debug Console");
//}
//#endif

CGameInstance::CGameInstance()
{
}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, _Out_ ID3D11Device** ppDeviceOut, ID3D11DeviceContext** ppContextOut)
{
//#ifdef _DEBUG
//	EnableConsole(); // 콘솔 창 띄우기
//#endif
	srand(static_cast<unsigned>(time(nullptr)));

	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.isWindowed, EngineDesc.iWinSizeX, EngineDesc.iWinSizeY, ppDeviceOut, ppContextOut);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pInput_Device = CInput_Device::Create(EngineDesc.hInstance, EngineDesc.hWnd);
	if (nullptr == m_pInput_Device)
		return E_FAIL;


	m_pTimer_Manager = CTimer_Manager::Create();
	if (nullptr == m_pTimer_Manager)
		return E_FAIL;

	m_pLevel_Manager = CLevel_Manager::Create();
	if (nullptr == m_pLevel_Manager)
		return E_FAIL;

	m_pPrototype_Manager = CPrototype_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pPrototype_Manager)
		return E_FAIL;

	m_pObject_Manager = CObject_Manager::Create(EngineDesc.iNumLevels);
	if (nullptr == m_pObject_Manager)
		return E_FAIL;

	m_pTarget_Manager = CTarget_Manager::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pTarget_Manager)
		return E_FAIL;

	m_pRenderer = CRenderer::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pRenderer)
		return E_FAIL;

	m_pPipeLine = CPipeLine::Create();
	if (nullptr == m_pPipeLine)
		return E_FAIL;

	m_pLight_Manager = CLight_Manager::Create();
	if (nullptr == m_pLight_Manager)
		return E_FAIL;

	m_pFont_Manager = CFont_Manager::Create(*ppDeviceOut, *ppContextOut);
	if(nullptr == m_pFont_Manager)
		return E_FAIL;

	m_pPicking = CPicking::Create(EngineDesc.hWnd, *ppDeviceOut, *ppContextOut);
	if (nullptr == m_pPicking)
		return E_FAIL;

	m_pShadow = CShadow::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pShadow)
		return E_FAIL;

	m_pFrustum = CFrustum::Create();
	if (nullptr == m_pFrustum)
		return E_FAIL;

	m_pPhysX_Manager = CPhysX_Manager::Create();
	if (nullptr == m_pPhysX_Manager)
		return E_FAIL;

	m_pSound_Device = CSound_Device::Create();
	if (nullptr == m_pSound_Device)
		return E_FAIL;

	m_pObserver_Manager = CObserver_Manager::Create();
	if (nullptr == m_pObserver_Manager)
		return E_FAIL;

	m_pOctoTree_Manager = COctoTree_Manager::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pOctoTree_Manager)
		return E_FAIL;

	m_pArea_Manager = CArea_Manager::Create(*ppDeviceOut, *ppContextOut);
	if (nullptr == m_pArea_Manager)
		return E_FAIL;


	return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{
	m_pPicking->Update();

	m_pSound_Device->Update();

	m_pInput_Device->Update();

	m_pObject_Manager->Priority_Update(fTimeDelta);
	m_pLevel_Manager->Priority_Update(fTimeDelta);

	m_pObject_Manager->Update(fTimeDelta);	
	m_pLevel_Manager->Update(fTimeDelta);

	m_pPipeLine->Update();
	m_pFrustum->Transform_ToWorldSpace();

	m_pObject_Manager->Late_Update(fTimeDelta);
	m_pLevel_Manager->Late_Update(fTimeDelta);

	m_pObject_Manager->Last_Update(fTimeDelta);

 	m_pPhysX_Manager->Simulate(fTimeDelta);


	//m_pPhysX_Manager->Sync();
#ifdef _DEBUG
	if (Key_Down(DIK_F5))
		m_pRenderer->Set_RenderTarget();

	if (Key_Down(DIK_F6))
		m_pRenderer->Set_RenderCollider();
	
	if (Key_Down(DIK_EQUALS))
		m_pRenderer->Change_DebugRT();

	if (Key_Down(DIK_M))
		m_pRenderer->Set_RenderMapCollider();
#endif

}

HRESULT CGameInstance::Begin_Draw()
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pGraphic_Device->Clear_BackBuffer_View(_float4(0.f, 0.0f, 1.f, 1.f));
	m_pGraphic_Device->Clear_DepthStencil_View();

	return S_OK;
}

HRESULT CGameInstance::Draw()
{
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pRenderer->Draw();

	m_pLevel_Manager->Render();
	m_pOctoTree_Manager->DebugDrawCells();
	m_pArea_Manager->DebugDrawCells();

	return S_OK;
}

HRESULT CGameInstance::End_Draw()
{
	m_pGraphic_Device->Present();

	return S_OK;
}

void CGameInstance::Clear(_uint iLevelIndex)
{
	/* 특정 레벨의 자원을 삭제한다. */
	
	/* 특정 레벨의 객체을 삭제한다. */
	m_pObject_Manager->Clear(iLevelIndex);

	/* 특정 레벨의 원형객을 삭제한다. */
	m_pPrototype_Manager->Clear(iLevelIndex);

 
}


_float CGameInstance::Compute_Random_Normal()
{
	return rand() / static_cast<_float>(RAND_MAX);	
}

_float CGameInstance::Compute_Random(_float fMin, _float fMax)
{
	return fMin + (fMax - fMin) * Compute_Random_Normal();	
}

#pragma region LEVEL_MANAGER

HRESULT CGameInstance::Change_Level(_uint iLevelIndex, CLevel* pNewLevel)
{
	return m_pLevel_Manager->Change_Level(iLevelIndex, pNewLevel);
}
CLevel* CGameInstance::Get_CurrentLevel() const
{
	return m_pLevel_Manager->Get_CurrentLevel();
}
#pragma endregion

#pragma region PROTOTYPE_MANAGER

HRESULT CGameInstance::Add_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Add_Prototype(iPrototypeLevelIndex, strPrototypeTag, pPrototype);
}

CBase* CGameInstance::Clone_Prototype(PROTOTYPE ePrototypeType, _uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, void* pArg)
{
	return m_pPrototype_Manager->Clone_Prototype(ePrototypeType, iPrototypeLevelIndex, strPrototypeTag, pArg);
}
const map<const _wstring, class CBase*>* CGameInstance::Get_Prototypes()
{
	return m_pPrototype_Manager->Get_Prototypes();
}

class CBase* CGameInstance::Find_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag)
{
	return m_pPrototype_Manager->Find_Prototype(iLevelIndex, strPrototypeTag);
}
HRESULT CGameInstance::Delete_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag)
{
	return m_pPrototype_Manager->Delete_Prototype(iPrototypeLevelIndex, strPrototypeTag);
}
HRESULT CGameInstance::Replace_Prototype(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype)
{
	return m_pPrototype_Manager->Replace_Prototype(iPrototypeLevelIndex, strPrototypeTag, pPrototype);
}
#pragma endregion

#pragma region OBJECT_MANAGER
HRESULT CGameInstance::Add_GameObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, void* pArg)
{
	return m_pObject_Manager->Add_GameObject(iPrototypeLevelIndex, strPrototypeTag, iLevelIndex, strLayerTag, pArg);
}
HRESULT CGameInstance::Add_GameObjectReturn(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, _uint iLevelIndex, const _wstring& strLayerTag, CGameObject** ppOut, void* pArg)
{
	return m_pObject_Manager->Add_GameObjectReturn(iPrototypeLevelIndex, strPrototypeTag, iLevelIndex, strLayerTag, ppOut, pArg);
}

CComponent* CGameInstance::Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, const _wstring& strComponentTag, _uint iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, strComponentTag, iIndex);
	
}

CComponent* CGameInstance::Get_Component(_uint iLevelIndex, const _wstring& strLayerTag, _uint iPartID, const _wstring& strComponentTag, _uint iIndex)
{
	return m_pObject_Manager->Get_Component(iLevelIndex, strLayerTag, iPartID, strComponentTag, iIndex);
}

CGameObject* CGameInstance::Get_Object(_uint iLevelIndex, const _wstring& strLayerTag, _uint iIndex)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;

	return m_pObject_Manager->Find_Object(iLevelIndex, strLayerTag, iIndex);
}

CGameObject* CGameInstance::Get_LastObject(_uint iLevelIndex, const _wstring& strLayerTag)
{
	if (nullptr == m_pObject_Manager)
		return nullptr;
	return m_pObject_Manager->Find_LastObject(iLevelIndex, strLayerTag);
}

list<class CGameObject*>& CGameInstance::Get_ObjectList(_uint iLevelIndex, const _wstring& strLayerTag)
{
	static list<class CGameObject*> _nullList = {};
	if (nullptr == m_pObject_Manager)
		return _nullList;

	return m_pObject_Manager->Get_ObjectList(iLevelIndex, strLayerTag);
}

vector<wstring> CGameInstance::Find_LayerNamesContaining(_uint iLevelIndex, const wstring& SubString)
{
	return m_pObject_Manager->Find_LayerNamesContaining(iLevelIndex, SubString);
}

const map<const _wstring, class CLayer*>& CGameInstance::Get_Layers(_uint iLevelIndex) const
{
	return m_pObject_Manager->Get_Layers(iLevelIndex);
}

#pragma endregion

#pragma region RENDERER

HRESULT CGameInstance::Add_RenderGroup(RENDERGROUP eRenderGroup, CGameObject* pRenderObject)
{
	return m_pRenderer->Add_RenderGroup(eRenderGroup, pRenderObject);
}

void CGameInstance::ClearRenderObjects()
{
	return m_pRenderer->ClearRenderObjects();
}


#ifdef _DEBUG
HRESULT CGameInstance::Add_DebugComponent(CComponent* pDebugCom)
{
	return m_pRenderer->Add_DebugComponent(pDebugCom);
}

_bool CGameInstance::Get_RenderCollider()
{
	return m_pRenderer->Get_RenderCollider();
}

_bool CGameInstance::Get_RenderMapCollider()
{
	return m_pRenderer->Get_RenderMapCollider();
}

#endif

#pragma endregion

#pragma region TIMER_MANAGER

_float CGameInstance::Get_TimeDelta(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(strTimerTag);
}

_float CGameInstance::Get_ScaledTimeDelta(const wstring& timerTag)
{
	return m_pTimer_Manager->Get_TimeDelta(timerTag) * m_fTimeScale;
}

HRESULT CGameInstance::Add_Timer(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Add_Timer(strTimerTag);
}

void CGameInstance::Update_Timer(const _wstring& strTimerTag)
{
	return m_pTimer_Manager->Update(strTimerTag);
}
#pragma endregion

#pragma region PIPELINE

void CGameInstance::Set_Transform(D3DTS eState, _fmatrix TransformMatrix)
{
	m_pPipeLine->Set_Transform(eState, TransformMatrix);
}

const _float4x4* CGameInstance::Get_Transform_Float4x4(D3DTS eState) const
{
	return m_pPipeLine->Get_Transform_Float4x4(eState);
}

_matrix CGameInstance::Get_Transform_Matrix(D3DTS eState) const
{
	return m_pPipeLine->Get_Transform_Matrix(eState);
}

const _float4x4* CGameInstance::Get_Transform_Float4x4_Inv(D3DTS eState) const
{
	return m_pPipeLine->Get_Transform_Float4x4_Inv(eState);
}


_matrix CGameInstance::Get_Transform_Matrix_Inv(D3DTS eState) const
{
	return m_pPipeLine->Get_Transform_Matrix_Inv(eState);
}

const _float4* CGameInstance::Get_CamPosition() const
{
	return m_pPipeLine->Get_CamPosition();
}
#pragma endregion

#pragma region INPUT_DEVICE

_byte CGameInstance::Get_DIKeyState(_ubyte byKeyID)
{
	return m_pInput_Device->Get_DIKeyState(byKeyID);
}

_byte CGameInstance::Get_DIMouseState(DIM eMouse)
{
	return m_pInput_Device->Get_DIMouseState(eMouse);
}

_long CGameInstance::Get_DIMouseMove(DIMM eMouseState)
{
	return m_pInput_Device->Get_DIMouseMove(eMouseState);
}

_bool CGameInstance::Key_Down(_byte byKeyID)
{
 	return m_pInput_Device->Key_Down(byKeyID);
}

_bool CGameInstance::Key_Up(_byte byKeyID)
{
	return m_pInput_Device->Key_Up(byKeyID);
}

_bool CGameInstance::Key_Pressing(_byte byKeyID)
{
	return m_pInput_Device->Key_Pressing(byKeyID);
}

_bool CGameInstance::Mouse_Down(DIM eMouseBtn)
{
	if (!m_pInput_Device)
	{
		OutputDebugStringA("m_pInput_Device is null!\n");
		return false;
	}
	return m_pInput_Device->Mouse_Down(eMouseBtn);
}

_bool CGameInstance::Mouse_Up(DIM eMouseBtn)
{
	return m_pInput_Device->Mouse_Up(eMouseBtn);
}

_bool CGameInstance::Mouse_Pressing(DIM eMouseBtn)
{
	return m_pInput_Device->Mouse_Pressing(eMouseBtn);
}

_long CGameInstance::Get_DIMouseWheelDelta()
{
	return m_pInput_Device->Get_DIMouseWheelDelta();
}
#pragma endregion

#pragma region LIGHT_MANAGER

const LIGHT_DESC* CGameInstance::Get_Light(_uint iIndex)
{
	return m_pLight_Manager->Get_Light(iIndex);
}

HRESULT CGameInstance::Add_Light(const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_Light(LightDesc);
}

HRESULT CGameInstance::Render_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pLight_Manager->Render_Lights(pShader, pVIBuffer);
}

HRESULT CGameInstance::Render_PBR_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer, _uint Level)
{
	return m_pLight_Manager->Render_PBR_Lights(pShader, pVIBuffer, Level);
}
HRESULT CGameInstance::Render_Volumetric_Lights(CShader* pShader, CVIBuffer_Rect* pVIBuffer, _uint Level)
{
	return m_pLight_Manager->Render_Volumetric_Lights(pShader, pVIBuffer, Level);
}
HRESULT CGameInstance::Add_LevelLightData(_uint iLevelIndex, const LIGHT_DESC& LightDesc)
{
	return m_pLight_Manager->Add_LevelLightData(iLevelIndex, LightDesc);
}

HRESULT CGameInstance::Add_LevelLightDataReturn(_uint iLevelIndex, const LIGHT_DESC& LightDesc, CLight** ppOut)
{
	return m_pLight_Manager->Add_LevelLightDataReturn(iLevelIndex, LightDesc, ppOut);
}

HRESULT CGameInstance::Remove_NoLevelLight()
{
	return m_pLight_Manager->Remove_NoLevelLight();
}

HRESULT CGameInstance::Remove_Light(_uint iLevelIndex, CLight* pLight)
{
	return m_pLight_Manager->Remove_Light(iLevelIndex, pLight);
}

HRESULT CGameInstance::RemoveAll_Light(_uint iLevelIndex)
{
	return m_pLight_Manager->RemoveAll_Light(iLevelIndex);
}

_uint CGameInstance::Get_LightCount(_uint TYPE, _uint iLevel) const
{
	return m_pLight_Manager->Get_LightCount(TYPE, iLevel);
}

HRESULT CGameInstance::Add_Font(const _wstring& strFontTag, const _tchar* pFontFilePath)
{
	return m_pFont_Manager->Add_Font(strFontTag, pFontFilePath);
}

void CGameInstance::Draw_Font(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, _float fScale, _float fOffset )
{
	m_pFont_Manager->Draw(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale, fOffset);
}
void CGameInstance::Draw_Font_Centered(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, _float fScale, _float fOffset )
{
	m_pFont_Manager->Draw_Centered(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale, fOffset);
}
void CGameInstance::Draw_Font_Righted(const _wstring& strFontTag, const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fRotation, const _float2& vOrigin, _float fScale, _float fOffset)
{
	m_pFont_Manager->Draw_Righted(strFontTag, pText, vPosition, vColor, fRotation, vOrigin, fScale, fOffset);
}
_float2 CGameInstance::Calc_Draw_Range(const _wstring& strFontTag, const _tchar* pText)
{
	return m_pFont_Manager->Draw_Range(strFontTag, pText);
}
#pragma endregion

#pragma region TARGET_MANAGER
HRESULT CGameInstance::Add_RenderTarget(const _wstring& strTargetTag, _uint iWidth, _uint iHeight, DXGI_FORMAT ePixelFormat, const _float4& vClearColor)
{
	return m_pTarget_Manager->Add_RenderTarget(strTargetTag, iWidth, iHeight, ePixelFormat, vClearColor);
}

HRESULT CGameInstance::Add_MRT(const _wstring& strMRTTag, const _wstring& strTargetTag)
{
	return m_pTarget_Manager->Add_MRT(strMRTTag, strTargetTag);
}

HRESULT CGameInstance::Begin_MRT(const _wstring& strMRTTag, ID3D11DepthStencilView* pDSV, _bool isTargetClear, _bool isDepthClear)
{
	return m_pTarget_Manager->Begin_MRT(strMRTTag, pDSV, isTargetClear, isDepthClear);
}

HRESULT CGameInstance::End_MRT()
{
	return m_pTarget_Manager->End_MRT();
}

HRESULT CGameInstance::Bind_RT_ShaderResource(const _wstring& strTargetTag, CShader* pShader, const _char* pContantName)
{
	return m_pTarget_Manager->Bind_ShaderResource(strTargetTag, pShader, pContantName);
}

HRESULT CGameInstance::Copy_RT_Resource(const _wstring& strTargetTag, ID3D11Texture2D* pDest)
{
	return m_pTarget_Manager->Copy_Resource(strTargetTag, pDest);
}

//렌더 타겟을 제거한다.
HRESULT CGameInstance::Delete_RenderTarget(const _wstring& strTargetTag)
{
	return m_pTarget_Manager->Delete_RenderTarget(strTargetTag);
}
//멀티 렌더타겟을 제거한다.
HRESULT CGameInstance::Delete_MRT(const _wstring& strMRTTag)
{
	return m_pTarget_Manager->Delete_MRT(strMRTTag);
}

class CRenderTarget* CGameInstance::Find_RenderTarget(const _wstring& strTargetTag)
{
	return m_pTarget_Manager->Find_RenderTarget(strTargetTag);
}


#ifdef _DEBUG

HRESULT CGameInstance::Ready_RT_Debug(const _wstring& strTargetTag, _float fX, _float fY, _float fSizeX, _float fSizeY)
{
	return m_pTarget_Manager->Ready_Debug(strTargetTag, fX, fY, fSizeX, fSizeY);
}

HRESULT CGameInstance::Render_MRT_Debug(const _wstring& strMRTTag, CShader* pShader, CVIBuffer_Rect* pVIBuffer)
{
	return m_pTarget_Manager->Render_Debug(strMRTTag, pShader, pVIBuffer);
}
#endif
#pragma endregion

#pragma region PICKING
_bool CGameInstance::Picking(_float4* pOut)
{
	return m_pPicking->Picking(pOut);
}

_bool CGameInstance::PickByClick(_int* pOut)
{
	return m_pPicking->PickByClick(pOut);
}

_bool CGameInstance::PickInRect(const _float2& vStart, const _float2& vEnd, set<_int>* pOut)
{
	return m_pPicking->PickInRect(vStart, vEnd, pOut);
}

_bool CGameInstance::Picking_ToolMesh(_int* pOut)
{
	return m_pPicking->PickingToolMesh(pOut);
}
#pragma endregion

#pragma region SHADOW
HRESULT CGameInstance::Ready_Light_For_Shadow(const CShadow::SHADOW_DESC& Desc, SHADOW eShadow)
{
	return m_pShadow->Ready_Light_For_Shadow(Desc, eShadow);
}

const _float4x4* CGameInstance::Get_Light_ViewMatrix(SHADOW eShadow)
{
	return m_pShadow->Get_LightViewMatrix(eShadow);
}
const _float4x4* CGameInstance::Get_Light_ProjMatrix(SHADOW eShadow)
{
	return m_pShadow->Get_LightProjMatrix(eShadow);
}
#pragma endregion

#pragma region FRUSTUM

void CGameInstance::Transform_Frustum_ToLocalSpace(_fmatrix WorldMatrix)
{
	m_pFrustum->Transform_ToLocalSpace(WorldMatrix);
}

_bool CGameInstance::isIn_Frustum_WorldSpace(_fvector vWorldPos, _float fRange)
{
	return m_pFrustum->isIn_WorldSpace(vWorldPos, fRange);
}

_bool CGameInstance::isIn_Frustum_LocalSpace(_fvector vLocalPos, _float fRange)
{
	return m_pFrustum->isIn_LocalSpace(vLocalPos, fRange);
}

_bool CGameInstance::isIn_PhysXAABB(CPhysXActor* pPhysXActor)
{
	return m_pFrustum->isIn_PhysXAABB(pPhysXActor);
}


PxTriangleMeshGeometry CGameInstance::CookTriangleMesh(const PxVec3* vertices, PxU32 vertexCount, const PxU32* indices, PxU32 triangleCount, PxMeshScale geomScale)
{
	return m_pPhysX_Manager->CookTriangleMesh(vertices, vertexCount, indices, triangleCount, geomScale);
}

PxConvexMeshGeometry CGameInstance::CookConvexMesh(const PxVec3* vertices, PxU32 vertexCount, PxMeshScale geomScale)
{
	return m_pPhysX_Manager->CookConvexMesh(vertices, vertexCount, geomScale);
}

PxBoxGeometry CGameInstance::CookBoxGeometry(const PxVec3* vertices, PxU32 vertexCount, _float fScale)
{
	return m_pPhysX_Manager->CookBoxGeometry(vertices, vertexCount, fScale);
}

PxBoxGeometry CGameInstance::CookBoxGeometry(const PxVec3& halfExtents)
{
	return m_pPhysX_Manager->CookBoxGeometry(halfExtents);
}

PxCapsuleGeometry CGameInstance::CookCapsuleGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float geomScale)
{
	return m_pPhysX_Manager->CookCapsuleGeometry(pVertices, vertexCount, geomScale);
}

PxCapsuleGeometry CGameInstance::CookCapsuleGeometry(_float fRadius, _float fCapsuleHeight)
{
	return m_pPhysX_Manager->CookCapsuleGeometry(fRadius, fCapsuleHeight);
}

PxSphereGeometry CGameInstance::CookSphereGeometry(_float fRadius)
{
	return m_pPhysX_Manager->CookSphereGeometry(fRadius);
}

PxSphereGeometry CGameInstance::CookSphereGeometry(const PxVec3* pVertices, PxU32 vertexCount, _float fScale)
{
	return m_pPhysX_Manager->CookSphereGeometry(pVertices, vertexCount, fScale);
}

PxScene* CGameInstance::Get_Scene()
{
	return m_pPhysX_Manager->Get_Scene();
}

PxControllerManager* CGameInstance::Get_ControllerManager()
{
	return m_pPhysX_Manager->Get_ControllerManager();
}

PxPhysics* CGameInstance::GetPhysics()
{
	return m_pPhysX_Manager->GetPhysics();
}

PxMaterial* CGameInstance::GetMaterial(const wstring& name)
{
	return m_pPhysX_Manager->GetMaterial(name);
}
#pragma endregion

#pragma region SOUND_DEVICE

FORCEINLINE
HRESULT CGameInstance::LoadSound(const string& Path, _bool is3D, _bool loop, _bool stream, unordered_map<string, class CSound_Core*>* _Out_ pOut)
{
	return m_pSound_Device->LoadSound(Path, is3D, loop, stream, pOut);
}

FORCEINLINE
CSound_Core* CGameInstance::Get_Single_Sound(const string& strKey)
{
	return m_pSound_Device->Get_Single_Sound(strKey);
}

void CGameInstance::Release_Single_Sound(const string& strKey)
{
	m_pSound_Device->Release_Single_Sound(strKey);
}

FORCEINLINE
void CGameInstance::Set_Listener_Position(CTransform* pTransform, const _float3& vel)
{
	m_pSound_Device->Set_Listener_Position(pTransform, vel);
}

FORCEINLINE
void CGameInstance::Set_Master_Volume(_float volume)
{
	m_pSound_Device->Set_Master_Volume(volume);
}
#pragma endregion

#pragma region OBSERVER

HRESULT CGameInstance::Add_Observer(const _wstring strTag, CObserver* pObserver)
{
	return m_pObserver_Manager->Add_Observer(strTag, pObserver);
}

HRESULT CGameInstance::Remove_Observer(const _wstring strTag)
{
	return m_pObserver_Manager->Remove_Observer(strTag);
}

void CGameInstance::Notify(const _wstring& strTag, const _wstring& eventType, void* pData)
{
	m_pObserver_Manager->Notify(strTag, eventType, pData);
}

void CGameInstance::Notify_Pull(const _wstring& strTag, const _wstring& eventType, void* pData)
{
	m_pObserver_Manager->Notify_Pull(strTag, eventType, pData);
}

void CGameInstance::Notify_Push(const _wstring& strTag, const _wstring& eventType, void* pData)
{
	m_pObserver_Manager->Notify_Push(strTag, eventType, pData);
}

CObserver* CGameInstance::Find_Observer(const _wstring& strTag)
{
	return m_pObserver_Manager->Find_Observer(strTag);
}

void CGameInstance::Register_PullCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback)
{
	m_pObserver_Manager->Register_PullCallback(strTag, callback);
}

void CGameInstance::Register_PushCallback(const _wstring& strTag, function<void(const _wstring& eventType, void* data)> callback)
{
	m_pObserver_Manager->Register_PushCallback(strTag, callback);
}

void CGameInstance::Reset(const _wstring& strTag)
{
	m_pObserver_Manager->Reset(strTag);
}

void CGameInstance::Reset_All()
{
	m_pObserver_Manager->Reset_All();
}
#pragma endregion

#pragma region OCTOTREE_MANAGER

HRESULT CGameInstance::Ready_OctoTree(const vector<AABBBOX>& staticBounds, const map<Handle, _uint>& handleToIndex)
{
	return m_pOctoTree_Manager->Ready_OctoTree(staticBounds, handleToIndex);
}
void CGameInstance::InitIndexToHandle(const map<Handle, _uint>& handleToIndex, size_t count)
{
	m_pOctoTree_Manager->InitIndexToHandle(handleToIndex, count);
}
void CGameInstance::BeginQueryFrame(const XMMATRIX& view, const XMMATRIX& proj)
{
	m_pOctoTree_Manager->BeginQueryFrame(view, proj);
}
HRESULT CGameInstance::SetObjectType(const vector<OCTOTREEOBJECTTYPE>& vTypes)
{
	return m_pOctoTree_Manager->SetObjectType(vTypes);
}
const vector<OCTOTREEOBJECTTYPE>& CGameInstance::GetObjectType()
{
	return m_pOctoTree_Manager->GetObjectType();
}
vector<class CGameObject*> CGameInstance::GetIndexToObj() const
{
	return m_pOctoTree_Manager->GetIndexToObj();
}
void CGameInstance::PushBackIndexToObj(CGameObject* vec)
{
	m_pOctoTree_Manager->PushBackIndexToObj(vec);
}
vector<_uint> CGameInstance::GetCulledStaticObjects() const
{
	return m_pOctoTree_Manager->GetCulledStaticObjects();
}
Handle CGameInstance::StaticIndexToHandle(_uint idx) const
{
	return m_pOctoTree_Manager->StaticIndexToHandle(idx);
}
void CGameInstance::ToggleDebugOctoTree()
{
	m_pOctoTree_Manager->ToggleDebugCells();
}
void CGameInstance::ClearIndexToObj()
{
	m_pOctoTree_Manager->ClearIndexToObj();
}
void CGameInstance::QueryVisible()
{
	m_pOctoTree_Manager->QueryVisible();
}
void CGameInstance::PushOctoTreeObjects(class CGameObject* pObject)
{
	m_pOctoTree_Manager->PushOctoTreeObjects(pObject);
}
vector<class CGameObject*> CGameInstance::GetOctoTreeObjects() const
{
	return m_pOctoTree_Manager->GetOctoTreeObjects();	
}
void CGameInstance::ClaerOctoTreeObjects()
{
	m_pOctoTree_Manager->ClaerOctoTreeObjects();
}
_bool CGameInstance::AddArea_AABB(_int iAreaId, const _float3& vMin, const _float3& vMax, const vector<_uint>& vecAdjacentIds, AREA::EAreaType eType, _int iPriority)
{
	return m_pArea_Manager->AddArea_AABB(iAreaId,vMin,vMax,vecAdjacentIds, eType, iPriority);
}
void CGameInstance::GetActiveAreaBounds(vector<AABBBOX>& vecOutBounds, _float fPad) const
{
	m_pArea_Manager->GetActiveAreaBounds(vecOutBounds, fPad);
}
HRESULT CGameInstance::FinalizePartition()
{
	return m_pArea_Manager->FinalizePartition();
}
_int CGameInstance::FindAreaContainingPoint()
{
	return m_pArea_Manager->FindAreaContainingPoint();
}
HRESULT CGameInstance::Reset_Parm()
{
	return m_pArea_Manager->Reset_Parm();
}
void CGameInstance::SetPlayerPosition(const _vector& vPos)
{
	m_pArea_Manager->SetPlayerPosition(vPos); 
}
void CGameInstance::ToggleDebugArea()
{
	m_pArea_Manager->ToggleDebugCells();
}
#pragma endregion


void CGameInstance::Release_Engine()
{
	Safe_Release(m_pFrustum);

	Safe_Release(m_pShadow);

	Safe_Release(m_pPicking);

	Safe_Release(m_pFont_Manager);

	Safe_Release(m_pLight_Manager);

	Safe_Release(m_pPipeLine);

	Safe_Release(m_pTimer_Manager);

	Safe_Release(m_pRenderer);

	Safe_Release(m_pObject_Manager);

	Safe_Release(m_pPrototype_Manager);

	Safe_Release(m_pLevel_Manager);

	//오브젝트 매니저에서 타겟 없애야 되는게 있어서 후 순위로 옮겼음 (모델 미리보기 기능)
	Safe_Release(m_pTarget_Manager);

	Safe_Release(m_pInput_Device);

	Safe_Release(m_pGraphic_Device);

	m_pPhysX_Manager->Shutdown();
	Safe_Release(m_pPhysX_Manager);

	Safe_Release(m_pSound_Device);

	Safe_Release(m_pObserver_Manager);

	Safe_Release(m_pOctoTree_Manager);

	Safe_Release(m_pArea_Manager);

	CComputeShader::ReleaseCache(); // 캐싱해둔 컴퓨트 셰이더들 해제


	Destroy_Instance();
}

void CGameInstance::Free()
{
	__super::Free();
	
}
