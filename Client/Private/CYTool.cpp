#include "CYTool.h"
#include "GameInstance.h"
#include "EffectSequence.h"
#include "ToolParticle.h"
#include "ToolSprite.h"
#include "ToolMeshEffect.h"
#include "Client_Calculation.h"


//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CCYTool::CCYTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CCYTool::CCYTool(const CCYTool& Prototype)
	: CGameObject(Prototype)
{

}

HRESULT CCYTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CCYTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	m_pSequence = new CEffectSequence();

	Load_Textures();

	return S_OK;
}

void CCYTool::Priority_Update(_float fTimeDelta)
{
	Key_Input();
	if (m_bPlaySequence)
	{
		m_fCurFrame += m_fTickPerSecond * fTimeDelta;
		m_iCurFrame = static_cast<_int>(m_fCurFrame);
	}
	else
	{
		m_fCurFrame = static_cast<_float>(m_iCurFrame);

	}
	if (m_fCurFrame > m_pSequence->GetFrameMax())
	{
		m_fCurFrame = static_cast<_float>(m_pSequence->GetFrameMin());
		for (auto& pItem : m_pSequence->m_Items)
		{
			pItem.pEffect->Reset_TrackPosition();
		}
	}
	for (auto& pItem : m_pSequence->m_Items)
	{
		if (m_iCurFrame >= *pItem.iStart && m_iCurFrame <= *pItem.iEnd)
		{
			pItem.pEffect->Priority_Update(fTimeDelta);
			// 이펙트를 재생
			//PlaySpriteEffect(pItem.iType, m_iCurFrame - pItem.iStart);
		}
	}
}

void CCYTool::Update(_float fTimeDelta)
{
	//if (m_bPlaySequence)
	{
		for (auto& pItem : m_pSequence->m_Items)
		{
			if (m_iCurFrame >= *pItem.iStart && m_fCurFrame <= *pItem.iEnd)
			{
				pItem.pEffect->Update_Tool(fTimeDelta, static_cast<_float>(m_iCurFrame - *pItem.iStart));
				// 이펙트를 재생
				//PlaySpriteEffect(pItem.iType, m_fCurFrame - pItem.iStart);
			}
		}
	}
}

void CCYTool::Late_Update(_float fTimeDelta)
{
	for (auto& pItem : m_pSequence->m_Items)
	{
		if (m_iCurFrame >= *pItem.iStart && m_iCurFrame <= *pItem.iEnd)
		{
			pItem.pEffect->Late_Update(fTimeDelta);
		}
	}
}

HRESULT CCYTool::Render()
{
	//if (FAILED(Render_EffectTool()))
	//	return E_FAIL;
	//ImGui::ShowDemoWindow(); // Show demo window! :)

	if (FAILED(SequenceWindow()))
		return E_FAIL;

	if (FAILED(Edit_Preferences()))
		return E_FAIL;

	if (m_bOpenSaveEffectOnly)
		Save_Effect();
	if (m_bOpenLoadEffectOnly)
		Load_Effect();

	return S_OK;
}

HRESULT CCYTool::Render_EffectTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("CY Tools", &open, NULL);

	IGFD::FileDialogConfig config;
	if (Button(u8"안녕"))
	{
		IFILEDIALOG->OpenDialog("Effect Tool Dialog", "Select File", nullptr, config);
	}

	if (IFILEDIALOG->Display("Effect Tool Dialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{

				}
			}
		}
		IFILEDIALOG->Close();
	}

	ImGui::End();
	return S_OK;
}

HRESULT CCYTool::SequenceWindow()
{
	ImGui::Begin("ImSequence Window");
	if (ImGui::RadioButton("Sprite Effect", m_eEffectType == EFF_SPRITE)) {
		m_eEffectType = EFF_SPRITE;
		m_strSeqItemName = "Sprite";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 200, 60, 40);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Particle Effect", m_eEffectType == EFF_PARTICLE)) {
		m_eEffectType = EFF_PARTICLE;
		m_strSeqItemName = "Particle";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 60, 200, 80);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Mesh Effect", m_eEffectType == EFF_MESH)) {
		m_eEffectType = EFF_MESH;
		m_strSeqItemName = "Mesh";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 100, 100, 220);
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("Trail Effect", m_eEffectType == EFF_TRAIL)) {
		m_eEffectType = EFF_TRAIL;
		m_strSeqItemName = "Trail";
		m_iSeqItemColor = D3DCOLOR_ARGB(255, 170, 80, 250);
	}
	ImGui::SameLine();
	if (ImGui::Button("Add to Sequence"))
	{
		CEffectBase* pInstance = { nullptr };
		switch (m_eEffectType)
		{
		case Client::EFF_SPRITE:
		{
			CToolSprite::DESC desc = {};
			desc.bAnimation = true;
			desc.fRotationPerSec = 0.f;
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 8;
			desc.iTileY = 8;
			desc.iShaderPass = ENUM_CLASS(SE_UVSPRITE_COLOR);
			desc.bTool = true;
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
		}
			break;
		case Client::EFF_PARTICLE:
		{
			CToolParticle::DESC desc = {};
			desc.iShaderPass = ENUM_CLASS(SE_UVSPRITE_COLOR);
			desc.fRotationPerSec = XMConvertToRadians(90.f);
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 8;
			desc.iTileY = 8;
			desc.ePType = m_eParticleType;
			desc.iNumInstance = m_iNumInstance;
			desc.isLoop = m_isLoop;
			desc.vCenter = m_vCenter;
			desc.vLifeTime = m_vLifeTime;
			desc.vPivot = m_vPivot;
			desc.vRange = m_vRange;
			desc.vSize = m_vSize;
			desc.vSpeed = m_vSpeed;
			desc.bBillboard = false;
			desc.bTool = true;
			desc.iShaderPass = ENUM_CLASS(PE_DEFAULT);
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"), &desc));
		}
			break;
		case Client::EFF_MESH:
		{
			CToolMeshEffect::DESC desc = {};
			desc.bAnimation = true;
			desc.fRotationPerSec = 0.f;
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 4;
			desc.iTileY = 1;
			desc.bBillboard = false;
			desc.bTool = true;
			desc.iShaderPass = ENUM_CLASS(ME_MASKONLY);
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
				PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"), &desc));
		}
			break;
		case Client::EFF_TRAIL:
			break;
		}
		if (pInstance != nullptr)
			m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
	}
	ImGui::SameLine();
	if (ImGui::Button("Load EffectContainer"))
	{

	}

	ImSequencer::Sequencer(
		m_pSequence,
		&m_iCurFrame,
		&m_bExpanded,
		&m_iSelected,
		&m_iFirstFrame,
		ImSequencer::SEQUENCER_EDIT_ALL);

	ImGui::End();
	return S_OK;
}

HRESULT CCYTool::Edit_Preferences()
{
	ImGui::Begin("Edit Item Preferences");
	if (ImGui::Button("Load Effect"))
	{
		m_bOpenLoadEffectOnly = true;
	}
	ImGui::SameLine();
	if (m_pSequence == nullptr || m_pSequence->m_Items.empty() || m_iSelected == -1)
	{
		ImGui::End();
		return S_OK;
	}
	auto pEffect = m_pSequence->m_Items[m_iSelected].pEffect;
	
	if (ImGui::Button("Save Selected Effect"))
	{
		m_bOpenSaveEffectOnly = true;
	}

	ImGui::Text("StartTrackPos: %d", *pEffect->Get_StartTrackPosition_Ptr());
	ImGui::Text("EndTrackPos: %d", *pEffect->Get_EndTrackPosition_Ptr());
	ImGui::Text("Duration: %d", *pEffect->Get_Duration_Ptr());
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::Checkbox("Billboard", pEffect->Get_Billboard_Ptr());
	ImGui::Checkbox("Animation", pEffect->Get_Animation_Ptr());
	ImGui::Checkbox("Flip UV", pEffect->Get_FlipUV_Ptr());

	ImGui::PushItemWidth(100);
	ImGui::Text("Tile X");
	ImGui::SameLine();
	ImGui::InputInt("##Tile X", pEffect->Get_TileX());
	ImGui::SameLine();
	ImGui::Text("Tile Y");
	ImGui::SameLine();
	ImGui::InputInt("##Tile Y", pEffect->Get_TileY());
	ImGui::PopItemWidth();
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// 텍스쳐 선택 창
	if (FAILED(Draw_TextureBrowser(pEffect)))
	{
		MSG_BOX("Failed to draw texture browser");
		ImGui::End();
		return E_FAIL;
	}



	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// 키프레임 
	Edit_Keyframes(pEffect);

	// 텍스쳐 선택, UV 칸 수 조절 등

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	switch (m_pSequence->m_Items[m_iSelected].iType)
	{
	case Client::EFF_SPRITE:
		if (FAILED(Window_Sprite()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	case Client::EFF_PARTICLE:
		if (FAILED(Window_Particle()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	case Client::EFF_MESH:
		if (FAILED(Window_Mesh()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	}
	//ImGui::SameLine();




	ImGui::End();


	return S_OK;
}

HRESULT CCYTool::Window_Sprite()
{
	CToolSprite* pSE = dynamic_cast<CToolSprite*>(m_pSequence->m_Items[m_iSelected].pEffect);

	ImGui::Text("Select Pass\n0. Default\t1. SoftEffect\t2. UVSprite\t3. UVSprite_Color");

	for (_uint i = 0; i < SE_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##SE").c_str(), m_eSelectedPass_SE == i)) {
			m_eSelectedPass_SE = (SPRITEEFFECT_PASS_INDEX)i;
			pSE->Set_ShaderPass(i);
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));


	return S_OK;
}

HRESULT CCYTool::Window_Particle()
{
	CToolParticle* pPE = dynamic_cast<CToolParticle*>(m_pSequence->m_Items[m_iSelected].pEffect);

	ImGui::Text("Select Pass\n0. Default");
	for (_uint i = 0; i < PE_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##PE").c_str(), m_eSelectedPass_PE == i)) {
			m_eSelectedPass_PE = (PARTICLEEFFECT_PASS_INDEX)i;
			pPE->Set_ShaderPass(i);
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	ImGui::DragInt("Num Instance", &m_iNumInstance, 1, 1, 1000, "%d");
	ImGui::DragFloat3("Pivot", reinterpret_cast<_float*>(&m_vPivot), 0.1f, -1000.f, 1000.f, "%.1f");
	ImGui::DragFloat2("LifeTime", reinterpret_cast<_float*>(&m_vLifeTime), 0.1f, 0.f, 100.f, "%.1f");
	ImGui::DragFloat2("Speed", reinterpret_cast<_float*>(&m_vSpeed), 0.1f, 1.f, 1000.f, "%.1f");
	ImGui::DragFloat3("Range", reinterpret_cast<_float*>(&m_vRange), 0.1f, 1.f, 1000.f, "%.1f");
	ImGui::DragFloat2("Size", reinterpret_cast<_float*>(&m_vSize), 0.1f, 1.f, 1000.f, "%.1f");
	ImGui::DragFloat3("Center", reinterpret_cast<_float*>(&m_vCenter), 0.1f, -1000.f, 1000.f, "%.1f");
	// 아직안만듦
	ImGui::Checkbox("Gravity", &m_bGravity);
	ImGui::Checkbox("Orbit Pivot", &m_bOrbit);
	ImGui::DragFloat("Rotation Speed", &m_fRotationSpeed, 1.f, -360.f, 360.f, "%.1f");

	if (ImGui::RadioButton("Explosion", m_eParticleType == PTYPE_SPREAD)) {
		m_eParticleType = PTYPE_SPREAD;
	}
	if (ImGui::RadioButton("Drop", m_eParticleType == PTYPE_DROP)) {
		m_eParticleType = PTYPE_DROP;
	}


	if(ImGui::Button("Update Particle"))
	{
		CVIBuffer_Point_Instance::DESC desc = {};
		desc.ePType = m_eParticleType;
		desc.iNumInstance = m_iNumInstance;
		desc.isLoop = m_isLoop;
		desc.vCenter = m_vCenter;
		desc.vLifeTime = m_vLifeTime;
		desc.vPivot = m_vPivot;
		desc.vRange = m_vRange;
		desc.vSize = m_vSize;
		desc.vSpeed = m_vSpeed;
		desc.isTool = true;
		pPE->Change_InstanceBuffer(&desc);
	}

	// Gravity
	// Accel, Decel
	// Random? <- 랜덤한 방향을 잡을 인터벌, 랜덤한 방향각도 최대치 설정 필요
	// Rotation Speed
	// Loop

	return S_OK;
}

HRESULT CCYTool::Window_Mesh()
{
	CToolMeshEffect* pME = dynamic_cast<CToolMeshEffect*>(m_pSequence->m_Items[m_iSelected].pEffect);
	if (pME == nullptr)
		return E_FAIL;

	ImGui::Text("Select Pass\n0. Default\t1. Mask only\t2. Mask Noise\t3. UVMask");
	for (_uint i = 0; i < ME_END; i++)
	{
		if (ImGui::RadioButton((to_string(i) + "##ME").c_str(), m_eSelectedPass_ME == i)) {
			m_eSelectedPass_ME = (MESHEFFECT_PASS_INDEX)i;
			pME->Set_ShaderPass(i);
		}
		if (i % 6 != 0 || i == 0)
			ImGui::SameLine();
	}
	ImGui::Dummy(ImVec2(0.0f, 2.0f));

	if (ImGui::Button("Load Models"))
	{
		IGFD::FileDialogConfig config;
		config.path = R"(..\Bin\Resources\Models\EffectMesh)";
		config.countSelectionMax = 0; // 무제한
		IFILEDIALOG->OpenDialog("Mesh Effect Model Select", "Select File", ".bin", config);
		
	}
	if (FAILED(Load_EffectModel()))
		return E_FAIL;
	

	if (ImGui::BeginCombo("##SelectModel", m_iSelectedModelIdx >= 0 ? m_ModelNames[m_iSelectedModelIdx].c_str() : "Preset Model"))
	{
		for (_int i = 0; i < m_ModelNames.size(); ++i)
		{
			_bool isSelected = (i == m_iSelectedModelIdx);
			if (ImGui::Selectable(m_ModelNames[i].c_str(), isSelected))
			{
				if (FAILED(pME->Change_Model(StringToWString(m_ModelNames[i]))))
				{
					MSG_BOX("모델 변경 실패 ");
					return E_FAIL;
				}
				m_iSelectedModelIdx = i;
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	//Edit_Keyframes(pME);

	return S_OK;
}

void CCYTool::Edit_Keyframes(CEffectBase* pEffect)
{
	ImGui::Begin("Edit Keyframes");
	auto& KeyFrames = pEffect->Get_KeyFrames();

	_int iIdx = {};
	for (auto& Keyframe : KeyFrames)
	{
		ImGui::DragFloat(string("Frame" + to_string(iIdx)).c_str(), &Keyframe.fTrackPosition, 1.f, 0.f, static_cast<_float>(*pEffect->Get_Duration_Ptr()/* + *m_pSequence->m_Items[m_iSelected].iStart*/), "%.0f");

		ImGui::DragFloat3(string("Translation##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&Keyframe.vTranslation), 0.1f);

		_float3 vRotAxis = QuaternionToEuler(XMLoadFloat4(&Keyframe.vRotation));
		ImGui::BeginDisabled();
		ImGui::DragFloat3(string("Rotation##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&vRotAxis));
		ImGui::EndDisabled();

		ImGui::DragFloat3(string("Scaling##" + to_string(iIdx)).c_str(), reinterpret_cast<_float*>(&Keyframe.vScale), 0.1f);

		ImGui::ColorEdit4(string("Color##" + to_string(iIdx)).c_str(), (float*)&Keyframe.vColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_None);

		_int iSelected = Keyframe.eInterpolationType;
		if (ImGui::Combo(string("##interpolation type" + to_string(iIdx)).c_str(), &iSelected, m_InterpolationTypes, IM_ARRAYSIZE(m_InterpolationTypes)))
		{
			Keyframe.eInterpolationType = static_cast<INTERPOLATION>(iSelected);
			//pTS->Set_InterpolationType(iIdx, static_cast<CEffectBase::INTERPOLATION>(iSelected));
			//:3
			//:>
		}
		++iIdx;
		ImGui::Separator();
	}
	if (ImGui::Button("Add Keyframe"))
	{
		pEffect->Add_KeyFrame(CEffectBase::EFFKEYFRAME{});
	}
	if (iIdx > 1 && ImGui::Button("Delete Keyframe"))
	{
		pEffect->Delete_KeyFrame();
	}
	ImGui::End();
}

HRESULT CCYTool::Load_EffectModel()
{
	if (IFILEDIALOG->Display("Mesh Effect Model Select"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{
					Make_EffectModel_Prototypes(FilePath.second);
				}
			}
		}
		IFILEDIALOG->Close();
	}

	return S_OK;
}

HRESULT CCYTool::Make_EffectModel_Prototypes(const string strModelFilePath)
{
	path ModelPath = strModelFilePath;
	path ModelName = ModelPath.stem();
	_wstring strModelTag = ModelName.wstring();

	_matrix		PreTransformMatrix = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), _wstring(L"Prototype_Component_Model_") + strModelTag,
		CModel::Create(m_pDevice, m_pContext, MODEL::NONANIM, strModelFilePath.c_str(), PreTransformMatrix))))
		return E_FAIL;

	m_ModelNames.push_back(ModelName.string());
	return S_OK;
}

HRESULT CCYTool::Save_EffectSet()
{
	path SavePath = R"(../Bin/DataFiles/Effect/EffectContainer/)";
	ofstream ofs(SavePath);
	if (!ofs.is_open())
		return E_FAIL;
	json jSave;

	auto& Items = m_pSequence->Get_Items();

	for (auto& Item : Items)
	{
		json jItem;
		jItem["Name"] = Item.strName.data();
		jItem["StartPos"] = *Item.iStart;
		jItem["EndPos"] = *Item.iEnd;
		jItem["EffectType"] = Item.iType;

		jItem.push_back(Item.pEffect->Serialize());

		jSave["EffectObject"].push_back(jItem);
	}
	//미완
	return S_OK;
}

HRESULT CCYTool::Load_EffectSet()
{
	return S_OK;
}

HRESULT CCYTool::Save_Effect()
{
	IGFD::FileDialogConfig config;
	config.path = R"(..\Bin\DataFiles\Effect\)";

	IFILEDIALOG->OpenDialog("SaveEffectonlyDialog", "Choose directory to save", ".json", config);

	if (IFILEDIALOG->Display("SaveEffectonlyDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path savePath = IFILEDIALOG->GetFilePathName();

			// 확장자가 없으면 .json 붙이기
			if (savePath.extension().string() != ".json")
				savePath += ".json";


			ofstream ofs(savePath);

			if (!ofs.is_open())
				return E_FAIL;

			json jSave = m_pSequence->m_Items[m_iSelected].pEffect->Serialize();

			ofs << setw(4) << jSave;
			ofs.close();
		}
		m_bOpenSaveEffectOnly = false;
		IFILEDIALOG->Close();
	}

	return S_OK;
}

// 이펙트 단일 로드
HRESULT CCYTool::Load_Effect()
{
	IGFD::FileDialogConfig config;
	config.path = R"(..\Bin\DataFiles\Effect\)";
	json j;
	IFILEDIALOG->OpenDialog("LoadEffectonlyDialog", "Choose File to Load", ".json", config);

	if (IFILEDIALOG->Display("LoadEffectonlyDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			path loadPath = IFILEDIALOG->GetFilePathName();
			string filename = IFILEDIALOG->GetCurrentFileName();
			string prefix = filename.substr(0, 2);
			
			// put extension '.json'
			if (loadPath.extension().string() != ".json")
				loadPath += ".json";

			// Compare Prefix for making each effects
			// Check the filenames before saving
			if (prefix == "SE"){
				m_eEffectType = EFF_SPRITE;
			}
			else if (prefix == "PE"){
				m_eEffectType = EFF_PARTICLE;
			}
			else if (prefix == "ME"){
				m_eEffectType = EFF_MESH;
			}
			else if (prefix == "TE"){
				m_eEffectType = EFF_TRAIL;
			}
			else
			{
				MSG_BOX("Filename should start with \"SE / PE / ME / TE\"");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return S_OK;
			}

			ifstream ifs(loadPath);

			if (!ifs.is_open())
			{
				MSG_BOX("File open Failed");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return E_FAIL;
			}

			ifs >> j;
			ifs.close();

			CEffectBase* pInstance = { nullptr };

			switch (m_eEffectType)
			{
			case Client::EFF_SPRITE:
			{
				CToolSprite::DESC desc = {};
				desc.bTool = true;
				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
			}
			break;
			case Client::EFF_PARTICLE:
			{
				CToolParticle::DESC desc = {};
				desc.fRotationPerSec = XMConvertToRadians(90.f);
				desc.fSpeedPerSec = 5.f;
				desc.bTool = true;
				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolParticle"), &desc));
			}
			break;
			case Client::EFF_MESH:
			{
				CToolMeshEffect::DESC desc = {};
				desc.fRotationPerSec = XMConvertToRadians(90.f);
				desc.fSpeedPerSec = 5.f;
				desc.bTool = true;
				pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(
					PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolMeshEffect"), &desc));
			}
			break;
			case Client::EFF_TRAIL:
				break;
			}
			if (pInstance != nullptr)
			{
				pInstance->Deserialize(j);
				m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
			}
			else
			{
				MSG_BOX("Failed to make Effect");
				m_bOpenLoadEffectOnly = false;
				IFILEDIALOG->Close();
				return E_FAIL;
			}
		}
		m_bOpenLoadEffectOnly = false;
		IFILEDIALOG->Close();
	}

	return S_OK;
}

HRESULT CCYTool::Load_Textures()
{
	path TexturePath = R"(../Bin/Resources/Textures/Effect/)";
	if (!exists(TexturePath))
	{
		MSG_BOX("텍스쳐 경로가 이상해요");
		return E_FAIL;
	}

	for (const auto& entry : directory_iterator(TexturePath))
	{
		if (entry.is_regular_file() && (entry.path().extension() == L".dds" || entry.path().extension() == L".png"))
		{
			_wstring filePath = entry.path().wstring();
			_wstring stemName = entry.path().stem().wstring();
			_wstring prototypeTag = L"Prototype_Component_Texture_" + stemName;
			if (FAILED(m_pGameInstance->Add_Prototype(ENUM_CLASS(LEVEL::CY), prototypeTag,
				CTexture::Create(m_pDevice, m_pContext,
					filePath.c_str(), 1))))
			{
				string msg = "파일 오픈 실패:\n대상 경로: ";

				msg += WStringToString(filePath);

				MessageBoxA(nullptr, msg.c_str(), "오류", MB_OK | MB_ICONERROR);
				continue;
			}

			string strRelativePath = filesystem::relative(entry.path(), filesystem::current_path()).string();
			m_Textures.push_back({
				static_cast<CTexture*>(m_pGameInstance->Find_Prototype(ENUM_CLASS(LEVEL::CY), prototypeTag))->Get_SRV(0),
				WStringToString(stemName)
				});
		}
	}

	return S_OK;
}

//HRESULT CCYTool::Draw_TextureBrowser(CEffectBase* pEffect)
//{
//	ImGui::Text("Select a Texture:");
//	int columnCount = 4; // 한 줄에 몇 개 보여줄지
//
//	ImGui::BeginChild("TextureGrid", ImVec2(0, 300), true); // 스크롤 영역
//	ImGui::Columns(columnCount, nullptr, false);
//
//	for (int i = 0; i < m_Textures.size(); ++i) {
//		ImGui::PushID(i);
//
//		// 텍스처 미리보기 이미지
//		if (ImGui::ImageButton("SelectTexture", reinterpret_cast<ImTextureID>(m_Textures[i].pSRV), ImVec2(64, 64))) {
//			m_iSelectedTextureIdx = i;
//			if (FAILED(pEffect->Change_Texture(StringToWString(m_Textures[i].name))))
//				return E_FAIL;
//		}
//
//		// 텍스트 이름
//		ImGui::TextWrapped("%s", m_Textures[i].name.c_str());
//
//		ImGui::NextColumn();
//		ImGui::PopID();
//	}
//
//	ImGui::Columns(1);
//	ImGui::EndChild();
//
//	return S_OK;
//}

HRESULT CCYTool::Draw_TextureBrowser(CEffectBase* pEffect)
{
	ImGui::BeginGroup();

	// 텍스처 미리보기 목록 (드래그 가능)
	ImGui::Text("Select a Texture:");
	int columnCount = 4;

	ImGui::BeginChild("TextureGrid", ImVec2(400, 300), true);
	ImGui::Columns(columnCount, nullptr, false);

	for (int i = 0; i < m_Textures.size(); ++i) {
		ImGui::PushID(i);

		// 이미지 버튼
		if (ImGui::ImageButton("##TexBtn", reinterpret_cast<ImTextureID>(m_Textures[i].pSRV), ImVec2(64, 64)))
			m_iSelectedTextureIdx = i;

		// 드래그 소스 등록
		if (ImGui::BeginDragDropSource()) {
			ImGui::SetDragDropPayload("TEXTURE_PAYLOAD", &i, sizeof(int));
			ImGui::Text("Dragging: %s", m_Textures[i].name.c_str());
			ImGui::EndDragDropSource();
		}

		ImGui::TextWrapped("%s", m_Textures[i].name.c_str());
		ImGui::NextColumn();
		ImGui::PopID();
	}

	ImGui::Columns(1);
	ImGui::EndChild();

	ImGui::EndGroup();
	ImGui::Separator();
	//ImGui::SameLine();

	// 슬롯 UI (Drop Target)
	ImGui::BeginGroup();
	ImGui::Text("Texture Slots:");
	const _char* slotNames[4] = { "Diffuse", "Mask1", "Mask2", "Mask3" };
	ImVec2 slotSize = ImVec2(48, 48);

	for (_int slotIdx = 0; slotIdx < CEffectBase::TU_END; ++slotIdx) {
		if (slotIdx > 0)
			ImGui::SameLine(); // 가로 정렬
		ImGui::BeginGroup();
		ImGui::PushID(slotIdx);

		ImGui::Text("%s", slotNames[slotIdx]);

		if (m_pSlotSRV[slotIdx])
			ImGui::Image(reinterpret_cast<ImTextureID>(m_pSlotSRV[slotIdx]), slotSize);
		else
			ImGui::Dummy(slotSize);

		// 드롭 대상 처리
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEXTURE_PAYLOAD")) {
				_int draggedIndex = *(const int*)payload->Data;

				m_pSlotSRV[slotIdx] = m_Textures[draggedIndex].pSRV;
				m_SlotTexNames[slotIdx] = m_Textures[draggedIndex].name;

				if (FAILED(pEffect->Change_Texture(StringToWString(m_SlotTexNames[slotIdx]), (CEffectBase::TEXUSAGE)slotIdx)))
					return E_FAIL;
			}
			ImGui::EndDragDropTarget();
		}

		// 이름 출력 및 Clear 버튼
		if (!m_SlotTexNames[slotIdx].empty()) {
			ImGui::TextWrapped("(%s)", m_SlotTexNames[slotIdx].c_str());
			if (ImGui::Button("Clear")) {
				m_pSlotSRV[slotIdx] = nullptr;
				m_SlotTexNames[slotIdx].clear();
				pEffect->Delete_Texture((CEffectBase::TEXUSAGE)slotIdx);
			}
		}

		//ImGui::Separator();
		ImGui::PopID();
		ImGui::EndGroup();
	}

	ImGui::EndGroup();

	return S_OK;
}


void CCYTool::Key_Input()
{
	if (KEY_DOWN(DIK_SPACE))
	{
		m_bPlaySequence = !m_bPlaySequence;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		if (m_pSequence &&
			!m_pSequence->m_Items.empty() && 
			m_iSelected != -1)
		{
			m_pSequence->Del(m_iSelected);
			m_iSelected = - 1;
		}
	}
}

CCYTool* CCYTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CCYTool* pInstance = new CCYTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CCYTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CCYTool::Clone(void* pArg)
{
	CCYTool* pInstance = new CCYTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CCYTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CCYTool::Free()
{
	__super::Free();

	Safe_Delete(m_pSequence);

	m_Textures.clear();
}
 