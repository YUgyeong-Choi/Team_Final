#include "AnimTool.h"
#include "GameInstance.h"

//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감

CAnimTool::CAnimTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject{ pDevice, pContext }
{

}

CAnimTool::CAnimTool(const CAnimTool& Prototype)
	: CGameObject(Prototype)
{
}

HRESULT CAnimTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CAnimTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;


	return S_OK;
}

void CAnimTool::Priority_Update(_float fTimeDelta)
{

}

void CAnimTool::Update(_float fTimeDelta)
{
}

void CAnimTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CAnimTool::Render()
{
	if (FAILED(Render_HiTool()))
		return E_FAIL;

	if (FAILED(Render_Hi2Tool()))
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

}
