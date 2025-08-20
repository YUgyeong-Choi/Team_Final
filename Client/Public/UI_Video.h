#pragma once

#include "Client_Defines.h"
#include "UIObject.h"

NS_BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Rect;
class CSoundController;
NS_END

NS_BEGIN(Client)

// 음 파일로는 저장 따로 안하고 그냥 코드에서 생성하도록?

class CUI_Video final : public CUIObject
{
public:
	enum class VIDEO_TYPE
	{
		LOGO,
		INTRO
	};
public:
	typedef struct tagVideoUIDesc : public CUIObject::UIOBJECT_DESC
	{
		VIDEO_TYPE eType;
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

	HRESULT UploadFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, BYTE* pData, UINT32 width, UINT32 height, ID3D11ShaderResourceView** ppSRV);

	HRESULT InitFFmpegAndOpenVideo(const char* szFilePath);
	HRESULT ReadFrameToBuffer(BYTE** ppData, DWORD* pWidth, DWORD* pHeight, LONGLONG* pTimeStamp);

	void Release_FFmpeg();

private:
	void Play_Sound();
private:
	CShader* m_pShaderCom = { nullptr };
	CVIBuffer_Rect* m_pVIBufferCom = { nullptr };

	CSoundController* m_pSoundCom = { nullptr };
private:
	_bool	 m_isLoop = { true };
	_wstring m_strVideoPath = {};
	_float   m_fPlaybackSpeed = {};

	_float   m_fFrameInterval = {};
	_float   m_fElapsedTime = {};
	_float   m_fDuration = {};

	ID3D11ShaderResourceView* m_pVideoSRV = { nullptr };
	ID3D11Texture2D* m_pTexture = { nullptr };


	//
	AVFormatContext* m_pFormatCtx = nullptr;
	AVCodecContext* m_pCodecCtx = nullptr;
	AVFrame* m_pFrame = nullptr;
	AVFrame* m_pFrameRGB = nullptr;
	struct SwsContext* m_pSwsCtx = nullptr;
	_int m_videoStreamIndex = -1;
	uint8_t* m_rgbBuffer = nullptr;

private:
	HRESULT Ready_Components();
private:
	_int m_iDrawnFrameIndex = -1;
	VIDEO_TYPE m_eVideoType = VIDEO_TYPE::LOGO;
public:
	static CUI_Video* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END