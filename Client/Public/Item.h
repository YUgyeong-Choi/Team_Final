#pragma once

#include "Client_Defines.h"
#include "GameObject.h"


NS_BEGIN(Client)

class CItem abstract : public CGameObject
{
protected:
	CItem(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CItem(const CItem& Prototype);
	virtual ~CItem() = default;

public:
	const _wstring& Get_ProtoTag() { return m_strProtoTag; }
	_bool& Get_isRender() { return m_isRender; }
	void Set_isRender(_bool isRender) { m_isRender = isRender; }
	_bool& Get_isUsable() { return m_isUsable; }
	_bool& Get_isConsumable() { return m_isConsumable; }
	_int& Get_UseCount() { return m_iUseCount; }
public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	// ��� ȿ�� ��ӹ޾Ƽ� �����ϱ�
	// �ִϸ��̼� ������ �� true, ������ false
	virtual void Activate(_bool isActive) = 0;

	virtual ITEM_DESC Get_ItemDesc() { return ITEM_DESC(); };

protected:
	// �̰ŷ� �����ϴ��� �ƴϸ� �ٸ��ŷ� �����ϴ���
	// �׳� enum�ؼ� �ϴ°� ��������?

	_wstring m_strProtoTag = {};
	// UI ǥ�� �뵵
	// ������ �ҰŴ� ���� �ִ�
	// �׻� ���̴°Ŵ� �׻� true��? (���� ������)
	_bool    m_isRender = {	false };
	// ��� �������� �Ұ�������
	_bool    m_isUsable = { true };
	// �Ҹ�ǰ���� �ƴ���, �Ҹ�ǰ�̸� ��� Ƚ�� ���� ����ϵ���
	_bool    m_isConsumable = { false };

	_int     m_iUseCount = { 0 };

	_bool    m_isActive = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
	
};

NS_END

