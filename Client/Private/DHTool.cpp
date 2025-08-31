#include "DHTool.h"

#include "GameInstance.h"
#include "DH_ToolMesh.h"
#include "Light.h"

const char* CDHTool::szLightName[] = { "Directional", "Spot","Point"};
const char* CDHTool::szLevelName[] = {"KRAT_CENTERAL_STATION","KRAT_HOTEL"};

CDHTool::CDHTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CDHTool::CDHTool(const CDHTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CDHTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CDHTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	/* [ 툴을 위한 셰이더 로딩 ] */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxPBRMesh")),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), _wstring(TEXT("Prototype_Component_Shader_VtxMesh_Instance")),
		TEXT("Com_ShaderInstance"), reinterpret_cast<CComponent**>(&m_pShaderComInstance))))
		return E_FAIL;

	//Add_Light(LIGHT_TYPE::POINT, LEVEL_TYPE::KRAT_CENTERAL_STATION);
	return S_OK;
}

void CDHTool::Priority_Update(_float fTimeDelta)
{

}

void CDHTool::Update(_float fTimeDelta)
{
	// QWER 키 조작 (Maya 스타일)
	if (KEY_DOWN(DIK_Q))
		m_pSelectedObject = nullptr;
	else if (KEY_DOWN(DIK_W))
		m_eGizmoOp = ImGuizmo::TRANSLATE;
	else if (KEY_DOWN(DIK_E))
		m_eGizmoOp = ImGuizmo::ROTATE;
	else if (KEY_DOWN(DIK_R))
		m_eGizmoOp = ImGuizmo::SCALE;

	// 선택 (오브젝트 클릭 시)
	if (MOUSE_DOWN(DIM::LBUTTON))
		Picking();

	if (KEY_DOWN(DIK_G))
		TogglePickMode();

	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
		DeleteSelectedObject(); 

	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_D))
		Duplicate_Selected_Object();
}

void CDHTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CDHTool::Render()
{
	if (FAILED(Render_ShaderTool()))
		return E_FAIL;

	if (FAILED(Render_LightTool()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDHTool::Render_ShaderTool()
{
    SetNextWindowSize(ImVec2(400, 800), ImGuiCond_Once);
    _bool open = true;
    if (ImGui::Begin("DH PBR Tools", &open, ImGuiWindowFlags_NoCollapse))
    {
		/* [ 각 파라미터에 대한 Max 값 ] */
        static _float fMaxDiffuse = 3.f;
        static _float fMaxNormal = 1.f;
        static _float fMaxAO = 2.f;
        static _float fMaxAOPower = 3.f;
        static _float fMaxRoughness = 5.f;
        static _float fMaxMetallic = 1.f;
        static _float fMaxReflection = 10.f;
        static _float fMaxSpecular = 10.f;
		ImGui::Text("PBR Intensity Controls");
        ImGui::Separator();

        static _float fDiffuseIntensity = 1.0f;
        static _float fNormalIntensity = 1.0f;
        static _float fAOIntensity = 1.0f;
        static _float fAOPower = 1.0f;
        static _float fRoughnessIntensity = 1.0f;
        static _float fMetallicIntensity = 1.0f;
        static _float fReflectionIntensity = 1.0f;
        static _float fSpecularIntensity = 1.0f;
        static _float4 vDiffuseTint = _float4{ 1.f, 1.f, 1.f, 1.f };

		_bool bUpdated = false;
        bUpdated |= ImGui::SliderFloat("Diffuse", &fDiffuseIntensity, 0.0f, fMaxDiffuse, "%.2f");
        bUpdated |= ImGui::SliderFloat("Normal", &fNormalIntensity, 0.0f, fMaxNormal, "%.2f");
        bUpdated |= ImGui::SliderFloat("AO", &fAOIntensity, 0.0f, fMaxAO, "%.2f");
        bUpdated |= ImGui::SliderFloat("AOPower", &fAOPower, 0.0f, fMaxAOPower, "%.2f");
        bUpdated |= ImGui::SliderFloat("Roughness", &fRoughnessIntensity, 0.0f, fMaxRoughness, "%.2f");
        bUpdated |= ImGui::SliderFloat("Metallic", &fMetallicIntensity, 0.0f, fMaxMetallic, "%.2f");
        bUpdated |= ImGui::SliderFloat("Reflection", &fReflectionIntensity, 0.0f, fMaxReflection, "%.2f");
        bUpdated |= ImGui::SliderFloat("Specular", &fSpecularIntensity, 0.0f, fMaxSpecular, "%.2f");

		if (ImGui::Button("Reset Parameters"))
		{
			bUpdated = true;
			fDiffuseIntensity = 1.f;
			fNormalIntensity = 1.f;
			fAOIntensity = 1.f;
			fAOPower = 1.f;
			fRoughnessIntensity = 1.f;
			fMetallicIntensity = 1.f;
			fReflectionIntensity = 1.f;
			fSpecularIntensity = 1.f;
			vDiffuseTint = _float4{1.f, 1.f, 1.f, 1.f};
		}

		ImVec4 vTintColor = ImVec4(vDiffuseTint.x, vDiffuseTint.y, vDiffuseTint.z, 1.f);

		if (m_bPickColor || ImGui::ColorPicker4("Tint Color", (float*)&vTintColor,
			ImGuiColorEditFlags_DisplayHSV |
			ImGuiColorEditFlags_PickerHueWheel))
		{
			m_bPickColor = false;
			bUpdated = true;
			vDiffuseTint = _float4(vTintColor.x, vTintColor.y, vTintColor.z, vTintColor.w);
		}

		ImGui::Separator();
		if (ImGui::Button("Save Parameters"))
		{
			Save_Shader(
				fDiffuseIntensity,
				fNormalIntensity,
				fAOIntensity,
				fAOPower,
				fRoughnessIntensity,
				fMetallicIntensity,
				fReflectionIntensity,
				fSpecularIntensity,
				vDiffuseTint
			);
		}
		ImGui::SameLine();
		if (ImGui::Button("Load Parameters"))
		{
			bUpdated = true;
			m_bPickColor = true;
			Load_Shader(
				fDiffuseIntensity,
				fNormalIntensity,
				fAOIntensity,
				fAOPower,
				fRoughnessIntensity,
				fMetallicIntensity,
				fReflectionIntensity,
				fSpecularIntensity,
				vDiffuseTint
			);
		}
		
        ImGui::Spacing();
		if (bUpdated)
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fDiffuseIntensity", &fDiffuseIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fDiffuseIntensity", &fDiffuseIntensity, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_vDiffuseTint", &vDiffuseTint, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_vDiffuseTint", &vDiffuseTint, sizeof(_float4))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fNormalIntensity", &fNormalIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fNormalIntensity", &fNormalIntensity, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fAOIntensity", &fAOIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fAOIntensity", &fAOIntensity, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fAOPower", &fAOPower, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fAOPower", &fAOPower, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fRoughnessIntensity", &fRoughnessIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fRoughnessIntensity", &fRoughnessIntensity, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fMetallicIntensity", &fMetallicIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fMetallicIntensity", &fMetallicIntensity, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fReflectionIntensity", &fReflectionIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fReflectionIntensity", &fReflectionIntensity, sizeof(_float))))
				return E_FAIL;

			if (FAILED(m_pShaderCom->Bind_RawValue("g_fSpecularIntensity", &fSpecularIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderComInstance->Bind_RawValue("g_fSpecularIntensity", &fSpecularIntensity, sizeof(_float))))
				return E_FAIL;
		}
    }
    ImGui::End();
    return S_OK;
}

HRESULT CDHTool::Render_LightTool()
{
	SetNextWindowSize(ImVec2(300, 800), ImGuiCond_Once);
	bool open = true;
	if (!ImGui::Begin("Light Tools", &open))
	{
		ImGui::End();
		return S_OK;
	}

	ImGui::Text("Mode");

	// 선택된 상태에 따라 손잡이 색상 지정
	ImVec4 moveColor = ImVec4(1.f, 0.2f, 0.2f, 1.f);   // 붉은색
	ImVec4 pickColor = ImVec4(0.2f, 0.6f, 1.f, 1.f);   // 파란색 등 자유 조정

	// 스타일 적용 (슬라이더 손잡이 색상 변경)
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, m_iLightMode == 0 ? pickColor : moveColor);
	// 슬라이더 폭 지정
	ImGui::PushItemWidth(265);
	// 슬라이더 본체
	ImGui::SliderInt("##LightModeSlider", &m_iLightMode, 0, 1,
		m_iLightMode == 0 ? "Pick" : "Move");

	// 스타일 원상복귀
	ImGui::PopItemWidth();
	ImGui::PopStyleColor();
	// 선택된 모드 출력 (예시)
	ImGui::Text("Current Mode: %s", m_iLightMode == 0 ? "Pick" : "Move");

	// ------------------------------------------------------------------//
	ImVec4 NoneColor = ImVec4(0.2f, 0.2f, 1.f, 1.f);
	ImVec4 VolumeColor = ImVec4(0.2f, 0.6f, 1.f, 1.f);

	// 스타일 적용 (슬라이더 손잡이 색상 변경)
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, m_iVolumetricMode == 0 ? NoneColor : VolumeColor);
	ImGui::PushItemWidth(265);
	if (ImGui::SliderInt("##VolumetricSlider", &m_iVolumetricMode, 0, 1,
		m_iVolumetricMode == 0 ? "VOLUMETRIC" : "NONE"))
	{
		if(m_pSelectedObject)
			m_pSelectedObject->SetbVolumetric(m_iVolumetricMode == 0);
	}
	else
	{
		if (m_pSelectedObject)
			m_iVolumetricMode = m_pSelectedObject->GetbVolumetric() ? 0 : 1;
		else
			m_iVolumetricMode = 1; // 기본값 설정
	}

	// 스타일 원상복귀
	ImGui::PopItemWidth();
	ImGui::PopStyleColor();
	ImGui::Text("Current Mode: %s", m_iVolumetricMode == 0 ? "VOLUMETRIC" : "NONE");

	// ComboBox로 조명 타입 선택
	ImGui::Text("Light Type");
	if (ImGui::BeginCombo("##LightTypeCombo", szLightName[m_iSelectedLightType]))
	{
		for (int i = 0; i < (int)LIGHT_TYPE::LIGHT_TYPE_END; ++i)
		{
			bool isSelected = (m_iSelectedLightType == i);
			if (ImGui::Selectable(szLightName[i], isSelected))
				m_iSelectedLightType = i;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Text("Level Type");
	if (ImGui::BeginCombo("##LevelTypeCombo", szLevelName[m_iSelectedLevelType]))
	{
		for (int i = 0; i < (int)LEVEL_TYPE::LEVEL_END; ++i)
		{
			bool isSelected = (m_iSelectedLevelType == i);
			if (ImGui::Selectable(szLevelName[i], isSelected))
				m_iSelectedLevelType = i;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// 선택된 조명 생성 버튼
	if (ImGui::Button("Create Light"))
	{
		Create_Light((LIGHT_TYPE)m_iSelectedLightType, (LEVEL_TYPE)m_iSelectedLevelType);
	}

	ImGui::Separator();

	// 현재 라이트 개수 표시
	int iDirectionalCount = m_pGameInstance->Get_LightCount(0, ENUM_CLASS(LEVEL::DH));
	int iSpotCount = m_pGameInstance->Get_LightCount(1, ENUM_CLASS(LEVEL::DH));
	int iPointCount = m_pGameInstance->Get_LightCount(2, ENUM_CLASS(LEVEL::DH));
	
	ImGui::Text("Light Count:");
	ImGui::BulletText("Point Light      : %d", iPointCount);
	ImGui::BulletText("Spot Light       : %d", iSpotCount);
	ImGui::BulletText("Directional Light: %d", iDirectionalCount);

	PickGuizmo();

	ImGui::Text("Light Parameters");

	// 세기(Intensity)
	if (m_pSelectedObject != nullptr)
	{
		// 1. 선택된 오브젝트의 Intensity 값 가져오기
		float fIntensity = m_pSelectedObject->GetIntensity();

		// 2. 슬라이더 UI로 표시 및 조정
		if (ImGui::SliderFloat("Intensity", &fIntensity, 0.0f, 10.0f, "%.2f"))
		{
			// 3. 사용자가 값 변경 시 다시 적용
			m_pSelectedObject->SetIntensity(fIntensity);
		}
	}
	else
	{
		float fIntensity = 1.f;
		ImGui::SliderFloat("Intensity", &fIntensity, 0.0f, 10.0f, "%.2f");
	}

	// 색상 (Color)
	if (m_pSelectedObject != nullptr)
	{
		// 1. XMFLOAT4 → ImVec4 변환
		_float4 dxColor = m_pSelectedObject->GetColor(); // XMFLOAT4 or _float4
		ImVec4 vColor = ImVec4(dxColor.x, dxColor.y, dxColor.z, dxColor.w);

		// 2. 색상 편집 UI
		if (ImGui::ColorEdit3("Color", (float*)&vColor)) // 알파 제외
		{
			// 3. ImVec4 → _float4로 다시 변환
			_float4 updatedColor(vColor.x, vColor.y, vColor.z, vColor.w);
			m_pSelectedObject->SetColor(updatedColor);
		}
	}
	else
	{
		static ImVec4 vColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImGui::ColorEdit3("Color", (float*)&vColor);
	}

	// 범위(Range)
	if (m_pSelectedObject != nullptr)
	{
		// 1. 선택된 오브젝트의 Range 값 가져오기
		float fRange = m_pSelectedObject->GetRange();

		// 2. Range 슬라이더 UI
		if (ImGui::SliderFloat("Range", &fRange, 0.01f, 50.0f, "%.1f"))
		{
			// 3. 값이 바뀌었을 때 다시 적용
			m_pSelectedObject->SetRange(fRange);
		}
	}
	else
	{
		// 선택된 오브젝트가 없을 경우 기본값만 표시 (비활성 상태)
		static float fRange = 10.0f;
		ImGui::SliderFloat("Range", &fRange, 0.01f, 50.0f, "%.1f");
	}

	// SPOT(Range)
	if (m_pSelectedObject != nullptr)
	{
		float fInnerCosAngle = m_pSelectedObject->GetfInnerCosAngle();
		if (ImGui::SliderFloat("InnerCosAngle", &fInnerCosAngle, 0.1f, 100.0f, "%.1f"))
			m_pSelectedObject->SetfInnerCosAngle(fInnerCosAngle);
	}
	else
	{
		static float fInnerCosAngle = 15.f;
		ImGui::SliderFloat("InnerCosAngle", &fInnerCosAngle, 0.1f, 100.0f, "%.1f");
	}

	// SPOT(Range)
	if (m_pSelectedObject != nullptr)
	{
		float fOuterCosAngle = m_pSelectedObject->GetfOuterCosAngle();
		if (ImGui::SliderFloat("fOuterCosAngle", &fOuterCosAngle, 0.1f, 100.0f, "%.1f"))
			m_pSelectedObject->SetfOuterCosAngle(fOuterCosAngle);
	}
	else
	{
		static float fOuterCosAngle = 25.f;
		ImGui::SliderFloat("fOuterCosAngle", &fOuterCosAngle, 0.1f, 100.0f, "%.1f");
	}

	// SPOT(Fallof)
	if (m_pSelectedObject != nullptr)
	{
		float fFalloff = m_pSelectedObject->GetfFalloff();
		if (ImGui::SliderFloat("fFalloff", &fFalloff, 0.1f, 5.0f, "%.1f"))
			m_pSelectedObject->SetfFalloff(fFalloff);
	}
	else
	{
		static float fFalloff = 1.f;
		ImGui::SliderFloat("fFalloff", &fFalloff, 0.1f, 5.0f, "%.1f");
	}

	// Fog
	if (m_pSelectedObject != nullptr)
	{
		float fFogDensity = m_pSelectedObject->GetfFogDensity();
		if (ImGui::SliderFloat("fFogDensity", &fFogDensity, 0.1f, 100.0f, "%.1f"))
			m_pSelectedObject->SetfFogDensity(fFogDensity);
	}
	else
	{
		static float fFogDensity = 1.f;
		ImGui::SliderFloat("fFogDensity", &fFogDensity, 0.1f, 20.0f, "%.1f");
	}
	// FogCutOff
	if (m_pSelectedObject != nullptr)
	{
		float fFogCutOff = m_pSelectedObject->GetfFogCutOff();
		if (ImGui::SliderFloat("fFogCutOff", &fFogCutOff, 1.f, 30.f, "%.1f"))
			m_pSelectedObject->SetfFogCutOff(fFogCutOff);
	}
	else
	{
		static float fFogCutOff = 15.f;
		ImGui::SliderFloat("fFogCutOff", &fFogCutOff, 1.f, 30.f, "%.1f");
	}


	ImGui::Separator();
	if (ImGui::Button("Save Light", ImVec2(120, 0)))
	{
		Save_Lights(static_cast<LEVEL_TYPE>(m_iSelectedLevelType));
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Light", ImVec2(120, 0)))
	{
		DeleteAllLights();
		Load_Lights(static_cast<LEVEL_TYPE>(m_iSelectedLevelType));
	}

	ImGui::Spacing();

	static float curvePoints[8] = { 0.0f, 0.2f, 0.4f, 0.6f, 0.6f, 0.8f, 0.9f, 1.0f };

	ImGui::Text("Editable Curve");
	ImGui::PlotLines("##Curve", curvePoints, IM_ARRAYSIZE(curvePoints), 0, nullptr, 0.0f, 1.0f, ImVec2(300, 100));

	for (int i = 0; i < IM_ARRAYSIZE(curvePoints); ++i)
	{
		char label[32];
		sprintf_s(label, "P%d", i);
		ImGui::PushItemWidth(260.0f);
		ImGui::SliderFloat(label, &curvePoints[i], 0.0f, 1.0f);
		ImGui::PopItemWidth();
	}

	ImGui::End();
	return S_OK;
}

HRESULT CDHTool::Create_Light(LIGHT_TYPE eType, LEVEL_TYPE eLType)
{
	Add_Light(eType, eLType);
	return S_OK;
}

HRESULT CDHTool::Add_Light(LIGHT_TYPE eType, LEVEL_TYPE eLType)
{
	CDH_ToolMesh::DHTOOL_DESC Desc{};
	if(eType == LIGHT_TYPE::POINT)
	{
		Desc.szMeshID = TEXT("PointLight");
		lstrcpy(Desc.szName, TEXT("PointLight"));
	}
	if (eType == LIGHT_TYPE::SPOT)
	{
		Desc.szMeshID = TEXT("SpotLight");
		lstrcpy(Desc.szName, TEXT("SpotLight"));
	}
	if (eType == LIGHT_TYPE::DIRECTIONAL)
	{
		Desc.szMeshID = TEXT("DirrectionalLight");
		lstrcpy(Desc.szName, TEXT("DirrectionalLight"));
	}

	if(eLType == LEVEL_TYPE::KRAT_CENTERAL_STATION)
		Desc.eLEVEL = LEVEL::KRAT_CENTERAL_STATION;
	if (eLType == LEVEL_TYPE::KRAT_HOTEL)
		Desc.eLEVEL = LEVEL::KRAT_HOTEL;


	Desc.fRotationPerSec = 0.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.m_vInitPos = _float3(0.f, 10.f, 10.f);
	Desc.iID = m_iID++;

	CGameObject* pGameObject = nullptr;
	if (FAILED(m_pGameInstance->Add_GameObjectReturn(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_GameObject_ToolMesh"),
		ENUM_CLASS(LEVEL::DH), L"Layer_ToolMesh", &pGameObject, &Desc)))
		return E_FAIL;

	m_vecLights.push_back(dynamic_cast<CDH_ToolMesh*>(pGameObject));

	return S_OK;
}

void CDHTool::Picking()
{
	if (m_iLightMode == 0)
	{
		_int iID = -1;
		if (m_pGameInstance->Picking_ToolMesh(&iID))
		{
			printf("ID: %d\n", iID);
			//같은 아이디를 가진 오브젝트에 포커스
			//모델 그룹중에서 같은 아이디를 가진 오브젝트를 만날때까지순회

			_bool bFind = false;
			_bool bOpened = false;

			auto LightList = m_pGameInstance->Get_ObjectList(ENUM_CLASS(LEVEL::DH), TEXT("Layer_ToolMesh"));
			for (auto& LightMesh : LightList)
			{
				if (bFind)
					break;

				if (static_cast<CDH_ToolMesh*>(LightMesh)->Get_ID() == iID)
				{
					wprintf(L"m_iSelectedModelIndex: %s\n", LightMesh->Get_Name().c_str());
					m_pSelectedObject = dynamic_cast<CDH_ToolMesh*>(LightMesh);
					bFind = true;
					break;
				}
			}

			if (!bFind)
			{
				m_pSelectedObject = nullptr;
			}
		}
	}
}
void CDHTool::PickGuizmo()
{
	ImGuiIO& io = ImGui::GetIO();

	if (!m_pSelectedObject)
		return;

	CTransform* pTransform = dynamic_cast<CTransform*>(m_pSelectedObject->Get_TransfomCom());
	if (nullptr == pTransform)
		return;

	_float4x4 WorldMatrix{}, ViewMatrix{}, ProjMatrix{};

	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	WorldMatrix = *pTransform->Get_WorldMatrix_Ptr();

	RECT rect;
	GetClientRect(g_hWnd, &rect);

	ImGuizmo::SetRect(0, 0, (float)(rect.right - rect.left), (float)(rect.bottom - rect.top));
	ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
	ImGuizmo::Enable(true);

	if (ImGuizmo::Manipulate(
		(float*)&ViewMatrix,
		(float*)&ProjMatrix,
		m_eGizmoOp,
		ImGuizmo::LOCAL,
		(float*)&WorldMatrix))
	{
		_float3 vTranslation{}, vRotation{}, vScale{};
		ImGuizmo::DecomposeMatrixToComponents(
			(float*)&WorldMatrix,
			(float*)&vTranslation.x,
			(float*)&vRotation.x,
			(float*)&vScale.x
		);

		if (m_iLightMode == 0)
			return;

		switch (m_eGizmoOp)
		{
		case ImGuizmo::TRANSLATE:
			pTransform->Set_State(
				STATE::POSITION,
				XMVectorSet(vTranslation.x, vTranslation.y, vTranslation.z, 1.f));
			break;

		case ImGuizmo::ROTATE:
			_matrix matWorld = XMLoadFloat4x4(&WorldMatrix);
			pTransform->QuaternionRotate(matWorld);
			break;

		case ImGuizmo::SCALE:
			pTransform->SetUp_Scale(vScale.x, vScale.y, vScale.z);
			break;
		}
	}
}

void CDHTool::TogglePickMode()
{
	m_iLightMode = (m_iLightMode + 1) % 2; // 0: Pick, 1: Move
}

void CDHTool::DeleteSelectedObject()
{
	if (m_pSelectedObject)
	{
		m_pSelectedObject->Set_bDead();
		auto iter = std::find(m_vecLights.begin(), m_vecLights.end(), m_pSelectedObject);
		if (iter != m_vecLights.end())
			m_vecLights.erase(iter);

		m_pSelectedObject = nullptr;
	}
}




HRESULT CDHTool::Save_Shader(
	_float Diffuse, _float Normal, _float AO, _float AOPower, _float Roughness, _float Metallic, _float Reflection, _float Specular, _float4 vTint)
{
	namespace fs = filesystem;
	fs::create_directories("../Bin/Save/ShaderParameters");

	ofstream ShaderDataFile("../Bin/Save/ShaderParameters/ShaderData.json");
	if (!ShaderDataFile.is_open())
	{
		MSG_BOX("셰이더 저장 실패: 파일 열기 실패");
		return E_FAIL;
	}

	json ShaderDataJson;

	ShaderDataJson["DiffuseIntensity"] = Diffuse;
	ShaderDataJson["NormalIntensity"] = Normal;
	ShaderDataJson["AOIntensity"] = AO;
	ShaderDataJson["AOPower"] = AOPower;
	ShaderDataJson["RoughnessIntensity"] = Roughness;
	ShaderDataJson["MetallicIntensity"] = Metallic;
	ShaderDataJson["ReflectionIntensity"] = Reflection;
	ShaderDataJson["SpecularIntensity"] = Specular;
	ShaderDataJson["AlbedoTint"] = { vTint.x, vTint.y, vTint.z, vTint.w };

	ShaderDataFile << setw(4) << ShaderDataJson << endl;
	ShaderDataFile.close();

	MSG_BOX("셰이더 저장 성공");
	return S_OK;
}
HRESULT CDHTool::Load_Shader(
	_float& Diffuse, _float& Normal, _float& AO, _float& AOPower, _float& Roughness, _float& Metallic, _float& Reflection, _float& Specular, _float4& vTint)
{
	// [1] 경로 준비
	const string strPath = "../Bin/Save/ShaderParameters/ShaderData.json";

	// [2] 파일 존재 여부 확인
	if (!filesystem::exists(strPath))
	{
		MSG_BOX("셰이더 불러오기 실패: 저장된 파일이 없습니다.");
		return E_FAIL;
	}

	// [3] JSON 파일 열기
	ifstream ShaderDataFile(strPath);
	if (!ShaderDataFile.is_open())
	{
		MSG_BOX("셰이더 불러오기 실패: 파일 열기 실패.");
		return E_FAIL;
	}

	// [4] JSON 파싱
	json ShaderDataJson;
	ShaderDataFile >> ShaderDataJson;

	// [5] 각 값이 존재할 경우에만 안전하게 불러오기
	if (ShaderDataJson.contains("DiffuseIntensity"))     Diffuse = ShaderDataJson["DiffuseIntensity"];
	if (ShaderDataJson.contains("NormalIntensity"))      Normal = ShaderDataJson["NormalIntensity"];
	if (ShaderDataJson.contains("AOIntensity"))          AO = ShaderDataJson["AOIntensity"];
	if (ShaderDataJson.contains("AOPower"))              AOPower = ShaderDataJson["AOPower"];
	if (ShaderDataJson.contains("RoughnessIntensity"))   Roughness = ShaderDataJson["RoughnessIntensity"];
	if (ShaderDataJson.contains("MetallicIntensity"))    Metallic = ShaderDataJson["MetallicIntensity"];
	if (ShaderDataJson.contains("ReflectionIntensity"))  Reflection = ShaderDataJson["ReflectionIntensity"];
	if (ShaderDataJson.contains("SpecularIntensity"))    Specular = ShaderDataJson["SpecularIntensity"];

	if (ShaderDataJson.contains("AlbedoTint"))
	{
		const auto& TintArray = ShaderDataJson["AlbedoTint"];
		if (TintArray.is_array() && TintArray.size() == 4)
		{
			vTint = _float4(
				TintArray[0].get<_float>(),
				TintArray[1].get<_float>(),
				TintArray[2].get<_float>(),
				TintArray[3].get<_float>()
			);
		}
	}

	ShaderDataFile.close();

	return S_OK;
}

void CDHTool::Save_Lights(LEVEL_TYPE eLType)
{
	nlohmann::json jLights = nlohmann::json::array();

	for (CDH_ToolMesh* pToolMesh : m_vecLights)
	{
		if (nullptr == pToolMesh)
			continue;

		// [ 툴 매쉬의 월드 행렬 , 라이트의 밝기, 색상, 범위 ]
		_matrix matWorld = pToolMesh->Get_TransfomCom()->Get_WorldMatrix();
		_float fIntensity = pToolMesh->GetIntensity();
		_float fRange = pToolMesh->GetRange();
		_float4 vColor = pToolMesh->GetColor();

		/* [ 이너앵글, 아웃앵글, 펠 오프, 포그덴시티, 포그 컷 오프 ] */
		_float fInnerCosAngle = pToolMesh->GetfInnerCosAngle();
		_float fOuterCosAngle = pToolMesh->GetfOuterCosAngle();
		_float fFalloff = pToolMesh->GetfFalloff();
		_float fFogDensity = pToolMesh->GetfFogDensity();
		_float fFogCutOff = pToolMesh->GetfFogCutOff();
		_bool  bVolumetric = pToolMesh->GetbVolumetric();

		XMFLOAT4X4 matOut;
		XMStoreFloat4x4(&matOut, matWorld);

		// 행렬을 2차원 배열로 변환
		vector<vector<float>> matrixArray = {
		{ matOut._11, matOut._12, matOut._13, matOut._14 },
		{ matOut._21, matOut._22, matOut._23, matOut._24 },
		{ matOut._31, matOut._32, matOut._33, matOut._34 },
		{ matOut._41, matOut._42, matOut._43, matOut._44 }
		};

		// JSON 객체로 구성
		nlohmann::json jLight;
		jLight["WorldMatrix"] = matrixArray;
		jLight["Intensity"] = fIntensity;
		jLight["Range"] = fRange;
		jLight["Color"] = { vColor.x, vColor.y, vColor.z, vColor.w };

		jLight["InnerCosAngle"] = fInnerCosAngle;
		jLight["OuterCosAngle"] = fOuterCosAngle;
		jLight["Falloff"] = fFalloff;
		jLight["FogDensity"] = fFogDensity;
		jLight["FogCutOff"] = fFogCutOff;
		jLight["Volumetric"] = bVolumetric;

		jLight["LightType"] = pToolMesh->GetLightType();
		jLight["LevelType"] = static_cast<int>(eLType);

		// 배열에 추가
		jLights.push_back(jLight);
	}

	wstring basePath = L"D:\\LieOfP\\Client\\Bin\\Save\\LightInfomation\\";
	wstring fileName = L"Light_Information.json";

	ofstream ofs(basePath + fileName);
	if (!ofs.is_open())
	{
		MSG_BOX("파일 열기에 실패했습니다.");
		return;
	}
	ofs << setw(4) << jLights;
	ofs.close();

	MSG_BOX("라이트 저장 성공");
}
void CDHTool::Load_Lights(LEVEL_TYPE eLType)
{
	wstring basePath = L"D:\\LieOfP\\Client\\Bin\\Save\\LightInfomation\\";
	wstring fileName = L"Light_Information.json";

	ifstream ifs(basePath + fileName);
	if (!ifs.is_open())
		return;

	nlohmann::json jLights;
	ifs >> jLights;
	ifs.close();

	for (const auto& jLight : jLights)
	{
		// 1. 기본 정보 추출
		vector<vector<float>> matrixArray = jLight["WorldMatrix"];
		_float fIntensity = jLight["Intensity"];
		_float fRange = jLight["Range"];
		vector<float> colorVec = jLight["Color"];

		_float fInnerCosAngle = jLight["InnerCosAngle"];
		_float fOuterCosAngle = jLight["OuterCosAngle"];
		_float fFalloff = jLight["Falloff"];
		_float fFogDensity = jLight["FogDensity"];
		_float fFogCutOff = jLight["FogCutOff"];
		_int m_iVolumetricMode = jLight["Volumetric"].get<int>();

		LIGHT_TYPE eLightType = static_cast<LIGHT_TYPE>(jLight["LightType"].get<int>());
		LEVEL_TYPE eLevelType = static_cast<LEVEL_TYPE>(jLight["LevelType"].get<int>());

		// 2. 행렬 복원
		XMFLOAT4X4 mat;
		mat._11 = matrixArray[0][0]; mat._12 = matrixArray[0][1]; mat._13 = matrixArray[0][2]; mat._14 = matrixArray[0][3];
		mat._21 = matrixArray[1][0]; mat._22 = matrixArray[1][1]; mat._23 = matrixArray[1][2]; mat._24 = matrixArray[1][3];
		mat._31 = matrixArray[2][0]; mat._32 = matrixArray[2][1]; mat._33 = matrixArray[2][2]; mat._34 = matrixArray[2][3];
		mat._41 = matrixArray[3][0]; mat._42 = matrixArray[3][1]; mat._43 = matrixArray[3][2]; mat._44 = matrixArray[3][3];
		_matrix matWorld = XMLoadFloat4x4(&mat);

		// 3. 라이트 생성
		if (FAILED(Add_Light(eLightType, eLevelType)))
			continue;

		// 4. 생성된 라이트 설정
		CDH_ToolMesh* pNewLight = m_vecLights.back();
		if (!pNewLight)
			continue;

		pNewLight->Get_TransfomCom()->Set_WorldMatrix(matWorld);
		pNewLight->SetIntensity(fIntensity);
		pNewLight->SetRange(fRange);
		pNewLight->SetColor(_float4(colorVec[0], colorVec[1], colorVec[2], colorVec[3]));
		pNewLight->SetfInnerCosAngle(fInnerCosAngle);
		pNewLight->SetfOuterCosAngle(fOuterCosAngle);
		pNewLight->SetfFalloff(fFalloff);
		pNewLight->SetfFogDensity(fFogDensity);
		pNewLight->SetfFogCutOff(fFogCutOff);
		pNewLight->SetbVolumetric(m_iVolumetricMode);
	}
}

void CDHTool::DeleteAllLights()
{
	for (CDH_ToolMesh* pToolMesh : m_vecLights)
	{
		if (nullptr == pToolMesh)
			continue;

		pToolMesh->Set_bDead();
	}

	m_pSelectedObject = nullptr;
}

HRESULT CDHTool::Duplicate_Selected_Object()
{
	if (!m_pSelectedObject)
		return E_FAIL;

	/* [ 선택된 라이트의 정보추출 ] */
	// 1. 기본 정보 추출
	_matrix matLightWorld = m_pSelectedObject->Get_TransfomCom()->Get_WorldMatrix();
	_float fIntensity = m_pSelectedObject->GetIntensity();
	_float fRange = m_pSelectedObject->GetRange();
	_float4 vColor = m_pSelectedObject->GetColor();

	_float fInnerCosAngle = m_pSelectedObject->GetfInnerCosAngle();
	_float fOuterCosAngle = m_pSelectedObject->GetfOuterCosAngle();
	_float fFalloff = m_pSelectedObject->GetfFalloff();
	_float fFogDensity = m_pSelectedObject->GetfFogDensity();
	_float fFogCutOff = m_pSelectedObject->GetfFogCutOff();

	LIGHT_TYPE eLightType = static_cast<LIGHT_TYPE>(m_pSelectedObject->GetLightType());
	LEVEL_TYPE eLevelType = static_cast<LEVEL_TYPE>(m_iSelectedLevelType);


	// 3. 라이트 생성
	if (FAILED(Add_Light(eLightType, eLevelType)))
		return E_FAIL;

	// 4. 생성된 라이트 설정
	CDH_ToolMesh* pNewLight = m_vecLights.back();

	pNewLight->Get_TransfomCom()->Set_WorldMatrix(matLightWorld);
	pNewLight->SetIntensity(fIntensity);
	pNewLight->SetRange(fRange);
	pNewLight->SetColor(vColor);
	pNewLight->SetfInnerCosAngle(fInnerCosAngle);
	pNewLight->SetfOuterCosAngle(fOuterCosAngle);
	pNewLight->SetfFalloff(fFalloff);
	pNewLight->SetfFogDensity(fFogDensity);
	pNewLight->SetfFogCutOff(fFogCutOff);

	//포커스 되는 오브젝트 변경
	m_pSelectedObject = pNewLight;

	return S_OK;
}

CDHTool* CDHTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CDHTool* pInstance = new CDHTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CDHTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CDHTool::Clone(void* pArg)
{
	CDHTool* pInstance = new CDHTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CDHTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CDHTool::Free()
{
	__super::Free();

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pShaderComInstance);
}
