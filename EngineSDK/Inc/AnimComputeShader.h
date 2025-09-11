#pragma once
#include "ComputeShader.h"
NS_BEGIN(Engine)

typedef struct tagAnimComputerShaderDesc
{
	_float blendWeight;      // 4����Ʈ
	_float blendFactor;      // 4����Ʈ  
	_uint  boneCount;        // 4����Ʈ
	_uint  currentLevel;     // 4����Ʈ  16����Ʈ �ϼ�

	_uint  isMasked;         // 4����Ʈ
	_float padding[3];       // 12����Ʈ �ι�° 16����Ʈ �ϼ�

	_float4x4 preTransformMatrix; // 64����Ʈ (4��16����Ʈ) 
}ANIM_CS_DESC;
class CAnimComputeShader : public CComputeShader
{
private:
	CAnimComputeShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CAnimComputeShader(const CAnimComputeShader& Prototype) = delete;
	virtual ~CAnimComputeShader() = default;

public:
	 HRESULT Initialize_AnimComputeShader(const _wstring& wstrFilePath,_uint iBoneCount);
	 void UploadBoneMatrices(const _float4x4* pBoneMatrices); // GPU ���۷� �ѱ�� 
	 void ExecuteHierarchical(const _float4x4& preTransform); // ������ ��� ����
	 HRESULT DownloadBoneMatrices(_float4x4* pOutBoneMatrices, _uint iCount); // GPU ���ۿ��� ��� ���� ��������
	 
public:
	void BuildHierarchyLevels();

	void SetBoneCount(_uint iBoneCount) { m_iBoneCount = iBoneCount; } // ���� ���� ����
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
	_uint m_iBoneCount{ 0 }; // ���� ����
	ID3D11Buffer* m_pCSParamBuffer = nullptr;
	ID3D11Buffer* m_pParentIndexBuffer = nullptr;
	ID3D11Buffer* m_pInputBuffer{ nullptr }; //����
	ID3D11Buffer* m_pOutputBuffer{ nullptr }; //����
	ID3D11Buffer* m_pBoneMaskBuffer = nullptr;
	
	ID3D11ShaderResourceView* m_pParentIndexSRV = nullptr;
	ID3D11ShaderResourceView* m_pBoneMaskSRV = nullptr;
	ID3D11ShaderResourceView* m_pSourceBoneSRV{ nullptr }; // �ҽ� ���� SRV
	ID3D11ShaderResourceView* m_pTargetBoneSRV{ nullptr }; // ���� SRV
	ID3D11ShaderResourceView* m_pOutputBoneSRVForVS{ nullptr }; // ���� SRV for VS

	ID3D11UnorderedAccessView* m_pOutputBoneUAV{ nullptr }; // ���� UAV

	vector<_int> m_BoneLevels;          // �� ���� ���� ���� (0=��Ʈ, 1=1�ܰ�...)
	vector<_int> m_ParentIndices; // �θ� �ε��� �迭
	_uint m_iMaxLevel = 0;              // �ִ� ���� ����
	ID3D11Buffer* m_pBoneLevelBuffer = nullptr;   // ���� ���� ����
	ID3D11ShaderResourceView* m_pBoneLevelSRV = nullptr;
public:
	static CAnimComputeShader* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _wstring& wstrFilePath, _uint iBoneCount);
	virtual void Free() override;

};
NS_END

