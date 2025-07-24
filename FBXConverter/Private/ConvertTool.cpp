#include "GameInstance.h"
#include "ConvertTool.h"

#include <filesystem>
using namespace std::filesystem;
//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() �̷� �̱������� ����� ���� ���ϰ� ��


CConvertTool::CConvertTool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CImGuiTool{ pDevice, pContext }
{

}

CConvertTool::CConvertTool(const CConvertTool& Prototype)
	: CImGuiTool(Prototype)
{
}

HRESULT CConvertTool::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CConvertTool::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	savePath = R"(C:\Users\CMAP\Documents\Dx11_Personal_Projects\3d\testing)";



	return S_OK;
}

void CConvertTool::Priority_Update(_float fTimeDelta)
{

}

void CConvertTool::Update(_float fTimeDelta)
{
}

void CConvertTool::Late_Update(_float fTimeDelta)
{
}

HRESULT CConvertTool::Render()
{
	if (m_pWindowData->ShowConvertMenu)
	{
		if (FAILED(Render_ConvertTool()))
			return E_FAIL;
	}

	return S_OK;
}

HRESULT CConvertTool::Render_ConvertTool()
{
	SetNextWindowSize(ImVec2(200, 300));
	Begin("Convert Tools", &m_pWindowData->ShowConvertMenu, NULL);
	
	IGFD::FileDialogConfig config;
	if (Button("Load NonAnim File"))
	{
		m_isAnim = false;	
		savePath = R"(../../Client\Bin\Resources\Models\Bin_NonAnim)";
		config.path = R"(Z:\Lie\LieOfP_SM\Game\LiesofP\Content\ArtAsset\ENV\AreaProp\Station\InStructure\FBX)";
		config.countSelectionMax = 0; // ������

		IFILEDIALOG->OpenDialog("FBXDialog", "Select Non Anim Staticmesh FBX Files", ".fbx", config);
	}

	if (Button("Load Anim File"))
	{
		m_isAnim = true;
		savePath = "..\\..\\Client\\Bin\\Resources\\Models\\Bin_Anim";
		config.path = R"(Z:\Lie\Animations)";
		config.countSelectionMax = 0; // ������

		IFILEDIALOG->OpenDialog("FBXDialog", "Select Anim Skeletalmesh FBX Files", ".fbx", config);
	}
	
	if (Button("Load Anim File (Anim Only)"))
	{
		m_isAnimOnly = true;
		savePath = "..\\..\\Client\\Bin\\Resources\\Models\\Bin_Anim";
		config.path = R"(Z:\Lie\Animations)";
		config.countSelectionMax = 0; // ������

		IFILEDIALOG->OpenDialog("FBXDialog", "Select Anim Skeletalmesh FBX Files", ".fbx", config);
	}

	if (IFILEDIALOG->Display("FBXDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// ó��
			// first: ���ϸ�.Ȯ����
			// second: ��ü ��� (���ϸ�����)
			if (!selections.empty())
			{
				for (auto FilePath : selections)
				{
					if (m_isAnimOnly)
					{
						Convert_AnimOnly(FilePath.second.data());
						continue;
					}
					else
					{
						if (false == m_isAnim)
							Convert_NonAnimFBX(FilePath.second.data());
						else
							Convert_AnimFBX(FilePath.second.data());
					}
				}
			}
			Copy_MaterialTextures();
		}
		IFILEDIALOG->Close();
	}

	ImGui::End();
	return S_OK;
}

HRESULT CConvertTool::Convert_NonAnimFBX(const _char* pModelFilePath)
{
	path ModelPath = pModelFilePath;
	path saveFileName = savePath / ModelPath.stem();
	saveFileName.replace_extension(".bin");
	ofstream ofs(saveFileName, ios::binary);
	if (!ofs.is_open())
	{
		MSG_BOX("��ִ� ������ �����");
		return E_FAIL;
	}

	int		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast | aiProcess_PreTransformVertices;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("������� �������� !!! �������̾� !!!");
		return E_FAIL;
	}

	/********************����********************/
	/************** �޽� -> ���׸���**************/
	/*******************************************/

	/**********�޽� ����**********/
	Write_NonAnimMeshData(ofs);

	/**********���׸��� ����**********/
	Write_MaterialData(pModelFilePath, ofs);

	return S_OK;

}

HRESULT CConvertTool::Convert_AnimFBX(const _char* pModelFilePath)
{
	path ModelPath = pModelFilePath;
	path saveFileName = savePath / ModelPath.stem();
	saveFileName.replace_extension(".bin");
	ofstream ofs(saveFileName, ios::binary);
	if (!ofs.is_open())
	{
		MSG_BOX("�ִ� ������ �����");
		return E_FAIL;
	}

	int		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("������� �������� !!! �������̾� !!!");
		return E_FAIL;
	}


/********************����********************/
/**** �� -> �޽� -> ���׸��� -> �ִϸ��̼� ****/
/*******************************************/


/**********�� ����**********/
	vector<FBX_BONEDATA> Bones;
	Write_BoneData(m_pAIScene->mRootNode, -1, Bones, ofs);
	_uint BonesSize = _uint(Bones.size());
	ofs.write(reinterpret_cast<const char*>(&BonesSize), sizeof(_uint));	// �� �� ���� �����ؿ�
	for (size_t i = 0; i < BonesSize; i++)
	{
		_uint NameLength = _uint(Bones[i].strBoneName.length());
		ofs.write(reinterpret_cast<const char*>(&NameLength), sizeof(_uint));	// �� �̸� ���� �����ؿ�
		ofs.write(reinterpret_cast<const char*>(Bones[i].strBoneName.c_str()), NameLength);	// �� �̸� �����ؿ�
		ofs.write(reinterpret_cast<const char*>(&Bones[i].TransformMatrix), sizeof(_float4x4));	// ��� �����ؿ�
		ofs.write(reinterpret_cast<const char*>(&Bones[i].iParentBoneIndex), sizeof(_int));			// �θ� �� �ε��� �����ؿ�
	}

/**********�޽� ����**********/
	Write_AnimMeshData(Bones, ofs);

/**********���׸��� ����**********/
	Write_MaterialData(pModelFilePath, ofs);

/**********�ִϸ��̼� ����**********/
	Write_AnimationData(Bones, ofs);


	return S_OK;
}

HRESULT CConvertTool::Convert_AnimOnly(const _char* pModelFilePath)
{
	path ModelPath = pModelFilePath;
	path saveFileName = savePath / ModelPath.stem();
	saveFileName.replace_extension(".anim");
	ofstream ofs(saveFileName, ios::binary);
	if (!ofs.is_open())
	{
		MSG_BOX("�ִϸ��̼� ���� ������ �����");
		return E_FAIL;
	}

	int		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("������� �������� !!! �������̾� !!!");
		return E_FAIL;
	}

	vector<FBX_BONEDATA> Bones;
	Write_BoneData(m_pAIScene->mRootNode, -1, Bones, ofs);

	Write_AnimationData(Bones, ofs);

	return S_OK;
}

HRESULT CConvertTool::Copy_MaterialTextures()
{
	for (auto srcPath : materialList)
	{
		try
		{
			_char       szFileName[MAX_PATH] = {};
			_char       szExt[MAX_PATH] = {}; // ������ �̸��̶� Ȯ���ڸ� ��������
		
			_splitpath_s(srcPath.string().c_str(), nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

			string dstPath = savePath + "\\" + szFileName + szExt;

			if (false == copy_file(srcPath, dstPath, copy_options::overwrite_existing))
				continue;
		}	
		catch (const filesystem_error& e)
		{
			// 1) ���� ������ �ϳ��� std::string���� ����
			std::string msg = "���� ���� ����:\n";
			msg += e.what();
			msg += "\n\n�ڵ� ��: " + std::to_string(e.code().value());
			msg += "\n���� �޽���: " + e.code().message();
			msg += "\n���� ���: " + e.path1().string();
			msg += "\n��� ���: " + e.path2().string();

			// 2) MessageBoxA�� ���
			MessageBoxA( nullptr, msg.c_str(), "���� ���� ����", MB_OK | MB_ICONERROR );
			continue;
		}          
	}
	materialList.clear();
	return S_OK;
}


HRESULT CConvertTool::Write_BoneData(const aiNode* pAINode, _int iParentBoneIndex, vector<FBX_BONEDATA>& m_Bones, ostream& ofs)
{
	{ // �� Create
		FBX_BONEDATA tBone = {};
		tBone.strBoneName = pAINode->mName.C_Str();


		_float4x4 TransformationMatrix = {};
		memcpy(&TransformationMatrix, &pAINode->mTransformation, sizeof(_float4x4));
		XMStoreFloat4x4(&tBone.TransformMatrix, XMMatrixTranspose(XMLoadFloat4x4(&TransformationMatrix)));
		tBone.iParentBoneIndex = iParentBoneIndex;
		m_Bones.push_back(tBone);
	}

	++m_iCurNumBones;
	_int		iParentIndex = m_iCurNumBones - 1;

	for (size_t i = 0; i < pAINode->mNumChildren; i++){
		Write_BoneData(pAINode->mChildren[i], iParentIndex, m_Bones, ofs);
	}
	return S_OK;
}

HRESULT CConvertTool::Write_NonAnimMeshData(ostream& ofs)
{
	/**********�޽� ����**********/
	ofs.write(reinterpret_cast<const char*>(&m_pAIScene->mNumMeshes), sizeof(_uint)); // �޽� �?
	for (size_t i = 0; i < m_pAIScene->mNumMeshes; i++)
	{
		aiMesh* pAIMesh = m_pAIScene->mMeshes[i];

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.length), sizeof(_uint));			// �޽� �̸� ���� �����ؿ�
		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.data), pAIMesh->mName.length);		// �޽� �̸� �����ؿ�

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mMaterialIndex), sizeof(_uint));			// ���׸��� �ε��� �����ؿ�

		_uint iNumVertices = pAIMesh->mNumVertices;
		ofs.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(_uint));						// ���ؽ� ��� �����ؿ�

		_uint iNumIndices = pAIMesh->mNumFaces * 3;
		ofs.write(reinterpret_cast<const char*>(&iNumIndices), sizeof(_uint));						// �ε��� ��� �����ؿ�

		vector<VTXMESH> pVertices;
		pVertices.reserve(iNumVertices);
		for (size_t j = 0; j < iNumVertices; j++) 
		{
			VTXMESH tVertex = {};
			memcpy(&tVertex.vPosition, &pAIMesh->mVertices[j], sizeof(_float3));
			memcpy(&tVertex.vNormal, &pAIMesh->mNormals[j], sizeof(_float3));
			memcpy(&tVertex.vTangent, &pAIMesh->mTangents[j], sizeof(_float3));
			memcpy(&tVertex.vTexcoord, &pAIMesh->mTextureCoords[0][j], sizeof(_float2));
			pVertices.push_back(tVertex);
		}

		ofs.write(reinterpret_cast<const char*>(pVertices.data()), iNumVertices * sizeof(VTXMESH));		// ���ؽ� ���� ����ü �迭�� �����ؿ�

		vector<_uint> pIndices;
		pIndices.reserve(iNumIndices);

		for (size_t j = 0; j < m_pAIScene->mMeshes[i]->mNumFaces; j++)	// �ε��� ä����
		{
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[0]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[1]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[2]);
		}
		ofs.write(reinterpret_cast<const char*>(pIndices.data()), iNumIndices * sizeof(_uint));		// �ε��� ���� �迭�� ��¥�����ؿ�
	}

	return S_OK;
}

HRESULT CConvertTool::Write_AnimMeshData(const vector<FBX_BONEDATA>& Bones, ostream& ofs)
{
	/**********�޽� ����**********/
	ofs.write(reinterpret_cast<const char*>(&m_pAIScene->mNumMeshes), sizeof(_uint)); // �޽� �?
	for (size_t i = 0; i < m_pAIScene->mNumMeshes; i++)
	{
		aiMesh* pAIMesh = m_pAIScene->mMeshes[i];

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.length), sizeof(_uint));	// �޽� �̸� ���� �����ؿ�
		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.data), pAIMesh->mName.length);		// �޽� �̸� �����ؿ�

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mMaterialIndex), sizeof(_uint));	// ���׸��� �ε��� �����ؿ�

		_uint iNumVertices = pAIMesh->mNumVertices;
		ofs.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(_uint));	// ���ؽ� ��� �����ؿ�

		_uint iNumIndices = pAIMesh->mNumFaces * 3;
		ofs.write(reinterpret_cast<const char*>(&iNumIndices), sizeof(_uint));		// �ε��� ��� �����ؿ�

		vector<VTXANIMMESH> pVertices;
		pVertices.reserve(iNumVertices);
		for (_uint j = 0; j < iNumVertices; j++)
		{
			VTXANIMMESH tVertex = {};
			memcpy(&tVertex.vPosition, &pAIMesh->mVertices[j], sizeof(_float3));
			memcpy(&tVertex.vNormal, &pAIMesh->mNormals[j], sizeof(_float3));
			memcpy(&tVertex.vTangent, &pAIMesh->mTangents[j], sizeof(_float3));
			memcpy(&tVertex.vTexcoord, &pAIMesh->mTextureCoords[0][j], sizeof(_float2));
			pVertices.push_back(tVertex);
		}

		_uint iNumBones = pAIMesh->mNumBones;
		ofs.write(reinterpret_cast<const char*>(&iNumBones), sizeof(_uint));		// �� �޽��� ������ �ִ� ���� ��� �����ؿ�
		vector<_float4x4> OffsetMatrices;
		vector<_uint> BoneIndices;
		for (_uint j = 0; j < iNumBones; j++)
		{
			aiBone* pAIBone = pAIMesh->mBones[j];
			_float4x4		OffsetMatrix;
			memcpy(&OffsetMatrix, &pAIBone->mOffsetMatrix, sizeof(_float4x4));
			XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));

			OffsetMatrices.push_back(OffsetMatrix);

			_uint	iBoneIndex = {};
			auto	iter = find_if(Bones.begin(), Bones.end(), [&](FBX_BONEDATA Bone)->_bool
				{
					if (!strcmp(Bone.strBoneName.c_str(), pAIBone->mName.C_Str()))
						return true;

					++iBoneIndex;

					return false;
				});

			BoneIndices.push_back(iBoneIndex);
			_uint		iNumWeights = pAIBone->mNumWeights;
			//ofs.write(reinterpret_cast<const char*>(&iNumWeights), sizeof(_uint));		// ������ ���� ������ ��ĥ �� �ִ� ���ؽ��� ������ �����ؿ� /* i��° ���� � �������� ������ �ִµ�?*/

			for (_uint k = 0; k < iNumWeights; k++)
			{
				/* i��° ���� ���⤷�� �ִ� k��° ������ ���� */
				aiVertexWeight	AIWeight = pAIBone->mWeights[k];

				if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.x)
				{
					/* �� �޽ÿ��� ������ �ִ� ���� �� i��° ���� �� �������� ������ �ֳ�. */
					pVertices[AIWeight.mVertexId].vBlendIndices.x = j;
					pVertices[AIWeight.mVertexId].vBlendWeights.x = AIWeight.mWeight;
				}

				else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.y)
				{
					pVertices[AIWeight.mVertexId].vBlendIndices.y = j;
					pVertices[AIWeight.mVertexId].vBlendWeights.y = AIWeight.mWeight;
				}

				else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.z)
				{
					pVertices[AIWeight.mVertexId].vBlendIndices.z = j;
					pVertices[AIWeight.mVertexId].vBlendWeights.z = AIWeight.mWeight;
				}

				else if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.w)
				{
					pVertices[AIWeight.mVertexId].vBlendIndices.w = j;
					pVertices[AIWeight.mVertexId].vBlendWeights.w = AIWeight.mWeight;
				}
			}
			if (0 == iNumBones)
			{
				iNumBones = 1;

				_uint	iBoneIndex = {};

				auto	iter = find_if(Bones.begin(), Bones.end(), [&](FBX_BONEDATA Bone)->_bool
					{
						if (!strcmp(Bone.strBoneName.c_str(), pAIBone->mName.C_Str()))
							return true;

						++iBoneIndex;

						return false;
					});

				BoneIndices.push_back(iBoneIndex);

				_float4x4		OffsetMatrix;
				XMStoreFloat4x4(&OffsetMatrix, XMMatrixIdentity());
				OffsetMatrices.push_back(OffsetMatrix);
			}
		}

		_uint offSize = static_cast<_uint>(OffsetMatrices.size());
		ofs.write(reinterpret_cast<const char*>(&offSize), sizeof(_uint));									// ������ ��� �迭 ũ�� �����ؿ�
		ofs.write(reinterpret_cast<const char*>(OffsetMatrices.data()), sizeof(_float4x4) * offSize);		// ������ ��� �迭 �����ؿ�
		_uint BoneIndicesSize = static_cast<_uint>(BoneIndices.size());
		ofs.write(reinterpret_cast<const char*>(&BoneIndicesSize), sizeof(_uint));							// ������ ������ ��ġ�� ������ �ε����� ��������� ���� �迭  ũ��  �����ؿ�
		ofs.write(reinterpret_cast<const char*>(BoneIndices.data()), sizeof(_uint) * BoneIndicesSize);		// ������ ������ ��ġ�� ������ �ε����� ��������� ���� �迭 �����ؿ�

		ofs.write(reinterpret_cast<const char*>(pVertices.data()), iNumVertices * sizeof(VTXANIMMESH));		// ���ؽ� ���� ����ü �迭�� �����ؿ�

		vector<_uint> pIndices;
		pIndices.reserve(iNumIndices);

		for (size_t j = 0; j < m_pAIScene->mMeshes[i]->mNumFaces; j++)	// �ε��� ä����
		{
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[0]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[1]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[2]);
		}
		ofs.write(reinterpret_cast<const char*>(pIndices.data()), iNumIndices * sizeof(_uint));		// �ε��� ���� �迭�� ��¥�����ؿ�
	}
	// �ƴ� ���� �ȴ��ݾ� �̰� �� �޽����? ����, 

	return S_OK;
}

HRESULT CConvertTool::Write_MaterialData(const _char* pModelFilePath, ostream& ofs)
{
	/**********���׸��� ����**********/
	_uint iNumMaterials = m_pAIScene->mNumMaterials;
	ofs.write(reinterpret_cast<const char*>(&iNumMaterials), sizeof(_uint));			// ���׸��� ��� �����ؿ�
	for (size_t i = 0; i < iNumMaterials; ++i)											// ���׸��� ��ü �� ���ƿ�
	{
		aiMaterial* pMaterial = m_pAIScene->mMaterials[i];
		vector<FBX_MATDATA> tMaterialData;
		for (_uint j = 1; j < AI_TEXTURE_TYPE_MAX; j++)				// �������Ŀ�
		{
			_uint iNumSRVs = pMaterial->GetTextureCount(static_cast<aiTextureType>(j));
			for (size_t k = 0; k < iNumSRVs; k++)
			{
				FBX_MATDATA tMat = {};
				aiString     strTexturePath;
				if (FAILED(m_pAIScene->mMaterials[i]->GetTexture(static_cast<aiTextureType>(j), static_cast<_uint>(k), &strTexturePath)))
					return E_FAIL;

				_char       szFullPath[MAX_PATH] = {};
				_char       szFileName[MAX_PATH] = {};
				_char       szDriveName[MAX_PATH] = {};
				_char       szDirName[MAX_PATH] = {};
				_char       szExt[MAX_PATH] = {}; // ������ �̸��̶� Ȯ���ڸ� ��������

				_char		szFBXName[MAX_PATH] = {};


				_splitpath_s(pModelFilePath, szDriveName, MAX_PATH, szDirName, MAX_PATH, szFBXName, MAX_PATH, nullptr, 0);
				_splitpath_s(strTexturePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

				path relPath = strTexturePath.data;
				path fbxTexturePath = (path)szDriveName / szDirName;
				fbxTexturePath /= relPath;
				fbxTexturePath = fbxTexturePath.lexically_normal();

				materialList.push_back(fbxTexturePath); // ���߿� ������ �� ����ٿ��ֱ� ���� �ؽ��ĵ� ��� ��Ƶ��� 

				strcat_s(szFullPath, szFileName);
				strcat_s(szFullPath, szExt);


				tMat.strTexturePath = szFullPath;					// �ؽ��� �̸��̶� Ȯ���ڸ� ����
				tMat.eTexType = static_cast<aiTextureType>(j);		// �ؽ���Ÿ�� �������� �����Ͻð�
				tMaterialData.push_back(tMat);

			}
		}
		_uint numSRVs = static_cast<_uint>(tMaterialData.size());
		ofs.write(reinterpret_cast<const char*>(&numSRVs), sizeof(_uint));						// ���׸��� �ȿ� srv�� � �ִ��� �����ؿ�
		for (auto matdata : tMaterialData)
		{
			_uint pathLength = static_cast<_uint>(matdata.strTexturePath.size());
			ofs.write(reinterpret_cast<const char*>(&pathLength), sizeof(_uint));				// ���ڿ� ���� �����ؿ�
			ofs.write(matdata.strTexturePath.data(), pathLength);								// ���ڿ� �����ؿ�

			_uint texType = static_cast<_uint>(matdata.eTexType);
			ofs.write(reinterpret_cast<const char*>(&texType), sizeof(_uint));					// �ؽ��� Ÿ�� �����ؿ�
		}
	}

	return S_OK;
}

HRESULT CConvertTool::Write_AnimationData(const vector<FBX_BONEDATA>& Bones, ostream& ofs)
{
	/**********�ִϸ��̼� ����**********/
	// Ready_Animations
	_uint iNumAnimations = m_pAIScene->mNumAnimations;
	ofs.write(reinterpret_cast<const char*>(&iNumAnimations), sizeof(_uint));			// �ִϸ��̼� ��� �����ؿ�
	for (size_t i = 0; i < iNumAnimations; i++) 
	{
		//  CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
		aiAnimation* pAIAnimation = m_pAIScene->mAnimations[i];

		_float tickspersec = (_float)pAIAnimation->mTicksPerSecond;
		_float duration = (_float)pAIAnimation->mDuration;
		string AnimationName = pAIAnimation->mName.C_Str(); 

		_uint nameLength = static_cast<_uint>(AnimationName.length());
		ofs.write(reinterpret_cast<const char*>(&nameLength), sizeof(_uint));        // ���ڿ� ���� ���� ����
		ofs.write(AnimationName.c_str(), nameLength);

		ofs.write(reinterpret_cast<const char*>(&tickspersec), sizeof(_float));					// �����ؾ��ұ� �ʴ�ƽ
		ofs.write(reinterpret_cast<const char*>(&duration), sizeof(_float));					// �ִϸ��̼� �� ����

		_uint iNumChannels = pAIAnimation->mNumChannels;
		ofs.write(reinterpret_cast<const char*>(&iNumChannels), sizeof(_uint));			// �� �ִϸ��̼��� ��Ʈ���ؾ��ϴ� ���� ����
		for (size_t j = 0; j < iNumChannels; j++)
		{
			// CChannel::Create(pAIAnimation->mChannels[i], Bones);
			aiNodeAnim* pAIChannel = pAIAnimation->mChannels[j];



			/* �̰Ÿ� �����ϸ� �Ǵ°� �ƴ�? �ƴԸ��� */
/**/		_uint m_iNumKeyFrames = {};				// �� Ű������ �� (�ؿ� �� ����)
/**/		vector<KEYFRAME> m_KeyFrames;			// Ű������ �A�A ���� ���� �����̳�
/**/		_uint m_iBoneIndex = {};				// �� ä���� ��ü �� �迭���� ���° �ε����� ģ�������� �ԶǴ���



			m_iNumKeyFrames = max(pAIChannel->mNumPositionKeys, pAIChannel->mNumScalingKeys);
			m_iNumKeyFrames = max(pAIChannel->mNumRotationKeys, m_iNumKeyFrames);

			m_KeyFrames.reserve(m_iNumKeyFrames);

			_float3		vScale = {};
			_float4		vRotation = {};
			_float3		vTranslation = {};

			for (size_t i = 0; i < m_iNumKeyFrames; i++)
			{
				KEYFRAME		KeyFrame{};

				if (i < pAIChannel->mNumScalingKeys)
				{
					memcpy(&vScale, &pAIChannel->mScalingKeys[i].mValue, sizeof(_float3));
					KeyFrame.fTrackPosition = (float)pAIChannel->mScalingKeys[i].mTime;
				}

				if (i < pAIChannel->mNumRotationKeys)
				{
					vRotation.x = pAIChannel->mRotationKeys[i].mValue.x;
					vRotation.y = pAIChannel->mRotationKeys[i].mValue.y;
					vRotation.z = pAIChannel->mRotationKeys[i].mValue.z;
					vRotation.w = pAIChannel->mRotationKeys[i].mValue.w;
					KeyFrame.fTrackPosition = (float)pAIChannel->mRotationKeys[i].mTime;
				}

				if (i < pAIChannel->mNumPositionKeys)
				{
					memcpy(&vTranslation, &pAIChannel->mPositionKeys[i].mValue, sizeof(_float3));
					KeyFrame.fTrackPosition = (float)pAIChannel->mPositionKeys[i].mTime;
				}

				KeyFrame.vScale = vScale;
				KeyFrame.vRotation = vRotation;
				KeyFrame.vTranslation = vTranslation;

				m_KeyFrames.push_back(KeyFrame);
			}

			auto	iter = find_if(Bones.begin(), Bones.end(), [&](FBX_BONEDATA Bone)->_bool
				{
					if (!strcmp(Bone.strBoneName.c_str(), pAIChannel->mNodeName.data))
						return true;

					++m_iBoneIndex;

					return false;
				});


			ofs.write(reinterpret_cast<const char*>(&m_iNumKeyFrames), sizeof(_uint));			// �� Ű������ �� 
			ofs.write(reinterpret_cast<const char*>(m_KeyFrames.data()), sizeof(KEYFRAME) * m_iNumKeyFrames);			// �� Ű������ (������)
			ofs.write(reinterpret_cast<const char*>(&m_iBoneIndex), sizeof(_uint));				// �� ä���� ��ü �� �迭���� ���° �ε����� ģ��������
		}
	}
	return S_OK;
}


CConvertTool* CConvertTool::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, void* pArg)
{
	CConvertTool* pInstance = new CConvertTool(pDevice, pContext);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Created : CConvertTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CGameObject* CConvertTool::Clone(void* pArg)
{
	CConvertTool* pInstance = new CConvertTool(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CConvertTool");
		Safe_Release(pInstance);
	}

	return pInstance;
}


void CConvertTool::Free()
{
	__super::Free();

}
