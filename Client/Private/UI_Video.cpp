#include "UI_Video.h"
#include "GameInstance.h"

CUI_Video::CUI_Video(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CUIObject{ pDevice, pContext }
{
}

CUI_Video::CUI_Video(const CUI_Video& Prototype)
	: CUIObject(Prototype)
{
}

HRESULT CUI_Video::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CUI_Video::Initialize(void* pArg)
{
	VIDEO_UI_DESC* pDesc = static_cast<VIDEO_UI_DESC*>(pArg);

	m_strVideoPath = pDesc->strVideoPath;
	m_fPlaybackSpeed = pDesc->fSpeedPerSec;
	m_fFrameInterval = pDesc->fInterval;

	m_fFrameInterval = pDesc->fInterval;

	m_isLoop = pDesc->isLoop;
	

	if (FAILED(InitFFmpegAndOpenVideo(WStringToString(m_strVideoPath).c_str())))
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;

	BYTE* pData = nullptr;
	DWORD width = 0, height = 0;
	LONGLONG time = 0;

	HRESULT hr = ReadFrameToBuffer(&pData, &width, &height, &time);

	hr = UploadFrame(m_pDevice, m_pContext, pData, width, height, &m_pVideoSRV);

	Safe_Delete_Array(pData);

	return S_OK;
}

void CUI_Video::Priority_Update(_float fTimeDelta)
{
}

void CUI_Video::Update(_float fTimeDelta)
{
	if (m_bDead)
		return;
	m_fElapsedTime += fTimeDelta * m_fPlaybackSpeed;

	Fade(fTimeDelta);

	if (m_fElapsedTime >= m_fFrameInterval)
	{
		m_fElapsedTime = 0.f;

		BYTE* pData = nullptr;
		DWORD width = 0, height = 0;
		LONGLONG time = 0;

		HRESULT hr = ReadFrameToBuffer(&pData, &width, &height, &time);
		

		if (hr == MF_E_END_OF_STREAM || hr == AVERROR_EOF)
		{
			if (m_isLoop)
			{
				av_seek_frame(m_pFormatCtx, m_videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
				avcodec_flush_buffers(m_pCodecCtx);
				hr = ReadFrameToBuffer(&pData, &width, &height, &time);
			}
			else
			{
				Safe_Release(m_pVideoSRV);
				Safe_Delete_Array(pData);
				Set_bDead();
				return;
			}
		}
		else
		{
			Safe_Release(m_pVideoSRV);

			ID3D11ShaderResourceView* tempSRV = nullptr;
			hr = UploadFrame(m_pDevice, m_pContext, pData, width, height, &tempSRV);
			if (SUCCEEDED(hr))
			{
				m_pVideoSRV = tempSRV;
				Safe_Delete_Array(pData);
			}

			Safe_Delete_Array(pData);
		}
		
	}

	
}

void CUI_Video::Late_Update(_float fTimeDelta)
{
	if (m_bDead)
		return;

	
	m_pGameInstance->Add_RenderGroup(RENDERGROUP::RG_UI, this);
	
}

HRESULT CUI_Video::Render()
{
	if (nullptr == m_pVideoSRV)
		return E_FAIL;

	m_pShaderCom->Bind_SRV("g_Texture", m_pVideoSRV);

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_Color", &m_vColor, sizeof(_float4))))
		return E_FAIL;

	
	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
		return E_FAIL;
		
	
	m_pVIBufferCom->Bind_Buffers();
	m_pShaderCom->Begin(UI_FADE);
	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CUI_Video::UploadFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, BYTE* pData, UINT32 width, UINT32 height, ID3D11ShaderResourceView** ppSRV)
{
	if (!pData || !pDevice || !ppSRV)
		return E_INVALIDARG;

	Safe_Release(m_pTexture);

	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	
	HRESULT hr = pDevice->CreateTexture2D(&texDesc, nullptr, &m_pTexture);
	if (FAILED(hr))
		return hr;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	hr = pContext->Map(m_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
	{
		Safe_Release(m_pTexture);
		return hr;
	}

	for (UINT32 y = 0; y < height - 8; ++y)
	{
		memcpy((BYTE*)mapped.pData + y * mapped.RowPitch, pData + y * width * 4, width * 4);
	}
	
	// 초록 부분(쓰레기값)을 없애기 위해 가장 정상적인 데이터가 나오는 위치에 픽셀 값을 다 박는다
	BYTE* pSourceLine = pData + (height - 9) * mapped.RowPitch;

	BYTE* pDst = (BYTE*)mapped.pData + (height - 8) * mapped.RowPitch;

	for (UINT32 i = 0; i < 8; ++i)
	{
		memcpy(pDst + i * mapped.RowPitch, pSourceLine, width * 4);
	}

	pContext->Unmap(m_pTexture, 0);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pTexture, &srvDesc, ppSRV);
	Safe_Release(m_pTexture);
	return hr;
}

HRESULT CUI_Video::InitFFmpegAndOpenVideo(const char* szFilePath)
{
	avformat_network_init();

	if (avformat_open_input(&m_pFormatCtx, szFilePath, nullptr, nullptr) != 0)
		return E_FAIL;

	if (avformat_find_stream_info(m_pFormatCtx, nullptr) < 0)
		return E_FAIL;

	m_videoStreamIndex = -1;
	for (unsigned int i = 0; i < m_pFormatCtx->nb_streams; i++) {
		if (m_pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			m_videoStreamIndex = i;
			break;
		}
	}
	if (m_videoStreamIndex == -1) return E_FAIL;

	AVCodecParameters* pCodecParams = m_pFormatCtx->streams[m_videoStreamIndex]->codecpar;
	const AVCodec* pCodec = avcodec_find_decoder(pCodecParams->codec_id);
	if (!pCodec) return E_FAIL;

	m_pCodecCtx = avcodec_alloc_context3(pCodec);
	avcodec_parameters_to_context(m_pCodecCtx, pCodecParams);
	if (avcodec_open2(m_pCodecCtx, pCodec, nullptr) < 0) return E_FAIL;

	m_pFrame = av_frame_alloc();
	m_pFrameRGB = av_frame_alloc();

	int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGBA, m_pCodecCtx->width, m_pCodecCtx->height, 1);
	m_rgbBuffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));
	av_image_fill_arrays(m_pFrameRGB->data, m_pFrameRGB->linesize, m_rgbBuffer,
		AV_PIX_FMT_RGBA, m_pCodecCtx->width, m_pCodecCtx->height, 1);

	m_pSwsCtx = sws_getContext(
		m_pCodecCtx->width, m_pCodecCtx->height, m_pCodecCtx->pix_fmt,
		m_pCodecCtx->width, m_pCodecCtx->height, AV_PIX_FMT_BGRA,
		SWS_BILINEAR, nullptr, nullptr, nullptr
	);

	return S_OK;
}

HRESULT CUI_Video::ReadFrameToBuffer(BYTE** ppData, DWORD* pWidth, DWORD* pHeight, LONGLONG* pTimeStamp)
{
	if (!ppData || !pWidth || !pHeight || !pTimeStamp) return E_INVALIDARG;

	AVPacket packet;
	while (av_read_frame(m_pFormatCtx, &packet) >= 0) {
		if (packet.stream_index == m_videoStreamIndex) {
			int ret = avcodec_send_packet(m_pCodecCtx, &packet);
			if (ret < 0) {
				av_packet_unref(&packet);
				return E_FAIL;
			}

			ret = avcodec_receive_frame(m_pCodecCtx, m_pFrame);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
				av_packet_unref(&packet);
				continue;
			}
			else if (ret < 0) {
				av_packet_unref(&packet);
				return E_FAIL;
			}

			sws_scale(m_pSwsCtx,
				m_pFrame->data, m_pFrame->linesize,
				0, m_pCodecCtx->height,
				m_pFrameRGB->data, m_pFrameRGB->linesize);

			*ppData = new BYTE[m_pCodecCtx->width * m_pCodecCtx->height * 4];
			memcpy(*ppData, m_pFrameRGB->data[0], m_pCodecCtx->width * m_pCodecCtx->height * 4);
			*pWidth = m_pCodecCtx->width;
			*pHeight = m_pCodecCtx->height;
			*pTimeStamp = m_pFrame->pts;

			av_packet_unref(&packet);
			return S_OK;
		}
		av_packet_unref(&packet);
	}

	return MF_E_END_OF_STREAM; // 끝 도달
}

void CUI_Video::Release_FFmpeg()
{

	// 프레임
	if (m_pFrame) {
		av_frame_free(&m_pFrame);
		m_pFrame = nullptr;
	}
	if (m_pFrameRGB) {
		av_frame_free(&m_pFrameRGB);
		m_pFrameRGB = nullptr;
	}

	//context

	if (m_pSwsCtx) {
		sws_freeContext(m_pSwsCtx);
		m_pSwsCtx = nullptr;
	}

	if (m_pCodecCtx) {
		avcodec_free_context(&m_pCodecCtx);
		m_pCodecCtx = nullptr;
	}

	if (m_pFormatCtx) {
		avformat_close_input(&m_pFormatCtx);
		m_pFormatCtx = nullptr;
	}

	// 버퍼

	if (m_rgbBuffer) {
		av_free(m_rgbBuffer);
		m_rgbBuffer = nullptr;
	}

	avformat_network_deinit();
}

HRESULT CUI_Video::Ready_Components()
{

	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(ENUM_CLASS(LEVEL::STATIC), TEXT("Prototype_Component_Shader_UI"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(static_cast<int>(LEVEL::STATIC), TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	return S_OK;
}




CUI_Video* CUI_Video::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CUI_Video* pInstance = new CUI_Video(pDevice, pContext);
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Create : CUI_Video");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_Video::Clone(void* pArg)
{
	CUI_Video* pClone = new CUI_Video(*this);

	if (FAILED(pClone->Initialize(pArg)))
	{
		MSG_BOX("Failed to Clone : CUI_Video");
		Safe_Release(pClone);
		return nullptr;
	}

	
	return pClone;
}
	
void CUI_Video::Free()
{
	__super::Free();
	
	Safe_Release(m_pVideoSRV);
	Safe_Release(m_pVIBufferCom);
	
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTexture);

	Release_FFmpeg();
	
}