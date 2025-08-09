#include "GameInstance.h"

#include "NavTool.h"
#include "Cell.h"

CNavTool::CNavTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CYWTool(pDevice, pContext)
{
}

CNavTool::CNavTool(const CNavTool& Prototype)
	:CYWTool(Prototype)
{
}

HRESULT CNavTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CNavTool::Initialize(void* pArg)
{
	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CNavTool::Priority_Update(_float fTimeDelta)
{
}

void CNavTool::Update(_float fTimeDelta)
{
	m_pNavigationCom->Update(XMMatrixIdentity());


	Control(fTimeDelta);
}

void CNavTool::Late_Update(_float fTimeDelta)
{
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_NONLIGHT, this);
}

HRESULT CNavTool::Render()
{
#ifdef _DEBUG
	m_pNavigationCom->Render();
#endif

	return S_OK;
}

HRESULT	CNavTool::Render_ImGui()
{
	Render_CellList();

	return S_OK;
}

void CNavTool::Control(_float fTimeDelta)
{
	if (GetForegroundWindow() != g_hWnd)
		return;

	//Test T
	if (m_pGameInstance->Key_Down(DIK_T))
	{
		/*if (FAILED(m_pNavigationCom->Set_Index(1)))
			return;*/
	}

	//Ctrl + S 저장
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Key_Down(DIK_S))
	{
		if (FAILED(m_pNavigationCom->Save()))
		{
			MSG_BOX("네비게이션 저장 실패");
		}
		else
		{
			MSG_BOX("네비게이션 저장 성공");
		}
	}

	//셀 선택
	if (m_pGameInstance->Mouse_Down(DIM::LBUTTON))
	{
		_float4 WorldPos = {};
		if (m_pGameInstance->Picking(&WorldPos))
		{
			m_pNavigationCom->Select_Cell(XMLoadFloat4(&WorldPos));
		}
	}

	if (ImGui::IsKeyPressed(ImGuiKey_Delete))
	{
		m_pNavigationCom->Delete_Cell();
	}

	//Ctrl + 클릭(점 찍기)
	if (m_pGameInstance->Key_Pressing(DIK_LCONTROL) && m_pGameInstance->Mouse_Down(DIM::LBUTTON))
	{
		_float4 WorldPos = {};
		if (m_pGameInstance->Picking(&WorldPos))
		{
			_float3 Point = { WorldPos.x, WorldPos.y, WorldPos.z };
			m_Points.push_back(Point);

			if (m_Points.size() == 3)
			{
				Make_Clockwise(m_Points.data());

				if (FAILED(m_pNavigationCom->Add_Cell(m_Points.data())))
					return;
				m_Points.clear();
			}
		}
	}

}

void CNavTool::Render_CellList()
{
	if (ImGui::Begin("Nav Tool"))
	{
		// 테이블로 표시 (ImGui 1.80+)
		if (ImGui::BeginTable("CellList", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Index");
			ImGui::TableSetupColumn("Neighbor Count");
			ImGui::TableHeadersRow();

			for (size_t i = 0; i < m_pNavigationCom->Get_Cells().size(); ++i)
			{
				ImGui::TableNextRow();

				// Index 셀
				ImGui::TableSetColumnIndex(0);
				bool isSelected = (m_pNavigationCom->Get_Index() == static_cast<_int>(i));
				if (ImGui::Selectable(std::to_string(i).c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
				{
					m_pNavigationCom->Set_Index(static_cast<_int>(i)); // 클릭 시 선택
				}

				// Neighbor Count 셀
				ImGui::TableSetColumnIndex(1);
				int neighborCount = 0;
				for (int n = 0; n < 3; ++n)
				{
					if (m_pNavigationCom->m_Cells[i]->Get_Neighbors()[n])
						neighborCount++;
				}
				ImGui::Text("%d", neighborCount);
			}

			ImGui::EndTable();
		}
	}
	ImGui::End();

}

void CNavTool::Make_Clockwise(_float3* Points)
{
	_vector AB =  XMLoadFloat3(&Points[1]) - XMLoadFloat3(&Points[0]);
	_vector AC = XMLoadFloat3(&Points[2]) - XMLoadFloat3(&Points[0]);
	_vector normal = XMVector3Cross(AB, AC);

	// Y축 (0,1,0) 기준 내적
	if (XMVectorGetX(XMVector3Dot(normal, { 0.f, 1.f, 0.f })) < 0)
	{
		swap(Points[1], Points[2]);
	}
}

HRESULT CNavTool::Ready_Components()
{
	/* For.Com_Navigation */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::YW), TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom))))
		return E_FAIL;

	return S_OK;
}


CNavTool* CNavTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CNavTool* pInstance = new CNavTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CNavTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CNavTool::Clone(void* pArg)
{
	CNavTool* pInstance = new CNavTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CNavTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CNavTool::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);

}
