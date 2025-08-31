#pragma once

#include "GameObject.h"
#include "Serializable.h"

/* CUIObject_2D */
/* CUIObject_3D */

NS_BEGIN(Engine)

class ENGINE_DLL CUIObject abstract : public CGameObject, public ISerializable
{
public:
	typedef struct tagUIObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float			fX, fY, fSizeX{0.01f}, fSizeY{0.01f}, fAlpha{1.f};
		_float			fOffset = {0.f};
		_float4 vColor = { 1.f,1.f,1.f,1.f };
		_wstring strProtoTag = {};

		_float fRotation = {};
		_float fDelay = {};

	}UIOBJECT_DESC;


	_float Get_Depth() { return m_fOffset; }

	_bool  Get_isFade() { return m_isFade; }
	_float Get_Alpha() { return m_fCurrentAlpha; }
	void  Set_Alpha(_float fAlpha) { m_fCurrentAlpha = fAlpha; }

	void  Set_isVignetting(_bool isVignetting) { m_isDeferred = isVignetting; }

	void Set_Color(_float4 vColor) { m_vColor = vColor; }

	_wstring Get_ProtoTag() { return m_strProtoTag; }

	virtual void Set_isReverse(_bool isReverse) { m_isReverse = isReverse; }

	_float2 Get_Pos() { return { m_fX, m_fY }; }
	_float2 Get_Size() { return {m_fSizeX, m_fSizeY}; }

	virtual json Serialize();
	virtual void Deserialize(const json& j);

protected:
	CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject(const CUIObject& Prototype);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual void Update_Data();

	// ����Ʈ�� ���ؼ� ��ġ�� �����
	void Set_Position(_float fX, _float fY);

	// ���߿� ����, start�� ũ���ϸ� fade out, �۰��ϸ� fade in
	void FadeStart(_float fStartAlpha, _float fEndAlpha, _float fDuration);
	

	void Fade(_float fTimeDelta);
	
	
	virtual HRESULT Ready_Components_File(const wstring& strTextureTag) { return S_OK; }
	virtual void Update_UI_From_Tool(void* pArg) {};
protected:
	/* ����Ʈ ���� �������� �߽���ġ fX, fY, ������ fSiuzeX, fSizeY */
	_float			m_fX{}, m_fY{}, m_fSizeX{}, m_fSizeY{};
	_float			m_fOffset = {0.f};

	/* ���� ������ ���� ���. */
	_float4x4		m_ViewMatrix{}, m_ProjMatrix{};

	// fade ��, �����ϰ� ���Ÿ� �̰� ���
	_bool			m_isFade = {false};
	_float			m_fFadeTime = {};
	_float			m_fFadeElapsedTime = {};
	_float			m_fCurrentAlpha = {1.f};
	_float			m_fStartAlpha = {};
	_float			m_fEndAlpha = {};

	_wstring    m_strProtoTag = {  };

	// ���� ��. 
	_float4         m_vColor = {1.f,1.f,1.f,1.f};

	_bool			m_isDeferred = { false };

	// �̰ɷ� ��������
	_int			m_iUIType = {};

	_float			m_fRotation = {};

	_bool			m_isReverse = { false };

	_float			m_fDelay = {};
	

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END