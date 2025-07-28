
#include "CYTool.h"
#include "GameInstance.h"
#include "EffectSequence.h"
#include "ToolParticle.h"
#include "ToolSprite.h"
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
	
	if (FAILED(SequenceWindow()))
		return E_FAIL;

	if (FAILED(Edit_Preferences()))
		return E_FAIL;

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

HRESULT CCYTool::Make_Particles()
{
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
		case Client::CCYTool::EFF_SPRITE:
		{
			CToolSprite::DESC desc = {};
			desc.bAnimation = true;
			desc.fRotationPerSec = 0.f;
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 8;
			desc.iTileY = 8;
			desc.eShaderPass = EFF_UVSPRITE_COLOR;
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
		}
			break;
		case Client::CCYTool::EFF_PARTICLE:
		{
			CToolParticle::DESC desc = {};
			desc.eShaderPass = EFF_UVSPRITE_COLOR;
			desc.fRotationPerSec = XMConvertToRadians(90.f);
			desc.fSpeedPerSec = 5.f;
			desc.iTileX = 8;
			desc.iTileY = 8;
			//desc.
			pInstance = dynamic_cast<CEffectBase*>(m_pGameInstance->Clone_Prototype(PROTOTYPE::TYPE_GAMEOBJECT, ENUM_CLASS(LEVEL::CY), TEXT("Prototype_GameObject_ToolSprite"), &desc));
		}

			break;
		case Client::CCYTool::EFF_MESH:
			break;
		case Client::CCYTool::EFF_TRAIL:
			break;
		}
		if (pInstance != nullptr)
			m_pSequence->Add(m_strSeqItemName, pInstance, m_eEffectType, m_iSeqItemColor);
	}

	ImSequencer::Sequencer(m_pSequence, &m_iCurFrame, &m_bExpanded, &m_iSelected, &m_iFirstFrame, ImSequencer::SEQUENCER_EDIT_ALL);

	ImGui::End();
	return S_OK;
}

HRESULT CCYTool::Edit_Preferences()
{
	ImGui::Begin("Edit Item Preferences");
	if (m_pSequence == nullptr || m_pSequence->m_Items.empty() || m_iSelected == -1)
	{
		ImGui::End();
		return S_OK;
	}
	auto pEffect = m_pSequence->m_Items[m_iSelected].pEffect;

	ImGui::Text("StartTrackPos: %d", *pEffect->Get_StartTrackPosition_Ptr());
	ImGui::Text("EndTrackPos: %d", *pEffect->Get_EndTrackPosition_Ptr());
	ImGui::Text("Duration: %d", *pEffect->Get_Duration_Ptr());
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::Checkbox("Billboard", pEffect->Get_Billboard_Ptr());
	ImGui::Checkbox("Animation", pEffect->Get_Animation_Ptr());
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
	if (FAILED(Draw_TextureBrowser(pEffect)))
	{
		MSG_BOX("Failed to draw texture browser");
		ImGui::End();
		return E_FAIL;
	}
	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	// 텍스쳐 선택, UV 칸 수 조절 등

	ImGui::Separator();
	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	switch (m_pSequence->m_Items[m_iSelected].iType)
	{
	case Client::CCYTool::EFF_SPRITE:
		if (FAILED(Window_Sprite()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	case Client::CCYTool::EFF_PARTICLE:
		if (FAILED(Window_Particle()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	case Client::CCYTool::EFF_MESH:
		if (FAILED(Window_Mesh()))
		{
			ImGui::End();
			return E_FAIL;
		}
		break;
	}
	ImGui::SameLine();


	ImGui::End();


	return S_OK;
}

HRESULT CCYTool::Window_Sprite()
{
	CToolSprite* pTS = dynamic_cast<CToolSprite*>(m_pSequence->m_Items[m_iSelected].pEffect);


	auto& TSKeyFrames = pTS->Get_KeyFrames();

	_int iIdx = {};
	for (auto& Keyframe : TSKeyFrames)
	{
		ImGui::DragFloat(string("Frame" + to_string(iIdx)).c_str(), &Keyframe.fTrackPosition, 1.f, 0.f, static_cast<_float>(*pTS->Get_Duration_Ptr()/* + *m_pSequence->m_Items[m_iSelected].iStart*/), "%.0f");

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
			Keyframe.eInterpolationType = static_cast<CEffectBase::INTERPOLATION>(iSelected);
			//pTS->Set_InterpolationType(iIdx, static_cast<CEffectBase::INTERPOLATION>(iSelected));
			//:3
		}

		++iIdx;
		ImGui::Separator();
	}
	if (ImGui::Button("Add Keyframe"))
	{
		pTS->Add_KeyFrame(CEffectBase::EFFKEYFRAME{});
	} 
	if (iIdx > 1 && ImGui::Button("Delete Keyframe"))
	{
		pTS->Delete_KeyFrame();
	}


	//ImGui::Dummy(ImVec2(0.0f, 10.0f));
	//ImGui::SeparatorText("Sprite Preferences");
	//ImGui::Dummy(ImVec2(0.0f, 10.0f));
	//ImGui::PushItemWidth(100);
	//ImGui::Text("U Count");
	//ImGui::SameLine();
	//ImGui::InputInt("##Grid_U", &m_iGridTileX);
	//ImGui::SameLine();
	//ImGui::Text("V Count");
	//ImGui::SameLine();
	//ImGui::InputInt("##Grid_V", &m_iGridTileY);
	//ImGui::PopItemWidth();

	//ImGui::Checkbox("Animation", &m_bAnimateSprite);


	return S_OK;
}

HRESULT CCYTool::Window_Particle()
{
	CToolSprite* pTS = dynamic_cast<CToolSprite*>(m_pSequence->m_Items[m_iSelected].pEffect);

	ImGui::DragInt("Num Instance", &m_iNumInstance, 1, 1, 1000, "%d");
	ImGui::DragFloat3("Pivot", reinterpret_cast<_float*>(&m_vPivot), 0.1f, -1000.f, 1000.f, "%.1f");
	ImGui::DragFloat2("LifeTime", reinterpret_cast<_float*>(&m_vLifeTime), 0.1f, 0.f, 100.f, "%.1f");
	ImGui::DragFloat2("Speed", reinterpret_cast<_float*>(&m_vSpeed), 0.1f, 1.f, 1000.f, "%.1f");
	ImGui::DragFloat3("Range", reinterpret_cast<_float*>(&m_vRange), 0.1f, 1.f, 1000.f, "%.1f");
	ImGui::DragFloat2("Size", reinterpret_cast<_float*>(&m_vSize), 0.1f, 1.f, 1000.f, "%.1f");
	ImGui::DragFloat3("Center", reinterpret_cast<_float*>(&m_vCenter), 0.1f, -1000.f, 1000.f, "%.1f");
	ImGui::Checkbox("Gravity", &m_bGravity);
	ImGui::Checkbox("Orbit Pivot", &m_bOrbit);
	ImGui::DragFloat("Rotation Speed", &m_fRotationSpeed, 1.f, -360.f, 360.f, "%.1f");

	// Gravity
	// Accel, Decel
	// Rotation Speed
	// Loop





	return S_OK;
}

HRESULT CCYTool::Window_Mesh()
{
	return S_OK;
}

HRESULT CCYTool::Save_Particles()
{
	return S_OK;
}

HRESULT CCYTool::Load_Particles()
{
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

HRESULT CCYTool::Draw_TextureBrowser(CEffectBase* pEffect)
{
	ImGui::Text("Select a Texture:");
	int columnCount = 4; // 한 줄에 몇 개 보여줄지

	ImGui::BeginChild("TextureGrid", ImVec2(0, 300), true); // 스크롤 영역
	ImGui::Columns(columnCount, nullptr, false);

	for (int i = 0; i < m_Textures.size(); ++i) {
		ImGui::PushID(i);

		// 텍스처 미리보기 이미지
		if (ImGui::ImageButton("SelectTexture", reinterpret_cast<ImTextureID>(m_Textures[i].pSRV), ImVec2(64, 64))) {
			m_iSelectedTextureIdx = i;
			if (FAILED(pEffect->Change_Texture(StringToWString(m_Textures[i].name))))
				return E_FAIL;
		}

		// 텍스트 이름
		ImGui::TextWrapped("%s", m_Textures[i].name.c_str());

		ImGui::NextColumn();
		ImGui::PopID();
	}

	ImGui::Columns(1);
	ImGui::EndChild();

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
	//for (auto& tex : m_Textures)
	//{
	//	Safe_Release(const_cast<ID3D11ShaderResourceView*&>(tex.pSRV));
	//}
	m_Textures.clear();
}
 