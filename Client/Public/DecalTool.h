#pragma once
#include "YWTool.h"

#include "Client_Defines.h"

NS_BEGIN(Client)

class CDecalTool final : public CYWTool
{
private:
	CDecalTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDecalTool(const CDecalTool& Prototype);
	virtual ~CDecalTool() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta)override;
	virtual void Update(_float fTimeDelta)override;
	virtual void Late_Update(_float fTimeDelta)override;
	virtual HRESULT Render()override;
	virtual HRESULT	Render_ImGui() override;
	virtual HRESULT Save(const _char* Map) override; //void Arg�� �ٲ����� ���� ���
	virtual HRESULT Load(const _char* Map) override;

private:
	//�� ����
	void Control(_float fTimeDelta);
private:
	void Select_Decal();
	void Focus();
	void SnapTo();
	void Duplicate();

private:
	void Clear_All_Decal();
	HRESULT Ready_Texture(const _char* Map);


private:

	//���� ����� ��Į ������Ʈ�� ������
	class CDecalToolObject* Get_ClosestDecalObject(_fvector vPosition);

private:
	HRESULT Spawn_MonsterToolObject();
	void Delete_FocusObject();
private:
	void Render_Detail();
	void Detail_Transform();
	void Detail_Texture();
	void Detail_RenderMode();

private:
	ImGuizmo::OPERATION m_currentOperation = { ImGuizmo::TRANSLATE };

private:
	class CDecalToolObject* m_pFocusObject = { nullptr };


public:
	static CDecalTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg = nullptr);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END