#pragma once

#pragma region 프로토타입 넣는 매크로들
#pragma region SR매크로

#define ADD_TEXTURE(Name,Path,Cnt)																					\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_Component_Texture_"))+L###Name ,	\
CTexture::Create(m_pGraphic_Device, TEXT(Path), Cnt))))																\
return E_FAIL

#define ADD_TEXTURE_EX(Name,Path,Cnt,...)																			\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_Component_Texture_"))+L###Name ,	\
CTexture::Create(m_pGraphic_Device, TEXT(Path), Cnt, __VA_ARGS__))))												\
return E_FAIL

#define ADD_MODEL(Name)																								\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_Component_VIBuffer_"))+L###Name,	\
CVIBuffer_##Name::Create(m_pGraphic_Device))))																		\
return E_FAIL

#define ADD_MODEL_EX(Name,...)																						\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_Component_VIBuffer_"))+L###Name,	\
CVIBuffer_##Name::Create(m_pGraphic_Device, __VA_ARGS__ ))))														\
return E_FAIL

#define ADD_PRTOBJ(Name)																							\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_GameObject_"))+L###Name ,		\
C##Name::Create(m_pGraphic_Device))))																				\
return E_FAIL

#define ADD_PRTOBJ_EX(Name,...)																						\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_GameObject_"))+L###Name ,		\
C##Name::Create(m_pGraphic_Device, __VA_ARGS__ ))))																	\
return E_FAIL

#define ADD_PRTCOM(Name)																							\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_Component_")) +L###Name,			\
C##Name::Create(m_pGraphic_Device))))																				\
return E_FAIL

#define ADD_PRTCOM_EX(Name,...)																						\
if (FAILED(m_pGameInstance->Add_Prototype(m_eNextLevelID, _wstring(TEXT("Prototype_Component_")) +L###Name,			\
C##Name::Create(m_pGraphic_Device, __VA_ARGS__ ))))																	\
return E_FAIL	

#pragma endregion

#define ADD_MODEL_NA(Name, eLevel, fScale, Path)																			\
{_matrix PreTransMatrix = XMMatrixScaling(fScale, fScale, fScale) * XMMatrixRotationY(XMConvertToRadians(180.f));			\
if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(eLevel), _wstring(TEXT("Prototype_Component_Model_")) +L###Name,		\
CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, Path, PreTransMatrix))))												\
return E_FAIL;}

#pragma endregion

#pragma region 유틸리티

#define GET_PLAYER m_pGameInstance->Find_Object(LEVEL_STATIC,TEXT("Layer_Pawn"))
#define GET_PLAYER_TRANSFORM static_cast<CTransform*>(GET_PLAYER->Find_Component(TEXT("Com_Transform")))

#define GET_CAMERA											\
_float4x4 matCamWorld;										\
m_pGraphic_Device->GetTransform(D3DTS_VIEW, &matCamWorld);	\
matCamWorld.MakeInverseMat(matCamWorld);

#define Get_CamaraPos GET_CAMERA											\
_float3 vCameraPos = { matCamWorld._41, matCamWorld._42, matCamWorld._43 };	

#define _VUp		_float3(0.f,1.f,0.f)

#define FWINCX static_cast<_float>(g_iWinSizeX)
#define FWINCY static_cast<_float>(g_iWinSizeY)

#define KEY_DOWN		m_pGameInstance->Key_Down
#define	KEY_PRESSING	m_pGameInstance->Key_Pressing
#define KEY_UP			m_pGameInstance->Key_Up

#define MOUSE_DOWN		m_pGameInstance->Mouse_Down
#define MOUSE_PRESSING	m_pGameInstance->Mouse_Pressing
#define MOUSE_UP		m_pGameInstance->Mouse_Up

#define CLAMP(x, min, max) (x < min ? min : (x > max ? max : x))

#define IFILEDIALOG ImGuiFileDialog::Instance()

#pragma endregion
