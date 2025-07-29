#include "GLTool.h"
#include "GameInstance.h"
#include "UI_Feature.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CGLTool::CGLTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CGLTool::CGLTool(const CGLTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CGLTool::Initialize_Prototype()
{

	
	
	return S_OK;
}

HRESULT CGLTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	auto map = m_pGameInstance->Get_Prototypes();

	for (auto& pair : *map)
	{
		if(pair.first.find(L"Texture") != pair.first.npos)
			m_TextureNames.push_back(pair.first);
	}

	m_pSequence = new CUI_Sequence();

	return S_OK;
}

void CGLTool::Priority_Update(_float fTimeDelta)
{

}

void CGLTool::Update(_float fTimeDelta)
{
	if (m_isPlay)
	{
		m_fElapsedTime += fTimeDelta;

		if (m_fElapsedTime > 0.016f)
		{
			++m_iCurrentFrame;
			m_fElapsedTime = 0.f;

		}
	}

	for (const auto& pObj : m_DynamicUIList)
	{

		if (nullptr == pObj || pObj->Get_bDead())
			continue;

		pObj->Update_UI_From_Tool(m_iCurrentFrame);
	}

	
		
}

void CGLTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CGLTool::Render()
{

	if (FAILED(Render_SelectOptionTool()))
		return E_FAIL;

	if (FAILED(Render_UIList()))
		return E_FAIL;

	if (FAILED(Render_Sequence()))
		return E_FAIL;

	return S_OK;
}

void CGLTool::Save_File()
{
	json JsonArrayStatic = json::array();
	

	for (auto& pObj : m_StaticUIList)
	{
		json j;

		if (nullptr == pObj || pObj->Get_bDead())
			continue;
		auto eDesc = pObj->Get_Desc();

		// 이제 값 채워서 dump로 넘겨준다 
		j["TextureTag"] = WStringToString(eDesc.strTextureTag);
		j["PassIndex"] = eDesc.iPassIndex;
		j["TextureIndex"] = eDesc.iTextureIndex;
		j["Offset"] = eDesc.fOffset;
		j["fSizeX"] = eDesc.fSizeX / g_iWinSizeX;
		j["fSizeY"] = eDesc.fSizeY / g_iWinSizeY;
		j["fX"] = eDesc.fX / g_iWinSizeX; 
		j["fY"] = eDesc.fY / g_iWinSizeY;
		j["Color"] = { eDesc.vColor.x,  eDesc.vColor.y,  eDesc.vColor.z,  eDesc.vColor.w, };
		JsonArrayStatic.push_back(j);
		
	}

	ofstream file("../Bin/DataFiles/UI/Temp_Static.json");

	file << JsonArrayStatic.dump(4);

	file.close();

	JsonArrayStatic.clear();

	// dynamic 따로 저장

	json JsonArrayDynamic = json::array();

	for (auto& pObj : m_DynamicUIList)
	{
		if (nullptr == pObj || pObj->Get_bDead())
			continue;

		auto eDesc = pObj->Get_Desc();

		json j;
		j["TextureTag"] = WStringToString(eDesc.strTextureTag);
		j["PassIndex"] = eDesc.iPassIndex;
		j["TextureIndex"] = eDesc.iTextureIndex;
		j["Offset"] = eDesc.fOffset;
		j["fSizeX"] = eDesc.fSizeX / g_iWinSizeX;
		j["fSizeY"] = eDesc.fSizeY / g_iWinSizeY;
		j["fX"] = eDesc.fX / g_iWinSizeX;
		j["fY"] = eDesc.fY / g_iWinSizeY;
		j["Color"] = { eDesc.vColor.x,  eDesc.vColor.y,  eDesc.vColor.z,  eDesc.vColor.w, };
		j["Duration"] = eDesc.fDuration;

		// Features 배열 생성
		json jFeatures = json::array();

		for (auto& pFeature : pObj->Get_Features())
		{
			if (nullptr == pFeature)
				continue;

			auto& eFeatureDesc = pFeature->Get_Desc();

			json jFeature;
			jFeature["ProtoTag"] = eFeatureDesc.strProtoTag;
			jFeature["isLoop"] = eFeatureDesc.isLoop;
			jFeature["iStartFrame"] = eFeatureDesc.iStartFrame;
			jFeature["iEndFrame"] = eFeatureDesc.iEndFrame;

			if (eFeatureDesc.strProtoTag.find("Fade") != string::npos)
			{
				UI_FEATURE_FADE_DESC& eFadeDesc = static_cast<UI_FEATURE_FADE_DESC&>(eFeatureDesc);
				jFeature["fStartAlpha"] = eFadeDesc.fStartAlpha;
				jFeature["fEndAlpha"] = eFadeDesc.fEndAlpha;
			}
			else if (eFeatureDesc.strProtoTag.find("Pos") != string::npos)
			{
				UI_FEATURE_POS_DESC& ePosDesc = static_cast<UI_FEATURE_POS_DESC&>(eFeatureDesc);
				jFeature["fStartPos"]["x"] = ePosDesc.fStartPos.x;
				jFeature["fStartPos"]["y"] = ePosDesc.fStartPos.y;
				jFeature["fEndPos"]["x"] = ePosDesc.fEndPos.x;
				jFeature["fEndPos"]["y"] = ePosDesc.fEndPos.y;
			}
			else if (eFeatureDesc.strProtoTag.find("UV") != string::npos)
			{
				UI_FEATURE_UV_DESC& ePosDesc = static_cast<UI_FEATURE_UV_DESC&>(eFeatureDesc);
				jFeature["fStartUV"]["x"] = ePosDesc.fStartUV.x;
				jFeature["fStartUV"]["y"] = ePosDesc.fStartUV.y;
				jFeature["fOffsetUV"]["x"] = ePosDesc.fOffsetUV.x;
				jFeature["fOffsetUV"]["y"] = ePosDesc.fOffsetUV.y;
			}
			else if (eFeatureDesc.strProtoTag.find("Scale") != string::npos)
			{
				UI_FEATURE_SCALE_DESC& ePosDesc = static_cast<UI_FEATURE_SCALE_DESC&>(eFeatureDesc);
				jFeature["fStartScale"]["x"] = ePosDesc.fStartScale.x;
				jFeature["fStartScale"]["y"] = ePosDesc.fStartScale.y;
				jFeature["fEndScale"]["x"] = ePosDesc.fEndScale.x;
				jFeature["fEndScale"]["y"] = ePosDesc.fEndScale.y;
			}

			jFeatures.push_back(jFeature);
		}

		// 최종적으로 Feature 배열을 넣어줌
		j["Features"] = jFeatures;

		JsonArrayDynamic.push_back(j);
	}

	ofstream ofile("../Bin/DataFiles/UI/Temp_Dynamic.json");
	ofile << JsonArrayDynamic.dump(4); 

	ofile.close();

}

void CGLTool::Open_File()
{
	

	
	IGFD::FileDialogConfig config;

	m_strSavePath = R"(../Bin/DataFiles/UI)";
	config.path = m_strSavePath;
	config.countSelectionMax = 0; // 무제한
	

	IFILEDIALOG->OpenDialog("JsonDialog", "Select json File", ".json", config);

	
}

void CGLTool::Add_Static_UI_From_File()
{

	string filePath = IFILEDIALOG->GetFilePathName();

	json j;

	ifstream file(filePath);

	file >> j;

	for (const auto& eUIJson : j)
	{
		CStatic_UI::STATIC_UI_DESC eStaticDesc = {};

		

		eStaticDesc.fOffset = eUIJson["Offset"]  ;
		eStaticDesc.iPassIndex = eUIJson["PassIndex"];
		eStaticDesc.iTextureIndex = eUIJson["TextureIndex"]; 
		eStaticDesc.fSizeX = eUIJson["fSizeX"].get<float>() * g_iWinSizeX;
		eStaticDesc.fSizeY = eUIJson["fSizeY"].get<float>() * g_iWinSizeY;
		eStaticDesc.fX = eUIJson["fX"].get<float>() * g_iWinSizeX;
		eStaticDesc.fY = eUIJson["fY"].get<float>() * g_iWinSizeY;

		eStaticDesc.vColor = { eUIJson["Color"][0], eUIJson["Color"][1], eUIJson["Color"][2],eUIJson["Color"][3], };

		string textureTag = eUIJson["TextureTag"];
		eStaticDesc.strTextureTag = wstring(textureTag.begin(), textureTag.end());

		if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
			static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eStaticDesc)))
			return;

		auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

		m_StaticUIList.push_back(static_cast<CStatic_UI*>(pObj));
	}

	file.close();
}

void CGLTool::Add_Dynamic_UI_From_File()
{
	//

	string filePath = IFILEDIALOG->GetFilePathName();

	json j;

	ifstream file(filePath);

	file >> j;

	for (const auto& eUIJson : j)
	{
		CDynamic_UI::DYNAMIC_UI_DESC eDesc = {};


		eDesc.fOffset = eUIJson["Offset"];
		eDesc.iPassIndex = eUIJson["PassIndex"];
		eDesc.iTextureIndex = eUIJson["TextureIndex"];
		eDesc.fSizeX = eUIJson["fSizeX"].get<float>() * g_iWinSizeX;
		eDesc.fSizeY = eUIJson["fSizeY"].get<float>() * g_iWinSizeY;
		eDesc.fX = eUIJson["fX"].get<float>() * g_iWinSizeX;
		eDesc.fY = eUIJson["fY"].get<float>() * g_iWinSizeY;
		eDesc.fDuration = eUIJson["Duration"].get<float>();
		eDesc.vColor = { eUIJson["Color"][0], eUIJson["Color"][1], eUIJson["Color"][2],eUIJson["Color"][3] };

		string textureTag = eUIJson["TextureTag"];
		eDesc.strTextureTag = wstring(textureTag.begin(), textureTag.end());
		eDesc.isFromTool = true;

		// 기능 읽어서 각 프로토타입에 맞게 구조체 생성해서 넣자
		if (!eUIJson.contains("Features"))
			continue;

		const auto& features = eUIJson["Features"];

		for (const auto& feature : features)
		{
			std::string protoTag = feature["ProtoTag"];

			if (protoTag.find("UV") != protoTag.npos)
			{
				UI_FEATURE_UV_DESC* efeatureDesc = new UI_FEATURE_UV_DESC;
				efeatureDesc->strProtoTag = (protoTag);
				efeatureDesc->iStartFrame = feature["iStartFrame"];
				efeatureDesc->iEndFrame = feature["iEndFrame"];
				efeatureDesc->isLoop = feature["isLoop"];

				efeatureDesc->fStartUV = { feature["fStartUV"]["x"],  feature["fStartUV"]["y"] };
				efeatureDesc->fOffsetUV = { feature["fOffsetUV"]["x"],  feature["fOffsetUV"]["y"] };

				eDesc.FeatureDescs.push_back(efeatureDesc);
			}
			else if (protoTag.find("Pos") != protoTag.npos)
			{
				UI_FEATURE_POS_DESC* efeatureDesc = new UI_FEATURE_POS_DESC;
				efeatureDesc->strProtoTag = (protoTag);
				efeatureDesc->iStartFrame = feature["iStartFrame"];
				efeatureDesc->iEndFrame = feature["iEndFrame"];
				efeatureDesc->isLoop = feature["isLoop"];

				efeatureDesc->fStartPos = { feature["fStartPos"]["x"],  feature["fStartPos"]["y"] };
				efeatureDesc->fEndPos = { feature["fEndPos"]["x"],  feature["fEndPos"]["y"] };

				eDesc.FeatureDescs.push_back(efeatureDesc);
			}
			else if (protoTag.find("Scale") != protoTag.npos)
			{
				UI_FEATURE_SCALE_DESC* efeatureDesc = new UI_FEATURE_SCALE_DESC;
				efeatureDesc->strProtoTag = (protoTag);
				efeatureDesc->iStartFrame = feature["iStartFrame"];
				efeatureDesc->iEndFrame = feature["iEndFrame"];
				efeatureDesc->isLoop = feature["isLoop"];

				efeatureDesc->fStartScale = { feature["fStartScale"]["x"],  feature["fStartScale"]["y"] };
				efeatureDesc->fEndScale = { feature["fEndScale"]["x"],  feature["fEndScale"]["y"] };

				eDesc.FeatureDescs.push_back(efeatureDesc);
			}
			else if (protoTag.find("Fade") != protoTag.npos)
			{
				UI_FEATURE_FADE_DESC* efeatureDesc = new UI_FEATURE_FADE_DESC;
				efeatureDesc->strProtoTag = (protoTag);
				efeatureDesc->iStartFrame = feature["iStartFrame"];
				efeatureDesc->iEndFrame = feature["iEndFrame"];
				efeatureDesc->isLoop = feature["isLoop"];

				efeatureDesc->fStartAlpha = feature["StartAlpha"];
				efeatureDesc->fEndAlpha = feature["fEndAlpha"];

				eDesc.FeatureDescs.push_back(efeatureDesc);
			}
		}

	
		if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
			static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"), &eDesc)))
			return;

		auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"));

		m_DynamicUIList.push_back(static_cast<CDynamic_UI*>(pObj));

		for (auto& pDesc : eDesc.FeatureDescs)
		{
			Safe_Delete(pDesc);
		}
		
	}

	file.close();
}

void CGLTool::Add_Static_UI()
{
	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Static_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"), &eStaticUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Static"));

	m_StaticUIList.push_back(static_cast<CStatic_UI*>(pObj));
}

void CGLTool::Add_Dynamic_UI()
{
	eDynamicUIDesc.isFromTool = true;

	if (FAILED(m_pGameInstance->Add_GameObject(static_cast<_uint>(LEVEL::STATIC), TEXT("Prototype_GameObject_Dynamic_UI"),
		static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"), &eDynamicUIDesc)))
		return;

	auto pObj = m_pGameInstance->Get_LastObject(static_cast<_uint>(LEVEL::GL), TEXT("Layer_Background_Dynamic"));

	m_DynamicUIList.push_back(static_cast<CDynamic_UI*>(pObj));
}

void CGLTool::Add_Sequence_To_DynamicUI()
{
	if (nullptr == m_pSelectDynamicObj)
		return;

	m_pSequence->Clear();

	for (auto& pFeature : m_pSelectDynamicObj->Get_Features())
	{
		if (nullptr == pFeature)
			continue;

		auto eDesc = pFeature->Get_Desc_From_Tool();
		m_pSequence->Push_Item(eDesc);
	}

	

	//m_pSelectDynamicObj->Add_Feature();
}

void CGLTool::Apply_Sequence_To_DynamicUI()
{
	if (nullptr == m_pSelectDynamicObj)
		return;

	for (auto& pFeature : m_pSelectDynamicObj->Get_Features())
	{
		Safe_Release(pFeature);
	}


	for (int i = 0; i < m_pSequence->GetItemCount(); ++i)
	{
		auto eDesc = m_pSequence->Get_Desc(i);

		if (0 == eDesc.iType)
		{
			// fade
			UI_FEATURE_FADE_DESC fadeDesc = {};
			fadeDesc.strProtoTag = "Prototype_Component_UI_Feature_Fade";
			fadeDesc.isLoop = eDesc.isLoop;
			fadeDesc.iStartFrame = eDesc.iStartFrame;
			fadeDesc.iEndFrame = eDesc.iEndFrame;
			fadeDesc.fStartAlpha = eDesc.fStartAlpha;
			fadeDesc.fEndAlpha = eDesc.fEndAlpha;

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(fadeDesc.strProtoTag), &fadeDesc);
			
		}
		else if (1 == eDesc.iType)
		{
			// uv
			UI_FEATURE_UV_DESC uvDesc = {};
			uvDesc.strProtoTag = "Prototype_Component_UI_Feature_UV";
			uvDesc.isLoop = eDesc.isLoop;
			uvDesc.iStartFrame = eDesc.iStartFrame;
			uvDesc.iEndFrame = eDesc.iEndFrame;
			uvDesc.fStartUV = eDesc.fStartUV;
			uvDesc.fOffsetUV = eDesc.fOffsetUV;

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(uvDesc.strProtoTag), &uvDesc);
		}
		else if (2 == eDesc.iType)
		{
			UI_FEATURE_POS_DESC posDesc = {};
			posDesc.strProtoTag = "Prototype_Component_UI_Feature_Pos";
			posDesc.isLoop = eDesc.isLoop;
			posDesc.iStartFrame = eDesc.iStartFrame;
			posDesc.iEndFrame = eDesc.iEndFrame;
			posDesc.fStartPos = eDesc.fStartPos;
			posDesc.fEndPos = eDesc.fEndPos;

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(posDesc.strProtoTag), &posDesc);
		}

		else if (3 == eDesc.iType)
		{
			UI_FEATURE_SCALE_DESC scaleDesc = {};
			scaleDesc.strProtoTag = "Prototype_Component_UI_Feature_Scale";
			scaleDesc.isLoop = eDesc.isLoop;
			scaleDesc.iStartFrame = eDesc.iStartFrame;
			scaleDesc.iEndFrame = eDesc.iEndFrame;
			scaleDesc.fStartScale = eDesc.fStartScale;
			scaleDesc.fEndScale = eDesc.fEndScale;
			

			m_pSelectDynamicObj->Add_Feature(static_cast<int>(LEVEL::STATIC), StringToWString(scaleDesc.strProtoTag), &scaleDesc);
		}
	}
}

HRESULT CGLTool::Render_SelectOptionTool()
{
	
	SetNextWindowSize(ImVec2(400, 400));
	_bool open = true;
	ImGui::Begin("Select option ", &open, NULL);

	//float value = 0.6f;
	//ImGui::SliderFloat("Offset", &value, 0.0f, 1.0f);

	if (ImGui::BeginTabBar("Util"))
	{
		if (ImGui::BeginTabItem("Input Static Desc"))
		{
			// 입력 칸 만들기

			ImGui::InputFloat4("Color", reinterpret_cast<float*>(&eStaticUITempDesc.vColor));

			ImGui::InputFloat("Offset", &eStaticUITempDesc.fOffset);
			ImGui::InputInt("PassIndex", &eStaticUITempDesc.iPassIndex);
			ImGui::InputInt("TextureIndex", &eStaticUITempDesc.iTextureIndex);
			ImGui::InputFloat("SizeX_Ratio", &eStaticUITempDesc.fSizeX);
			if (eStaticUITempDesc.fSizeX <= 0.0f)
				eStaticUITempDesc.fSizeX = 0.001f;
			ImGui::InputFloat("SizeY_Ratio", &eStaticUITempDesc.fSizeY);
			if (eStaticUITempDesc.fSizeY <= 0.0f)
				eStaticUITempDesc.fSizeY = 0.001f;
			ImGui::InputFloat("fX_Ratio", &eStaticUITempDesc.fX);
			ImGui::InputFloat("fY_Ratio", &eStaticUITempDesc.fY);

			// 필요하면 슬라이더로
			ImGui::SliderFloat("SizeX_Slider", &eStaticUITempDesc.fSizeX, 0.001f, 1.5f);
			ImGui::SliderFloat("SizeY_Slider", &eStaticUITempDesc.fSizeY, 0.001f, 1.5f);
			ImGui::SliderFloat("fX_Slider", &eStaticUITempDesc.fX, -0.5f, 1.5f);
			ImGui::SliderFloat("fY_Slider", &eStaticUITempDesc.fY, -0.5f, 1.5f);



			eStaticUIDesc = eStaticUITempDesc;
			eStaticUIDesc.fSizeX *= g_iWinSizeX;
			eStaticUIDesc.fSizeY *= g_iWinSizeY;
			eStaticUIDesc.fX *= g_iWinSizeX;
			eStaticUIDesc.fY *= g_iWinSizeY;

			eStaticUIDesc.strTextureTag = m_strSelectName;

			if (eStaticUIDesc.iPassIndex >= UI_END)
			{
				eStaticUIDesc.iPassIndex = UI_END - 1;
				eStaticUITempDesc.iPassIndex = UI_END - 1;
			}

			if (eStaticUIDesc.iPassIndex < 0)
			{
				eStaticUIDesc.iPassIndex = 0;
				eStaticUITempDesc.iPassIndex = 0;
			}

			if (nullptr == m_pSelectStaticObj || m_pSelectStaticObj->Get_bDead())
			{

			}
			else
			{
				m_pSelectStaticObj->Update_UI_From_Tool(eStaticUIDesc);
			}


			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Input Dynamic Desc"))
		{
		
			// 입력 칸 만들기
			ImGui::InputFloat4("Color", reinterpret_cast<float*>(&eDynamicUITempDesc.vColor));
			ImGui::InputFloat("Duration", &eDynamicUITempDesc.fDuration);
			ImGui::InputFloat("Offset", &eDynamicUITempDesc.fOffset);
			ImGui::InputInt("PassIndex", &eDynamicUITempDesc.iPassIndex);
			ImGui::InputInt("TextureIndex", &eDynamicUITempDesc.iTextureIndex);
			ImGui::InputFloat("SizeX_Ratio", &eDynamicUITempDesc.fSizeX);
			if (eDynamicUITempDesc.fSizeX <= 0.0f)
				eDynamicUITempDesc.fSizeX = 0.001f;
			ImGui::InputFloat("SizeY_Ratio", &eDynamicUITempDesc.fSizeY);
			if (eDynamicUITempDesc.fSizeY <= 0.0f)
				eDynamicUITempDesc.fSizeY = 0.001f;
			ImGui::InputFloat("fX_Ratio", &eDynamicUITempDesc.fX);
			ImGui::InputFloat("fY_Ratio", &eDynamicUITempDesc.fY);

			// 필요하면 슬라이더로
			ImGui::SliderFloat("SizeX_Slider", &eDynamicUITempDesc.fSizeX, 0.001f, 1.5f);
			ImGui::SliderFloat("SizeY_Slider", &eDynamicUITempDesc.fSizeY, 0.001f, 1.5f);
			ImGui::SliderFloat("fX_Slider", &eDynamicUITempDesc.fX, -0.5f, 1.5f);
			ImGui::SliderFloat("fY_Slider", &eDynamicUITempDesc.fY, -0.5f, 1.5f);

			eDynamicUIDesc = eDynamicUITempDesc;
			eDynamicUIDesc.fSizeX *= g_iWinSizeX;
			eDynamicUIDesc.fSizeY *= g_iWinSizeY;
			eDynamicUIDesc.fX *= g_iWinSizeX;
			eDynamicUIDesc.fY *= g_iWinSizeY;

			eDynamicUIDesc.strTextureTag = m_strSelectName;

			if (eDynamicUIDesc.iPassIndex >= D_UI_END)
			{
				eDynamicUIDesc.iPassIndex = D_UI_END - 1;
				eDynamicUITempDesc.iPassIndex = D_UI_END - 1;
			}

			if (eDynamicUIDesc.iPassIndex < 0)
			{
				eDynamicUIDesc.iPassIndex = 0;
				eDynamicUITempDesc.iPassIndex = 0;
			}

			if (nullptr == m_pSelectDynamicObj || m_pSelectDynamicObj->Get_bDead())
			{

			}
			else
			{
				m_pSelectDynamicObj->Update_UI_From_Tool(eDynamicUIDesc);
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Input Sequence Desc"))
		{
			// 입력 칸 만들기
			ImGui::Checkbox("IsLoop", &m_eFeatureDesc.isLoop);
			ImGui::InputInt("Type", &m_eFeatureDesc.iType);
			ImGui::InputInt("StartFrame", &m_eFeatureDesc.iStartFrame);
			ImGui::InputInt("EndFrame", &m_eFeatureDesc.iEndFrame);

			ImGui::InputFloat("StartAlpha", &m_eFeatureDesc.fStartAlpha);
			ImGui::InputFloat("EndAlpha", &m_eFeatureDesc.fEndAlpha);

			ImGui::InputFloat2("StartUV", reinterpret_cast<float*>(& m_eFeatureDesc.fStartUV));
			ImGui::InputFloat2("OffsetUV", reinterpret_cast<float*>(& m_eFeatureDesc.fOffsetUV));

			ImGui::InputFloat2("StartPos", reinterpret_cast<float*>(&m_eFeatureDesc.fStartPos));
			ImGui::InputFloat2("EndPos", reinterpret_cast<float*>(&m_eFeatureDesc.fEndPos));

			// 최소값 설정
			ImGui::InputFloat2("StartScale", reinterpret_cast<float*>(&m_eFeatureDesc.fStartScale));
			if (m_eFeatureDesc.fStartScale.x <= 0.0f)
				m_eFeatureDesc.fStartScale.x = 0.001f;
			if (m_eFeatureDesc.fStartScale.y <= 0.0f)
				m_eFeatureDesc.fStartScale.y = 0.001f;

			ImGui::InputFloat2("EndScale", reinterpret_cast<float*>(&m_eFeatureDesc.fEndScale));
			if (m_eFeatureDesc.fEndScale.x <= 0.0f)
				m_eFeatureDesc.fEndScale.x = 0.001f;
			if (m_eFeatureDesc.fEndScale.y <= 0.0f)
				m_eFeatureDesc.fEndScale.y = 0.001f;


			if (0 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "Fade";
			else if (1 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "UV";
			else if (2 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "Pos";
			else if (3 == m_eFeatureDesc.iType)
				m_eFeatureDesc.strTypeTag = "Scale";

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(u8"텍스트 입력하기"))
		{
			size_t buf_size = m_strInput.size() + 512;
			char* buf = new char[buf_size];
			memcpy(buf, m_strInput.c_str(), m_strInput.size() + 1);

			if (ImGui::InputTextMultiline("##multiline", buf, buf_size, ImVec2(-FLT_MIN, 200)))
			{
				m_strInput = buf;
			}

			delete[] buf;

			if (ImGui::Button(u8"메시지 박스 띄우기"))
			{
				_wstring wInput = StringToWStringU8(m_strInput);
				::MessageBox(nullptr, wInput.c_str(), L"error", MB_OK);
			}

		

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Button"))
		{

			IGFD::FileDialogConfig config;
			if (Button(u8"Save File"))
			{
				Save_File();
			}

			if (Button(u8"Open File"))
			{
				Open_File();

			}

			if (Button(u8"Add Static UI"))
			{
				Add_Static_UI();
			}

			if (Button(u8"Add Dynamic UI"))
			{
				Add_Dynamic_UI();
			}

			if (Button(u8"Delete Static UI"))
			{
				if (nullptr != m_pSelectStaticObj)
					m_pSelectStaticObj->Set_bDead();

				int currentIndex = 0;
				for (auto& pObj : m_StaticUIList)
				{
					if (m_iSelectObjIndex == currentIndex)
						pObj = nullptr;

					++currentIndex;
				}

				m_pSelectStaticObj = nullptr;
				m_iSelectObjIndex = -1;
			}

			if (Button(u8"Delete Dynamic UI"))
			{
				if (nullptr != m_pSelectDynamicObj)
					m_pSelectDynamicObj->Set_bDead();

				int currentIndex = 0;
				for (auto& pObj : m_DynamicUIList)
				{
					if (m_iDynamicObjIndex == currentIndex)
						pObj = nullptr;

					++currentIndex;
				}

				m_pSelectDynamicObj = nullptr;
				m_iDynamicObjIndex = -1;
			}

			if (Button(u8"Add Sequence"))
			{
				m_pSequence->Add(0);
				// 추가 한거 마지막에 만든 정보 삽입
				m_pSequence->Upadte_Items(m_pSequence->GetItemCount() - 1, m_eFeatureDesc);
			}

			if (Button(u8"Apply Sequence"))
			{
				// ui에 담아주기
				
				
				Apply_Sequence_To_DynamicUI();

			}

			if (Button(u8"update Select UI Sequence"))
			{
				// 현재 선택한 ui의 시퀀스를 띄운다
				Add_Sequence_To_DynamicUI();

			}
		
			if (Button(u8"Play Sequence"))
			{
				m_isPlay = true;
			}

			if (Button(u8"Stop Sequence and reset"))
			{
				m_isPlay = false;
				m_iCurrentFrame = 0;

				for (const auto& pObj : m_DynamicUIList)
				{

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					pObj->Reset();
				}
			}




			// 파일 탐색기를 띄운다
			ImGui::SetNextWindowSize(ImVec2(800, 600));
			if (IFILEDIALOG->Display("JsonDialog"))
			{
				if (IFILEDIALOG->IsOk())
				{
					string fileName = IFILEDIALOG->GetCurrentFileName();

					if (fileName.find("Static") != fileName.npos)
					{
						Add_Static_UI_From_File();
					}
					else if (fileName.find("Dynamic") != fileName.npos)
					{
						Add_Dynamic_UI_From_File();
					}

					

				}
				IFILEDIALOG->Close();
			}


			ImGui::EndTabItem();
		}


		ImGui::EndTabBar();
	}

		

	ImGui::End();
	return S_OK;
}

HRESULT CGLTool::Render_UIList()
{
	SetNextWindowSize(ImVec2(400, 300));

	_bool open = true;
	ImGui::Begin("UIList", &open, NULL);
	{
		if (ImGui::BeginTabBar("MyTabBar"))
		{
			if (ImGui::BeginTabItem("Static"))
			{
				int index = 0;

				for (const auto& pObj : m_StaticUIList)
				{
					bool isSelected = (index == m_iSelectObjIndex);

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					if (ImGui::Selectable(to_string(index).c_str(), isSelected))
					{
						m_iSelectObjIndex = index;
						m_pSelectStaticObj = pObj;
						eStaticUITempDesc = m_pSelectStaticObj->Get_Desc();
						eStaticUITempDesc.fSizeX /= g_iWinSizeX;
						eStaticUITempDesc.fSizeY /= g_iWinSizeY;
						eStaticUITempDesc.fX /= g_iWinSizeX;
						eStaticUITempDesc.fY /= g_iWinSizeY;
						eStaticUITempDesc.strTextureTag = pObj->Get_StrTextureTag();

					}


					++index;
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Dynamic"))
			{

				int index = 0;

				for (const auto& pObj : m_DynamicUIList)
				{
					bool isSelected = (index == m_iDynamicObjIndex);

					if (nullptr == pObj || pObj->Get_bDead())
						continue;

					if (ImGui::Selectable(to_string(index).c_str(), isSelected))
					{
						m_iDynamicObjIndex = index;
						m_pSelectDynamicObj = pObj;
						eDynamicUITempDesc = m_pSelectDynamicObj->Get_Desc();
						eDynamicUITempDesc.fSizeX /= g_iWinSizeX;
						eDynamicUITempDesc.fSizeY /= g_iWinSizeY;
						eDynamicUITempDesc.fX /= g_iWinSizeX;
						eDynamicUITempDesc.fY /= g_iWinSizeY;
						eDynamicUITempDesc.strTextureTag = pObj->Get_StrTextureTag();

					
					}


					++index;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Texture"))
			{


				int index = 0;

				for (const auto& strName : m_TextureNames)
				{
					bool isSelected = (index == m_iSelectTextureIndex);

					if (ImGui::Selectable(WStringToString(strName).c_str(), isSelected))
					{
						m_iSelectTextureIndex = index;
						m_strSelectName = strName;
					}
					++index;
				}

				ImGui::EndTabItem();
			}


			ImGui::EndTabBar();

		}


		ImGui::End();
	}

	
	
	return S_OK;
}

HRESULT CGLTool::Render_Sequence()
{
	SetNextWindowSize(ImVec2(800, 200));
	_bool open = true;
	ImGui::Begin("Sequence ", &open, NULL);

	//
	
	
	int iFirstFrame = 0;
	ImSequencer::Sequencer(m_pSequence,
		&m_iCurrentFrame,
		&m_bExpanded,
		&m_iSelectedEntry,
		&iFirstFrame, // Optional callback for drawing custom UI
		ImSequencer::SEQUENCER_EDIT_ALL |
		ImSequencer::SEQUENCER_ADD |
		ImSequencer::SEQUENCER_DEL |
		ImSequencer::SEQUENCER_COPYPASTE);

	if (m_iSelectedEntry != -1)
	{
		m_eFeatureDesc = m_pSequence->Get_Desc(m_iSelectedEntry);
	}

	ImGui::End();

	return S_OK;
}

CGLTool* CGLTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CGLTool* pInstance = new CGLTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CGLTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CGLTool::Clone(void* pArg)
{
	CGLTool* pInstance = new CGLTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CGLTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CGLTool::Free()
{
	__super::Free();

	Safe_Delete(m_pSequence);

}
