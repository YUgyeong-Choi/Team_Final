#include "DHTool.h"
#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

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

	return S_OK;
}

void CDHTool::Priority_Update(_float fTimeDelta)
{

}

void CDHTool::Update(_float fTimeDelta)
{
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
    SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
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
		
		static ImVec4 vTintColor = ImVec4(vDiffuseTint.x, vDiffuseTint.y, vDiffuseTint.z, 1.f);

		if (ImGui::ColorPicker4("Tint Color", (float*)&vTintColor,
			ImGuiColorEditFlags_DisplayHSV |
			ImGuiColorEditFlags_PickerHueWheel))
		{
			bUpdated = true;
		}
		
        ImGui::Spacing();
		if (bUpdated)
		{
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fDiffuseIntensity", &fDiffuseIntensity, sizeof(_float))))
				return E_FAIL;
			_float4 vAlbedoTint = _float4(vTintColor.x, vTintColor.y, vTintColor.z, vTintColor.w);
			if (FAILED(m_pShaderCom->Bind_RawValue("g_vDiffuseTint", &vAlbedoTint, sizeof(_float4))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fNormalIntensity", &fNormalIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fAOIntensity", &fAOIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fAOPower", &fAOPower, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fRoughnessIntensity", &fRoughnessIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fMetallicIntensity", &fMetallicIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fReflectionIntensity", &fReflectionIntensity, sizeof(_float))))
				return E_FAIL;
			if (FAILED(m_pShaderCom->Bind_RawValue("g_fSpecularIntensity", &fSpecularIntensity, sizeof(_float))))
				return E_FAIL;
		}
    }
    ImGui::End();
    return S_OK;
}

HRESULT CDHTool::Render_LightTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	ImGui::Begin("Hi2 Tools", &open, NULL);


	IGFD::FileDialogConfig config;
	if (Button("Merge Animations"))
	{
	}


	ImGui::End();
	return S_OK;
}

HRESULT CDHTool::Save_Shader(
	_float Diffuse, _float Normal, _float AO, _float AOPower, _float Roughness, _float Metallic, _float Reflection, _float Specular, _float4 vTint)
{
	namespace fs = std::filesystem;
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

	ShaderDataFile << std::setw(4) << ShaderDataJson << std::endl;
	ShaderDataFile.close();

	MSG_BOX("셰이더 저장 성공");
	return S_OK;
}

HRESULT CDHTool::Load_Shader(
	_float& Diffuse, _float& Normal, _float& AO, _float& AOPower, _float& Roughness, _float& Metallic, _float& Reflection, _float& Specular, _float4& vTint)
{
	// [1] 경로 준비
	const std::string strPath = "../Bin/Save/ShaderParameters/ShaderData.json";

	// [2] 파일 존재 여부 확인
	if (!std::filesystem::exists(strPath))
	{
		MSG_BOX("셰이더 불러오기 실패: 저장된 파일이 없습니다.");
		return E_FAIL;
	}

	// [3] JSON 파일 열기
	std::ifstream ShaderDataFile(strPath);
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
}
