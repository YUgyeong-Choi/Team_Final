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

	// 사용 효과 상속받아서 구현하기
	// 애니메이션 시작할 때 true, 끝날때 false
	virtual void Activate(_bool isActive) = 0;

	virtual ITEM_DESC Get_ItemDesc() { return ITEM_DESC(); };

protected:
	// 이거로 구분하던지 아니면 다른거로 구분하던지
	// 그냥 enum해서 하는게 좋을수도?

	_wstring m_strProtoTag = {};
	// UI 표시 용도
	// 쓸때만 할거는 껏다 켯다
	// 항상 보이는거는 항상 true로? (램프 같은거)
	_bool    m_isRender = {	false };
	// 사용 가능한지 불가능한지
	_bool    m_isUsable = { true };
	// 소모품인지 아닌지, 소모품이면 사용 횟수 같이 출력하도록
	_bool    m_isConsumable = { false };

	_int     m_iUseCount = { 0 };

	_bool    m_isActive = {};

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
	
};

NS_END

