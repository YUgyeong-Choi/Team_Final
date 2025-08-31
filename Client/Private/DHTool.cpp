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

	/* [ ���� ���� ���̴� �ε� ] */
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
	// QWER Ű ���� (Maya ��Ÿ��)
	if (KEY_DOWN(DIK_Q))
		m_pSelectedObject = nullptr;
	else if (KEY_DOWN(DIK_W))
		m_eGizmoOp = ImGuizmo::TRANSLATE;
	else if (KEY_DOWN(DIK_E))
		m_eGizmoOp = ImGuizmo::ROTATE;
	else if (KEY_DOWN(DIK_R))
		m_eGizmoOp = ImGuizmo::SCALE;

	// ���� (������Ʈ Ŭ�� ��)
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
		/* [ �� �Ķ���Ϳ� ���� Max �� ] */
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

	// ���õ� ���¿� ���� ������ ���� ����
	ImVec4 moveColor = ImVec4(1.f, 0.2f, 0.2f, 1.f);   // ������
	ImVec4 pickColor = ImVec4(0.2f, 0.6f, 1.f, 1.f);   // �Ķ��� �� ���� ����

	// ��Ÿ�� ���� (�����̴� ������ ���� ����)
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, m_iLightMode == 0 ? pickColor : moveColor);
	// �����̴� �� ����
	ImGui::PushItemWidth(265);
	// �����̴� ��ü
	ImGui::SliderInt("##LightModeSlider", &m_iLightMode, 0, 1,
		m_iLightMode == 0 ? "Pick" : "Move");

	// ��Ÿ�� ���󺹱�
	ImGui::PopItemWidth();
	ImGui::PopStyleColor();
	// ���õ� ��� ��� (����)
	ImGui::Text("Current Mode: %s", m_iLightMode == 0 ? "Pick" : "Move");

	// ------------------------------------------------------------------//
	ImVec4 NoneColor = ImVec4(0.2f, 0.2f, 1.f, 1.f);
	ImVec4 VolumeColor = ImVec4(0.2f, 0.6f, 1.f, 1.f);

	// ��Ÿ�� ���� (�����̴� ������ ���� ����)
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
			m_iVolumetricMode = 1; // �⺻�� ����
	}

	// ��Ÿ�� ���󺹱�
	ImGui::PopItemWidth();
	ImGui::PopStyleColor();
	ImGui::Text("Current Mode: %s", m_iVolumetricMode == 0 ? "VOLUMETRIC" : "NONE");

	// ComboBox�� ���� Ÿ�� ����
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

	// ���õ� ���� ���� ��ư
	if (ImGui::Button("Create Light"))
	{
		Create_Light((LIGHT_TYPE)m_iSelectedLightType, (LEVEL_TYPE)m_iSelectedLevelType);
	}

	ImGui::Separator();

	// ���� ����Ʈ ���� ǥ��
	int iDirectionalCount = m_pGameInstance->Get_LightCount(0, ENUM_CLASS(LEVEL::DH));
	int iSpotCount = m_pGameInstance->Get_LightCount(1, ENUM_CLASS(LEVEL::DH));
	int iPointCount = m_pGameInstance->Get_LightCount(2, ENUM_CLASS(LEVEL::DH));
	
	ImGui::Text("Light Count:");
	ImGui::BulletText("Point Light      : %d", iPointCount);
	ImGui::BulletText("Spot Light       : %d", iSpotCount);
	ImGui::BulletText("Directional Light: %d", iDirectionalCount);

	PickGuizmo();

	ImGui::Text("Light Parameters");

	// ����(Intensity)
	if (m_pSelectedObject != nullptr)
	{
		// 1. ���õ� ������Ʈ�� Intensity �� ��������
		float fIntensity = m_pSelectedObject->GetIntensity();

		// 2. �����̴� UI�� ǥ�� �� ����
		if (ImGui::SliderFloat("Intensity", &fIntensity, 0.0f, 10.0f, "%.2f"))
		{
			// 3. ����ڰ� �� ���� �� �ٽ� ����
			m_pSelectedObject->SetIntensity(fIntensity);
		}
	}
	else
	{
		float fIntensity = 1.f;
		ImGui::SliderFloat("Intensity", &fIntensity, 0.0f, 10.0f, "%.2f");
	}

	// ���� (Color)
	if (m_pSelectedObject != nullptr)
	{
		// 1. XMFLOAT4 �� ImVec4 ��ȯ
		_float4 dxColor = m_pSelectedObject->GetColor(); // XMFLOAT4 or _float4
		ImVec4 vColor = ImVec4(dxColor.x, dxColor.y, dxColor.z, dxColor.w);

		// 2. ���� ���� UI
		if (ImGui::ColorEdit3("Color", (float*)&vColor)) // ���� ����
		{
			// 3. ImVec4 �� _float4�� �ٽ� ��ȯ
			_float4 updatedColor(vColor.x, vColor.y, vColor.z, vColor.w);
			m_pSelectedObject->SetColor(updatedColor);
		}
	}
	else
	{
		static ImVec4 vColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImGui::ColorEdit3("Color", (float*)&vColor);
	}

	// ����(Range)
	if (m_pSelectedObject != nullptr)
	{
		// 1. ���õ� ������Ʈ�� Range �� ��������
		float fRange = m_pSelectedObject->GetRange();

		// 2. Range �����̴� UI
		if (ImGui::SliderFloat("Range", &fRange, 0.01f, 50.0f, "%.1f"))
		{
			// 3. ���� �ٲ���� �� �ٽ� ����
			m_pSelectedObject->SetRange(fRange);
		}
	}
	else
	{
		// ���õ� ������Ʈ�� ���� ��� �⺻���� ǥ�� (��Ȱ�� ����)
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
			//���� ���̵� ���� ������Ʈ�� ��Ŀ��
			//�� �׷��߿��� ���� ���̵� ���� ������Ʈ�� ������������ȸ

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
		MSG_BOX("���̴� ���� ����: ���� ���� ����");
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

	MSG_BOX("���̴� ���� ����");
	return S_OK;
}
HRESULT CDHTool::Load_Shader(
	_float& Diffuse, _float& Normal, _float& AO, _float& AOPower, _float& Roughness, _float& Metallic, _float& Reflection, _float& Specular, _float4& vTint)
{
	// [1] ��� �غ�
	const string strPath = "../Bin/Save/ShaderParameters/ShaderData.json";

	// [2] ���� ���� ���� Ȯ��
	if (!filesystem::exists(strPath))
	{
		MSG_BOX("���̴� �ҷ����� ����: ����� ������ �����ϴ�.");
		return E_FAIL;
	}

	// [3] JSON ���� ����
	ifstream ShaderDataFile(strPath);
	if (!ShaderDataFile.is_open())
	{
		MSG_BOX("���̴� �ҷ����� ����: ���� ���� ����.");
		return E_FAIL;
	}

	// [4] JSON �Ľ�
	json ShaderDataJson;
	ShaderDataFile >> ShaderDataJson;

	// [5] �� ���� ������ ��쿡�� �����ϰ� �ҷ�����
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

		// [ �� �Ž��� ���� ��� , ����Ʈ�� ���, ����, ���� ]
		_matrix matWorld = pToolMesh->Get_TransfomCom()->Get_WorldMatrix();
		_float fIntensity = pToolMesh->GetIntensity();
		_float fRange = pToolMesh->GetRange();
		_float4 vColor = pToolMesh->GetColor();

		/* [ �̳ʾޱ�, �ƿ��ޱ�, �� ����, ���׵���Ƽ, ���� �� ���� ] */
		_float fInnerCosAngle = pToolMesh->GetfInnerCosAngle();
		_float fOuterCosAngle = pToolMesh->GetfOuterCosAngle();
		_float fFalloff = pToolMesh->GetfFalloff();
		_float fFogDensity = pToolMesh->GetfFogDensity();
		_float fFogCutOff = pToolMesh->GetfFogCutOff();
		_bool  bVolumetric = pToolMesh->GetbVolumetric();

		XMFLOAT4X4 matOut;
		XMStoreFloat4x4(&matOut, matWorld);

		// ����� 2���� �迭�� ��ȯ
		vector<vector<float>> matrixArray = {
		{ matOut._11, matOut._12, matOut._13, matOut._14 },
		{ matOut._21, matOut._22, matOut._23, matOut._24 },
		{ matOut._31, matOut._32, matOut._33, matOut._34 },
		{ matOut._41, matOut._42, matOut._43, matOut._44 }
		};

		// JSON ��ü�� ����
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

		// �迭�� �߰�
		jLights.push_back(jLight);
	}

	wstring basePath = L"D:\\LieOfP\\Client\\Bin\\Save\\LightInfomation\\";
	wstring fileName = L"Light_Information.json";

	ofstream ofs(basePath + fileName);
	if (!ofs.is_open())
	{
		MSG_BOX("���� ���⿡ �����߽��ϴ�.");
		return;
	}
	ofs << setw(4) << jLights;
	ofs.close();

	MSG_BOX("����Ʈ ���� ����");
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
		// 1. �⺻ ���� ����
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

		// 2. ��� ����
		XMFLOAT4X4 mat;
		mat._11 = matrixArray[0][0]; mat._12 = matrixArray[0][1]; mat._13 = matrixArray[0][2]; mat._14 = matrixArray[0][3];
		mat._21 = matrixArray[1][0]; mat._22 = matrixArray[1][1]; mat._23 = matrixArray[1][2]; mat._24 = matrixArray[1][3];
		mat._31 = matrixArray[2][0]; mat._32 = matrixArray[2][1]; mat._33 = matrixArray[2][2]; mat._34 = matrixArray[2][3];
		mat._41 = matrixArray[3][0]; mat._42 = matrixArray[3][1]; mat._43 = matrixArray[3][2]; mat._44 = matrixArray[3][3];
		_matrix matWorld = XMLoadFloat4x4(&mat);

		// 3. ����Ʈ ����
		if (FAILED(Add_Light(eLightType, eLevelType)))
			continue;

		// 4. ������ ����Ʈ ����
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

	/* [ ���õ� ����Ʈ�� �������� ] */
	// 1. �⺻ ���� ����
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


	// 3. ����Ʈ ����
	if (FAILED(Add_Light(eLightType, eLevelType)))
		return E_FAIL;

	// 4. ������ ����Ʈ ����
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

	//��Ŀ�� �Ǵ� ������Ʈ ����
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
