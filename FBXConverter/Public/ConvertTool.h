#pragma once
#include "ImGuiTool.h"

NS_BEGIN(Client)


class CConvertTool final : public CImGuiTool
{
private:
	CConvertTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CConvertTool(const CConvertTool& Prototype);
	virtual ~CConvertTool() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();

private:
	Assimp::Importer		m_Importer;

	/* 모델에 대한 모든 정보르,ㄹ 담고 있는 구조체. */
	const aiScene* m_pAIScene = { nullptr };

	//FBXDATA m_pFBXData = { };
	list<path> materialList;
	string		savePath;

	_bool		m_isAnim = { false };
	_bool		m_isAnimOnly = { false };
	_int		m_iCurNumBones = { };
	vector<string> m_BoneNames;


	typedef struct tagChannelData
	{
		_uint iNumKeyFrames = {};
		vector<KEYFRAME> Keyframes;
		_uint iBoneIndex = {};
	}TOOLCHANNELDATA;


	typedef struct tagAnimData
	{
		_float fTicksPerSec = {};
		_float fDuration = {};
		_uint iNumChannels = {};
		vector<TOOLCHANNELDATA> Channels;
	}TOOLANIMDATA;

	_uint m_iDstNumAnimations = {};
	_uint m_iSrcNumAnimations = {};


private:
	HRESULT Render_ConvertTool();
	HRESULT Convert_NonAnimFBX(const _char* pModelFilePath);
	HRESULT Convert_AnimFBX(const _char* pModelFilePath);
	HRESULT Convert_AnimOnly(const _char* pModelFilePath);
	HRESULT Copy_MaterialTextures();

private:
	HRESULT Render_MergeTool();
	HRESULT Read_DestinationBinary(const _char* pModelFilePath, vector<char>& rawData, vector<TOOLANIMDATA>& AnimData);
	HRESULT Read_SourceBinary(const _char* pModelFilePath, vector<char>& rawData, vector<TOOLANIMDATA>& AnimData);
	HRESULT Merge_Animation(const _char* pModelFilePath, vector<char>& rawData, vector<TOOLANIMDATA>& DstAnimData, vector<TOOLANIMDATA>& SrcAnimData);

private: // 멍청해서 다시쓰는중 파일스트림으로
	HRESULT Write_BoneData(const aiNode* pAINode, _int iParentBoneIndex, vector<FBX_BONEDATA>& m_Bones, ostream& ofs);
	HRESULT Write_NonAnimMeshData(ostream& ofs);
	HRESULT Write_AnimMeshData(const vector<FBX_BONEDATA>& Bones, ostream& ofs);
	HRESULT Write_MaterialData(const _char* pModelFilePath, ostream& ofs);
	HRESULT Write_AnimationData(const vector<FBX_BONEDATA>& Bones, ostream& ofs);
public:
	static CConvertTool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;

};

NS_END