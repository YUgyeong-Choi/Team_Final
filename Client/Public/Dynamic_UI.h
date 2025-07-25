#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

class CDynamic_UI : public CUIObject
{
public:
	typedef struct tagDynamicUIDesc : public CUIObject::UIOBJECT_DESC
	{
		_bool isLoop = {};
		_int iPassIndex = { 0 };
		_int iTextureIndex = { 0 };
		wstring strTextureTag;
		_int iStartFrame = {};
		_int iEndFrame = {};
		_int iUIType = {};
		_float2 fOffsetUV = {};
	}DYNAMIC_UI_DESC;


	_wstring& Get_StrTextureTag() { return m_strTextureTag; }

	DYNAMIC_UI_DESC Get_Desc()
	{
		DYNAMIC_UI_DESC eDesc = {};
		eDesc.fSizeX = m_fSizeX;
		eDesc.fSizeY = m_fSizeY;
		eDesc.iPassIndex = m_iPassIndex;
		eDesc.iTextureIndex = m_iTextureIndex;
		eDesc.fX = m_fX;
		eDesc.fY = m_fY;
		eDesc.fOffset = m_fOffset;
		eDesc.strTextureTag = m_strTextureTag;
		eDesc.iStartFrame = m_iStartFrame;

		return eDesc;

	}



private:
	CDynamic_UI(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDynamic_UI(const CDynamic_UI& Prototype);
	virtual ~CDynamic_UI() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	void Update_UI_From_Tool(DYNAMIC_UI_DESC& eDesc);
	void Reset() {};

	HRESULT Ready_Components(const wstring& strTextureTag);
	HRESULT Bind_ShaderResources();

private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:

	// tool에서 사용할...
	_wstring m_strTextureTag = {};

	// 월드에 그릴 사이즈
	_float m_fScale = {};
	_int   m_iPassIndex = {};
	_int   m_iTextureIndex = {};

	//
	_int   m_iStartFrame = {};
	_int   m_iEndFrame = {};
	_int   m_iUIType = {};

	_bool  m_isLoop = {false};


	// 이걸 이제 상속받아서 변수들을 나누고, 함수도 나누고...

	// 이동할때 필요한 이동 방향, 이동 시간 
	_float4 m_vDir = {};
	_float	m_fMoveTime = {};

	// fade in out 에 사용할 변수?
	_bool	m_isFade = {};
	_float  m_fDuration = {};

	// uv
	_float2 m_fUV = {};
	_float2 m_fOffsetUV = {};
	// 가로 세로 몇개 있는지
	_int    m_iWidth = {};
	_int    m_iHeight = {};
	_int    m_iCount = {};


private:
	

public:
	static CDynamic_UI* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END