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
	if (FAILED(Render_HiTool()))
		return E_FAIL;

	if (FAILED(Render_Hi2Tool()))
		return E_FAIL;

	return S_OK;
}

HRESULT CDHTool::Render_HiTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	_bool open = true;
	Begin("DH Tools", &open, NULL);

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

HRESULT CDHTool::Render_Hi2Tool()
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

}
