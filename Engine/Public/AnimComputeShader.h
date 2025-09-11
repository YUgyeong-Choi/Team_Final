#pragma once
#include "ComputeShader.h"
NS_BEGIN(Engine)

typedef struct tagAnimComputerShaderDesc
{
	_float blendWeight;      // 4바이트
	_float blendFactor;      // 4바이트  
	_uint  boneCount;        // 4바이트
	_uint  currentLevel;     // 4바이트  16바이트 완성

	_uint  isMasked;         // 4바이트
	_float padding[3];       // 12바이트 두번째 16바이트 완성

	_float4x4 preTransformMatrix; // 64바이트 (4×16바이트) 
}ANIM_CS_DESC;
class CAnimComputeShader : public CComputeShader
{
private:
	CAnimComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimComputeShader(const CAnimComputeShader& Prototype) = delete;
	virtual ~CAnimComputeShader() = default;

public:
	 HRESULT Initialize_AnimComputeShader(const _wstring& wstrFilePath,_uint iBoneCount);
	 void UploadBoneMatrices(const _float4x4* pBoneMatrices); // GPU 버퍼로 넘기기 
	 void ExecuteHierarchical(const _float4x4& preTransform); // 계층적 계산 실행
	 HRESULT DownloadBoneMatrices(_float4x4* pOutBoneMatrices, _uint iCount); // GPU 버퍼에서 계산 정보 가져오기
	 
public:
	void BuildHierarchyLevels();

	void SetBoneCount(_uint iBoneCount) { m_iBoneCount = iBoneCount; } // 뼈대 개수 설정
	_uint GetBoneCount() const { return m_iBoneCount; }

	ID3D11Buffer* GetInputBuffer() const { return m_pInputBuffer; }
	ID3D11Buffer* GetOutputBuffer() const { return m_pOutputBuffer; }
	ID3D11ShaderResourceView* GetSourceBoneSRV() const { return m_pSourceBoneSRV; }
	ID3D11ShaderResourceView* GetTargetBoneSRV() const { return m_pTargetBoneSRV; }
	ID3D11UnorderedAccessView* GetOutputBoneUAV() const { return m_pOutputBoneUAV; }
	ID3D11ShaderResourceView* GetOutputBoneSRV() const { 
		return m_pOutputBoneSRVForVS; }

	void   SetCSParams(const ANIM_CS_DESC& desc);
	void   SetParentIndices(const vector<_int>& parents);
	void   SetBoneMask(const vector<_float>& mask);

private:
	_uint m_iBoneCount{ 0 }; // 뼈대 개수
	ID3D11Buffer* m_pCSParamBuffer = nullptr;
	ID3D11Buffer* m_pParentIndexBuffer = nullptr;
	ID3D11Buffer* m_pInputBuffer{ nullptr }; //버퍼
	ID3D11Buffer* m_pOutputBuffer{ nullptr }; //버퍼
	ID3D11Buffer* m_pBoneMaskBuffer = nullptr;
	
	ID3D11ShaderResourceView* m_pParentIndexSRV = nullptr;
	ID3D11ShaderResourceView* m_pBoneMaskSRV = nullptr;
	ID3D11ShaderResourceView* m_pSourceBoneSRV{ nullptr }; // 소스 뼈대 SRV
	ID3D11ShaderResourceView* m_pTargetBoneSRV{ nullptr }; // 뼈대 SRV
	ID3D11ShaderResourceView* m_pOutputBoneSRVForVS{ nullptr }; // 뼈대 SRV for VS

	ID3D11UnorderedAccessView* m_pOutputBoneUAV{ nullptr }; // 뼈대 UAV

	vector<_int> m_BoneLevels;          // 각 뼈의 계층 레벨 (0=루트, 1=1단계...)
	vector<_int> m_ParentIndices; // 부모 인덱스 배열
	_uint m_iMaxLevel = 0;              // 최대 계층 깊이
	ID3D11Buffer* m_pBoneLevelBuffer = nullptr;   // 레벨 정보 버퍼
	ID3D11ShaderResourceView* m_pBoneLevelSRV = nullptr;
public:
	static CAnimComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, _uint iBoneCount);
	virtual void Free() override;

};
NS_END

