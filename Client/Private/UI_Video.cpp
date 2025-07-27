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

	if (FAILED(InitMediaFoundationAndCreateReader(m_strVideoPath.c_str(), m_pReader)))
		return E_FAIL;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	if (FAILED(Ready_Components()))
		return E_FAIL;


	BYTE* pData = nullptr;
	DWORD width = 0, height = 0;
	LONGLONG time = 0;

	if (SUCCEEDED(ReadFrameToBuffer(m_pReader, &pData, &width, &height, &time)))
	{
		Safe_Release(m_pVideoSRV);
		ID3D11ShaderResourceView* tempSRV = {nullptr};
		if (SUCCEEDED(UploadFrame(m_pDevice, m_pContext, pData, width, height, &tempSRV)))
		{
			m_pVideoSRV = tempSRV;
		}
		
		delete[] pData;
	}

	return S_OK;
}

void CUI_Video::Priority_Update(_float fTimeDelta)
{
}

void CUI_Video::Update(_float fTimeDelta)
{
	m_fElapsedTime += fTimeDelta * m_fPlaybackSpeed;

	Fade(fTimeDelta);

	if (m_fElapsedTime >= m_fFrameInterval)
	{
		m_fElapsedTime = 0.f;

		BYTE* pData = nullptr;
		DWORD width = 0, height = 0;
		LONGLONG time = 0;

		HRESULT hr = ReadFrameToBuffer(m_pReader, &pData, &width, &height, &time);

		if (SUCCEEDED(hr))
		{
			Safe_Release(m_pVideoSRV);
			ID3D11ShaderResourceView* tempSRV = nullptr;
			hr = UploadFrame(m_pDevice, m_pContext, pData, width, height, &tempSRV);
			if (SUCCEEDED(hr))
			{
				m_pVideoSRV = tempSRV;
			}
			
			delete[] pData;
		}
		else if (hr == MF_E_END_OF_STREAM)
		{
			// 영상이 끝났을 경우: 루프 시작
			m_pReader->Flush(MF_SOURCE_READER_FIRST_VIDEO_STREAM);

			PROPVARIANT var;
			PropVariantInit(&var);
			var.vt = VT_I8;
			var.hVal.QuadPart = 0; // 0부터 다시 시작
			m_pReader->SetCurrentPosition(GUID_NULL, var);
			PropVariantClear(&var);
		}
		else
		{
			
			cout << L"영상 프레임 읽기 실패\n";
		}
	}

	
}

void CUI_Video::Late_Update(_float fTimeDelta)
{
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

	
	if (FAILED(m_pShaderCom->Bind_RawValue("g_Alpha", &m_fCurrentAlpha, sizeof(_float))))
		return E_FAIL;
		
	
	m_pVIBufferCom->Bind_Buffers();
	m_pShaderCom->Begin(UI_FADE);
	m_pVIBufferCom->Render();

	return S_OK;
}

HRESULT CUI_Video::InitMediaFoundationAndCreateReader(const WCHAR* szFilePath, IMFSourceReader*& outReader)
{
	HRESULT hr = MFStartup(MF_VERSION);
	if (FAILED(hr))
		return hr;

	IMFAttributes* pAttributes;
	hr = MFCreateAttributes(&pAttributes, 1);
	if (FAILED(hr))
		return hr;

	pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

	hr = MFCreateSourceReaderFromURL(szFilePath, pAttributes, &outReader);
	if (FAILED(hr))
		return hr;

	IMFMediaType* pTypeOut;
	hr = MFCreateMediaType(&pTypeOut);
	if (FAILED(hr))
		return hr;

	hr = pTypeOut->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	hr = pTypeOut->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	if (FAILED(hr))
		return hr;

	hr = outReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pTypeOut);


	if (nullptr != pAttributes) pAttributes->Release();
	if (nullptr != pTypeOut) pTypeOut->Release();

	return hr;
}

HRESULT CUI_Video::ReadFrameToBuffer(IMFSourceReader* pReader, BYTE** ppData, DWORD* pWidth, DWORD* pHeight, LONGLONG* pTimeStamp)
{
	if (!pReader || !ppData || !pWidth || !pHeight || !pTimeStamp)
		return E_INVALIDARG;

	IMFSample* pSample = nullptr;
	IMFMediaBuffer* pBuffer = nullptr;
	IMFMediaType* pMediaType = nullptr;

	DWORD dwStreamFlags = 0;
	LONGLONG llTimestamp = 0;
	HRESULT hr = pReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, &dwStreamFlags, &llTimestamp, &pSample);

	if (FAILED(hr))
		return hr;


	// 스트림 끝 체크
	if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		return MF_E_END_OF_STREAM;

	// 샘플 없으면 아직 프레임 없음 (일시적)
	if (pSample == nullptr)
		return S_FALSE;


	hr = pSample->ConvertToContiguousBuffer(&pBuffer);
	if (FAILED(hr))
	{
		if (pSample) pSample->Release();
		return hr;
	}

	BYTE* pSrc = nullptr;
	DWORD maxLen = 0, curLen = 0;
	hr = pBuffer->Lock(&pSrc, &maxLen, &curLen);
	if (FAILED(hr))
	{
		pBuffer->Release();
		pSample->Release();
		return hr;
	}

	hr = pReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pMediaType);
	if (FAILED(hr))
	{
		pBuffer->Unlock();
		pBuffer->Release();
		pSample->Release();
		return hr;
	}

	UINT32 width = 0, height = 0;
	hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &width, &height);
	if (FAILED(hr))
	{
		pMediaType->Release();
		pBuffer->Unlock();
		pBuffer->Release();
		pSample->Release();
		return hr;
	}

	*ppData = new BYTE[curLen];
	memcpy(*ppData, pSrc, curLen);
	*pWidth = width;
	*pHeight = height;
	*pTimeStamp = llTimestamp;

	pBuffer->Unlock();

	pMediaType->Release();
	pBuffer->Release();
	pSample->Release();

	return S_OK;
}

HRESULT CUI_Video::UploadFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, BYTE* pData, UINT32 width, UINT32 height, ID3D11ShaderResourceView** ppSRV)
{
	if (!pData || !pDevice || !ppSRV)
		return E_INVALIDARG;

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

	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hr = pDevice->CreateTexture2D(&texDesc, nullptr, &pTexture);
	if (FAILED(hr))
		return hr;

	D3D11_MAPPED_SUBRESOURCE mapped = {};
	hr = pContext->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (FAILED(hr))
	{
		pTexture->Release();
		return hr;
	}

	for (UINT32 y = 0; y < height; ++y)
	{
		memcpy((BYTE*)mapped.pData + y * mapped.RowPitch, pData + y * width * 4, width * 4);
	}
	pContext->Unmap(pTexture, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(pTexture, &srvDesc, ppSRV);
	pTexture->Release();
	return hr;
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
	Safe_Release(m_pTextureCom);
	Safe_Release(m_pVideoSRV);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pReader);
	Safe_Release(m_pShaderCom);

	MFShutdown();
}