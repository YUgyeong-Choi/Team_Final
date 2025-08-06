#pragma once

#include "Client_Defines.h"
#include "UIObject.h"





NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
NS_END

NS_BEGIN(Client)

// 음 파일로는 저장 따로 안하고 그냥 코드에서 생성하도록?



class CUI_Video final : public CUIObject
{
public:
	typedef struct tagVideoUIDesc : public CUIObject::UIOBJECT_DESC
	{
		_float	fInterval;
		wstring strVideoPath;
		_bool   isLoop = { true };
	}VIDEO_UI_DESC;



private:
	CUI_Video(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUI_Video(const CUI_Video& Prototype);
	virtual ~CUI_Video() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

	HRESULT InitMediaFoundationAndCreateReader(const WCHAR* szFilePath, IMFSourceReader*& outReader);
	HRESULT ReadFrameToBuffer(IMFSourceReader* pReader, BYTE** ppData, DWORD* pWidth, DWORD* pHeight, LONGLONG* pTimeStamp);
	HRESULT UploadFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, BYTE* pData, UINT32 width, UINT32 height, ID3D11ShaderResourceView** ppSRV);


private:
	CShader* m_pShaderCom = { nullptr };
	CTexture* m_pTextureCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

private:
	_bool	 m_isLoop = {true};
	_wstring m_strVideoPath = {};
	_float   m_fPlaybackSpeed = {};

	_float   m_fFrameInterval = {};
	_float   m_fElapsedTime = {};
	_float   m_fDuration = {};

	ID3D11ShaderResourceView* m_pVideoSRV = { nullptr };

	IMFSourceReader*		  m_pReader = { nullptr };

private:
	HRESULT Ready_Components();

public:
	static CUI_Video* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END