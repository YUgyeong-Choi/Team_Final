#include "AnimTool.h"
#include "Animator.h"
#include "Animation.h"
#include "AnimController.h"
#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CAnimTool::CAnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
	, m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);	
}

CAnimTool::CAnimTool(const CAnimTool& Prototype)
	: CGameObject(Prototype)
	, m_pGameInstance(CGameInstance::Get_Instance())
{
	Safe_AddRef(m_pGameInstance);
}

HRESULT CAnimTool::Initialize_Prototype()
{

	return S_OK;
}

HRESULT CAnimTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Shader_Com"), reinterpret_cast<CComponent**>(&m_pAnimShader))))
		return E_FAIL;

	_matrix ModelWorldMatrix = XMMatrixIdentity();
	XMStoreFloat4x4(&m_ModelWorldMatrix, ModelWorldMatrix);

	
	m_pTransformCom->Scaling(_float3(0.1f, 0.1f, 0.1f));
	return S_OK;
}

void CAnimTool::Priority_Update(_float fTimeDelta)
{

}

void CAnimTool::Update(_float fTimeDelta)
{
	UpdateCurrentModel(fTimeDelta);
}

void CAnimTool::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONBLEND, this);
	m_bRenerLevel = false;
}

HRESULT CAnimTool::Render()
{
	//if (FAILED(Render_HiTool()))
	//	return E_FAIL;

	//if (FAILED(Render_Hi2Tool()))
	//	return E_FAIL;

	if (m_bRenerLevel)
	{

	if (FAILED(Render_Load_Model()))
			return E_FAIL;
	}
	else
	{
		m_bRenerLevel = true;
	}

	if (FAILED(Bind_Shader()))
		return E_FAIL;


	return S_OK;
}

HRESULT CAnimTool::Render_HiTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("JW Tools", &open, NULL);

	IGFD::FileDialogConfig config;
	if (Button(u8"안녕"))
	{

	}

	if (IFILEDIALOG->Display("FBXDialog"))
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

HRESULT CAnimTool::Render_Hi2Tool()
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

HRESULT CAnimTool::Render_Load_Model()
{

	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("Load Model", &open, NULL);
	ImGui::Checkbox("Load Model", &m_bActiveLoadModel);
	if (!m_bActiveLoadModel)
	{
		ImGui::End();
		return S_OK;
	}

	IGFD::FileDialogConfig config;
	if (Button(u8"파일 경로"))
	{
		config.path = R"(../Bin/Resources/Models/Bin_Anim)";
		config.countSelectionMax = 0; // 하나만 선택 가능
		IFILEDIALOG->OpenDialog("Model Dialog", "Select Model File", ".bin", config);
	}

	if (IFILEDIALOG->Display("Model Dialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
			if (!selections.empty())
			{
				for (const auto& FilePath : selections)
				{
					auto pModel = CModel::Create(m_pDevice, m_pContext, MODEL::ANIM, FilePath.second.c_str());
					if (pModel)
					{
						string modelName = FilePath.first.find(".bin") != string::npos ?
							FilePath.first.substr(0, FilePath.first.find(".bin")) :
							FilePath.first;
						m_LoadedModels[modelName] = pModel;
						auto pAnimations = pModel->GetAnimations();
						m_LoadedAnimations[modelName] = pAnimations;

						auto pAnimator = CAnimator::Create(m_pDevice,m_pContext);
						if (pAnimator)
						{
							pAnimator->Initialize(pModel);
							m_LoadedAnimators[modelName] = pAnimator;
						}
					}
				}
			}
		}
		IFILEDIALOG->Close();
	}

	ImGui::End();

	if (FAILED(Render_Loaded_Models()))
		return E_FAIL;

	return S_OK;
}

HRESULT CAnimTool::Bind_Shader()
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
		return S_OK;

	if (m_pAnimShader == nullptr)
		return E_FAIL;
	if (FAILED(m_pAnimShader->Bind_Matrix("g_WorldMatrix", m_pTransformCom->Get_WorldMatrix_Ptr())))
		return E_FAIL;

	_float4x4 ViewMatrix, ProjViewMatrix;
	XMStoreFloat4x4(&ViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4(&ProjViewMatrix, m_pGameInstance->Get_Transform_Matrix(D3DTS::PROJ));
	if (FAILED(m_pAnimShader->Bind_Matrix("g_ViewMatrix", &ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pAnimShader->Bind_Matrix("g_ProjMatrix", &ProjViewMatrix)))
		return E_FAIL;

	_uint		iNumMesh = m_pCurModel->Get_NumMeshes();

	for (_uint i = 0; i < iNumMesh; i++)
	{
		if (FAILED(m_pCurModel->Bind_Material(m_pAnimShader, "g_DiffuseTexture", i, aiTextureType_DIFFUSE, 0)))
			return E_FAIL;

		m_pCurModel->Bind_Bone_Matrices(m_pAnimShader, "g_BoneMatrices", i);

		if (FAILED(m_pAnimShader->Begin(0)))
			return E_FAIL;

		if (FAILED(m_pCurModel->Render(i)))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CAnimTool::Render_Loaded_Models()
{
	if (m_LoadedModels.empty())
		return S_OK;

	static _int iSelectedModelIndex = -1;
	ImGui::Begin("Loaded Models");

	vector<string> modelNames;
	for (const auto& pair : m_LoadedModels)
		modelNames.push_back(pair.first);

	if (ImGui::BeginCombo("Models", iSelectedModelIndex >= 0 ? modelNames[iSelectedModelIndex].c_str() : "Select Model"))
	{
		for (_int i = 0; i < modelNames.size(); ++i)
		{
			_bool isSelected = (i == iSelectedModelIndex);
			if (ImGui::Selectable(modelNames[i].c_str(), isSelected))
			{
				iSelectedModelIndex = i;
				m_pCurModel = m_LoadedModels[modelNames[i]];
				m_pCurAnimator = m_LoadedAnimators[modelNames[i]];
				m_stSelectedModelName = modelNames[i];
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// 선택된 모델의 애니메이션들
	SelectAnimation();
	ImGui::End();
	return S_OK;
}

void CAnimTool::UpdateCurrentModel(_float fTimeDelta)
{
	if (m_pCurAnimator)
	{
		m_pCurAnimator->GetAnimController()->Update(fTimeDelta);
	}
	if (m_pCurModel)
	{
		m_pCurModel->Play_Animation(fTimeDelta);
	}
}

void CAnimTool::SelectAnimation()
{
	if (m_pCurModel == nullptr || m_pCurAnimator == nullptr)
		return;
	static _int iSelectedAnimIndex = -1;
	ImGui::Begin("Select Animation");

	vector<CAnimation*> anims = m_LoadedAnimations[m_stSelectedModelName]; // 현재 선택된 모델의 애니메이션들

	vector<string> animIndices;
	for (_uint i = 0; i < anims.size(); i++)
	{
		animIndices.push_back(to_string(i));
	}

	if (ImGui::BeginCombo("Animations", iSelectedAnimIndex >= 0 ? animIndices[iSelectedAnimIndex].c_str() : "Select Animation"))
	{
		for (_int i = 0; i < animIndices.size(); ++i)
		{
			_bool isSelected = (i == iSelectedAnimIndex);
			if (ImGui::Selectable(animIndices[i].c_str(), isSelected))
			{
				iSelectedAnimIndex = i;
				m_pCurAnimation = anims[iSelectedAnimIndex];
		        m_pCurAnimator->PlayClip(anims[iSelectedAnimIndex]);
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	if (m_pGameInstance->Key_Down(DIK_COMMA)) 
	{
		iSelectedAnimIndex--;
		if (iSelectedAnimIndex < 0) 
			iSelectedAnimIndex = static_cast<_int>(anims.size()) - 1; // 마지막으로 순환
			m_pCurAnimator->PlayClip(anims[iSelectedAnimIndex]);
		m_pCurAnimation = anims[iSelectedAnimIndex];
	}
	if (m_pGameInstance->Key_Down(DIK_PERIOD)) 
	{
		iSelectedAnimIndex++;
		if (iSelectedAnimIndex >= static_cast<_int>(anims.size())) 
			iSelectedAnimIndex = 0;
		m_pCurAnimator->PlayClip(anims[iSelectedAnimIndex]);
		m_pCurAnimation = anims[iSelectedAnimIndex];
	}

	ImGui::End();
}

void CAnimTool::Manipulate(Operation op, const _float snapT[3], const _float snapR[3], const _float snapS[3])
{
	if (!m_pTransformCom) return;

	ImGuizmo::BeginFrame();
	auto& io = ImGui::GetIO();
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	// 뷰·프로젝션·월드 매트릭스 float[16] 준비
	_float matV[16], matP[16], matW[16];
	XMStoreFloat4x4((XMFLOAT4X4*)matV, CGameInstance::Get_Instance()->Get_Transform_Matrix(D3DTS::VIEW));
	XMStoreFloat4x4((XMFLOAT4X4*)matP,CGameInstance::Get_Instance()->Get_Transform_Matrix(D3DTS::PROJ));

	_matrix xmW = XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrix_Ptr());
	XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(matW), xmW);

	// 조작 모드 결정
	ImGuizmo::OPERATION gizOp =
		op == Operation::TRANSLATE ? ImGuizmo::TRANSLATE :
		op == Operation::ROTATE ? ImGuizmo::ROTATE :
		ImGuizmo::SCALE;

	// Shift 누르고 있을 때만 스냅 적용
	const _float* snapPtr = nullptr;
	if (ImGui::IsKeyDown(ImGuiMod_Shift))
	{
		switch (op)
		{
		case Operation::TRANSLATE: snapPtr = snapT; break;
		case Operation::ROTATE:    snapPtr = snapR; break;
		case Operation::SCALE:     snapPtr = snapS; break;
		}
	}

	// Manipulate에 스냅 배열 전달
	ImGuizmo::Manipulate(matV, matP, gizOp, ImGuizmo::WORLD, matW, nullptr, snapPtr);

	//// 드래그 중이면 WorldMatrix 업데이트
	//if (ImGuizmo::IsUsing())
	//{
	//	XMFLOAT4X4 newW;
	//	memcpy(&newW, matW, sizeof(newW));
	//	m_pTransformCom->Set_WorldMatrix(newW);

	//	if (op == Operation::ROTATE)
	//	{
	//		pTransform->UpdateEulerAngles();
	//	}
	//}

}

CAnimTool* CAnimTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CAnimTool* pInstance = new CAnimTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CAnimTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CAnimTool::Clone(void* pArg)
{
	CAnimTool* pInstance = new CAnimTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CAnimTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CAnimTool::Free()
{
	__super::Free();
	Safe_Release(m_pGameInstance);
}
