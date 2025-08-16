#pragma once
#include "Base.h"
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CComputeShader : public CBase
{
protected:
	CComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CComputeShader(const CComputeShader& Prototype) = delete;
	virtual ~CComputeShader() = default;

public:
	virtual HRESULT Initialize(const _wstring& wstrFilePath);
	virtual void Bind();
	virtual void Unbind();
	void Dispatch(_uint iGroupX, _uint iGroupY = 1, _uint iGroupZ = 1)
	{
		m_pContext->Dispatch(iGroupX, iGroupY, iGroupZ);
			
	}

	static void           ReleaseCache();
protected:
	ID3D11ComputeShader* m_pComputeShader{ nullptr };
	ID3D11Device* m_pDevice{ nullptr };
	ID3D11DeviceContext* m_pContext{ nullptr };

private:
	static unordered_map<_wstring, ID3D11ComputeShader*> m_ComputeShaderCache; // 캐시를 사용하여 중복 로딩 방지

public:
	static CComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath);
	virtual void Free() override;

};
NS_END

