#include "GameInstance.h"
#include "ConvertTool.h"

#include <filesystem>
using namespace std::filesystem;
//ImGuiFileDialog g_ImGuiFileDialog;
//ImGuiFileDialog::Instance() 이래 싱글톤으로 쓰라고 신이 말하고 감


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
		config.countSelectionMax = 0; // 무제한

		IFILEDIALOG->OpenDialog("FBXDialog", "Select Non Anim Staticmesh FBX Files", ".fbx", config);
	}

	if (Button("Load Anim File"))
	{
		m_isAnim = true;
		savePath = "..\\..\\Client\\Bin\\Resources\\Models\\Bin_Anim";
		config.path = R"(Z:\Lie\Animations)";
		config.countSelectionMax = 0; // 무제한

		IFILEDIALOG->OpenDialog("FBXDialog", "Select Anim Skeletalmesh FBX Files", ".fbx", config);
	}
	
	if (Button("Load Anim File (Anim Only)"))
	{
		m_isAnimOnly = true;
		savePath = "..\\..\\Client\\Bin\\Resources\\Models\\Bin_Anim";
		config.path = R"(Z:\Lie\Animations)";
		config.countSelectionMax = 0; // 무제한

		IFILEDIALOG->OpenDialog("FBXDialog", "Select Anim Skeletalmesh FBX Files", ".fbx", config);
	}

	if (IFILEDIALOG->Display("FBXDialog"))
	{
		if (IFILEDIALOG->IsOk())
		{
			auto selections = IFILEDIALOG->GetSelection();
			// 처리
			// first: 파일명.확장자
			// second: 전체 경로 (파일명포함)
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
		MSG_BOX("논애님 폴더가 없어요");
		return E_FAIL;
	}

	int		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast | aiProcess_PreTransformVertices;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("어심프가 쓰러졌다 !!! 무슨일이야 !!!");
		return E_FAIL;
	}

	/********************순서********************/
	/************** 메쉬 -> 머테리얼**************/
	/*******************************************/

	/**********메쉬 저장**********/
	Write_NonAnimMeshData(ofs);

	/**********머테리얼 저장**********/
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
		MSG_BOX("애님 폴더가 없어요");
		return E_FAIL;
	}

	int		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("어심프가 쓰러졌다 !!! 무슨일이야 !!!");
		return E_FAIL;
	}


/********************순서********************/
/**** 본 -> 메쉬 -> 머테리얼 -> 애니메이션 ****/
/*******************************************/


/**********본 저장**********/
	vector<FBX_BONEDATA> Bones;
	Write_BoneData(m_pAIScene->mRootNode, -1, Bones, ofs);
	_uint BonesSize = _uint(Bones.size());
	ofs.write(reinterpret_cast<const char*>(&BonesSize), sizeof(_uint));	// 뼈 총 갯수 저장해요
	for (size_t i = 0; i < BonesSize; i++)
	{
		_uint NameLength = _uint(Bones[i].strBoneName.length());
		ofs.write(reinterpret_cast<const char*>(&NameLength), sizeof(_uint));	// 뼈 이름 길이 저장해요
		ofs.write(reinterpret_cast<const char*>(Bones[i].strBoneName.c_str()), NameLength);	// 뼈 이름 저장해요
		ofs.write(reinterpret_cast<const char*>(&Bones[i].TransformMatrix), sizeof(_float4x4));	// 행렬 저장해요
		ofs.write(reinterpret_cast<const char*>(&Bones[i].iParentBoneIndex), sizeof(_int));			// 부모 뼈 인덱스 저장해요
	}

/**********메쉬 저장**********/
	Write_AnimMeshData(Bones, ofs);

/**********머테리얼 저장**********/
	Write_MaterialData(pModelFilePath, ofs);

/**********애니메이션 저장**********/
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
		MSG_BOX("애니메이션 저장 폴더가 없어요");
		return E_FAIL;
	}

	int		iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;


	m_pAIScene = m_Importer.ReadFile(pModelFilePath, iFlag);

	if (nullptr == m_pAIScene)
	{
		MSG_BOX("어심프가 쓰러졌다 !!! 무슨일이야 !!!");
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
			_char       szExt[MAX_PATH] = {}; // 저장은 이름이랑 확장자만 저장하자
		
			_splitpath_s(srcPath.string().c_str(), nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

			string dstPath = savePath + "\\" + szFileName + szExt;

			if (false == copy_file(srcPath, dstPath, copy_options::overwrite_existing))
				continue;
		}	
		catch (const filesystem_error& e)
		{
			// 1) 에러 정보를 하나의 std::string으로 조합
			std::string msg = "파일 복사 실패:\n";
			msg += e.what();
			msg += "\n\n코드 값: " + std::to_string(e.code().value());
			msg += "\n에러 메시지: " + e.code().message();
			msg += "\n원본 경로: " + e.path1().string();
			msg += "\n대상 경로: " + e.path2().string();

			// 2) MessageBoxA로 출력
			MessageBoxA( nullptr, msg.c_str(), "파일 복사 오류", MB_OK | MB_ICONERROR );
			continue;
		}          
	}
	materialList.clear();
	return S_OK;
}


HRESULT CConvertTool::Write_BoneData(const aiNode* pAINode, _int iParentBoneIndex, vector<FBX_BONEDATA>& m_Bones, ostream& ofs)
{
	{ // 뼈 Create
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
	/**********메쉬 저장**********/
	ofs.write(reinterpret_cast<const char*>(&m_pAIScene->mNumMeshes), sizeof(_uint)); // 메쉬 몇개?
	for (size_t i = 0; i < m_pAIScene->mNumMeshes; i++)
	{
		aiMesh* pAIMesh = m_pAIScene->mMeshes[i];

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.length), sizeof(_uint));			// 메쉬 이름 길이 저장해요
		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.data), pAIMesh->mName.length);		// 메쉬 이름 저장해요

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mMaterialIndex), sizeof(_uint));			// 머테리얼 인덱스 저장해요

		_uint iNumVertices = pAIMesh->mNumVertices;
		ofs.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(_uint));						// 버텍스 몇갠지 저장해요

		_uint iNumIndices = pAIMesh->mNumFaces * 3;
		ofs.write(reinterpret_cast<const char*>(&iNumIndices), sizeof(_uint));						// 인덱스 몇갠지 저장해요

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

		ofs.write(reinterpret_cast<const char*>(pVertices.data()), iNumVertices * sizeof(VTXMESH));		// 버텍스 벡터 구조체 배열도 저장해요

		vector<_uint> pIndices;
		pIndices.reserve(iNumIndices);

		for (size_t j = 0; j < m_pAIScene->mMeshes[i]->mNumFaces; j++)	// 인덱스 채워요
		{
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[0]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[1]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[2]);
		}
		ofs.write(reinterpret_cast<const char*>(pIndices.data()), iNumIndices * sizeof(_uint));		// 인덱스 벡터 배열로 통짜저장해요
	}

	return S_OK;
}

HRESULT CConvertTool::Write_AnimMeshData(const vector<FBX_BONEDATA>& Bones, ostream& ofs)
{
	/**********메쉬 저장**********/
	ofs.write(reinterpret_cast<const char*>(&m_pAIScene->mNumMeshes), sizeof(_uint)); // 메쉬 몇개?
	for (size_t i = 0; i < m_pAIScene->mNumMeshes; i++)
	{
		aiMesh* pAIMesh = m_pAIScene->mMeshes[i];

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.length), sizeof(_uint));	// 메쉬 이름 길이 저장해요
		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mName.data), pAIMesh->mName.length);		// 메쉬 이름 저장해요

		ofs.write(reinterpret_cast<const char*>(&pAIMesh->mMaterialIndex), sizeof(_uint));	// 머테리얼 인덱스 저장해요

		_uint iNumVertices = pAIMesh->mNumVertices;
		ofs.write(reinterpret_cast<const char*>(&iNumVertices), sizeof(_uint));	// 버텍스 몇갠지 저장해요

		_uint iNumIndices = pAIMesh->mNumFaces * 3;
		ofs.write(reinterpret_cast<const char*>(&iNumIndices), sizeof(_uint));		// 인덱스 몇갠지 저장해요

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
		ofs.write(reinterpret_cast<const char*>(&iNumBones), sizeof(_uint));		// 이 메쉬에 영향을 주는 뼈가 몇갠지 저장해요
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
			//ofs.write(reinterpret_cast<const char*>(&iNumWeights), sizeof(_uint));		// 지금의 뼈가 영향을 끼칠 수 있는 버텍스의 갯수를 저장해요 /* i번째 뼈가 몇개 정점에게 영향을 주는데?*/

			for (_uint k = 0; k < iNumWeights; k++)
			{
				/* i번째 뼈가 영향ㅇ르 주는 k번째 정점의 정보 */
				aiVertexWeight	AIWeight = pAIBone->mWeights[k];

				if (0.f == pVertices[AIWeight.mVertexId].vBlendWeights.x)
				{
					/* 이 메시에게 영향을 주는 뼈들 중 i번째 뼈가 이 정점에게 영향을 주네. */
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
		ofs.write(reinterpret_cast<const char*>(&offSize), sizeof(_uint));									// 오프셋 행렬 배열 크기 저장해요
		ofs.write(reinterpret_cast<const char*>(OffsetMatrices.data()), sizeof(_float4x4) * offSize);		// 오프셋 행렬 배열 저장해요
		_uint BoneIndicesSize = static_cast<_uint>(BoneIndices.size());
		ofs.write(reinterpret_cast<const char*>(&BoneIndicesSize), sizeof(_uint));							// 나한테 영향을 미치는 뼈새끼 인덱스가 몇번인지에 대한 배열  크기  저장해요
		ofs.write(reinterpret_cast<const char*>(BoneIndices.data()), sizeof(_uint) * BoneIndicesSize);		// 나한테 영향을 미치는 뼈새끼 인덱스가 몇번인지에 대한 배열 저장해요

		ofs.write(reinterpret_cast<const char*>(pVertices.data()), iNumVertices * sizeof(VTXANIMMESH));		// 버텍스 벡터 구조체 배열도 저장해요

		vector<_uint> pIndices;
		pIndices.reserve(iNumIndices);

		for (size_t j = 0; j < m_pAIScene->mMeshes[i]->mNumFaces; j++)	// 인덱스 채워요
		{
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[0]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[1]);
			pIndices.push_back(pAIMesh->mFaces[j].mIndices[2]);
		}
		ofs.write(reinterpret_cast<const char*>(pIndices.data()), iNumIndices * sizeof(_uint));		// 인덱스 벡터 배열로 통짜저장해요
	}
	// 아니 말이 안대잖아 이게 다 메쉬라고? 으윽, 

	return S_OK;
}

HRESULT CConvertTool::Write_MaterialData(const _char* pModelFilePath, ostream& ofs)
{
	/**********머테리얼 저장**********/
	_uint iNumMaterials = m_pAIScene->mNumMaterials;
	ofs.write(reinterpret_cast<const char*>(&iNumMaterials), sizeof(_uint));			// 머테리얼 몇갠지 저장해요
	for (size_t i = 0; i < iNumMaterials; ++i)											// 머테리얼 전체 다 돌아요
	{
		aiMaterial* pMaterial = m_pAIScene->mMaterials[i];
		vector<FBX_MATDATA> tMaterialData;
		for (_uint j = 1; j < AI_TEXTURE_TYPE_MAX; j++)				// 죳뺑이쳐요
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
				_char       szExt[MAX_PATH] = {}; // 저장은 이름이랑 확장자만 저장하자

				_char		szFBXName[MAX_PATH] = {};


				_splitpath_s(pModelFilePath, szDriveName, MAX_PATH, szDirName, MAX_PATH, szFBXName, MAX_PATH, nullptr, 0);
				_splitpath_s(strTexturePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

				path relPath = strTexturePath.data;
				path fbxTexturePath = (path)szDriveName / szDirName;
				fbxTexturePath /= relPath;
				fbxTexturePath = fbxTexturePath.lexically_normal();

				materialList.push_back(fbxTexturePath); // 나중에 저장할 때 복사붙여넣기 해줄 텍스쳐들 경로 모아두자 

				strcat_s(szFullPath, szFileName);
				strcat_s(szFullPath, szExt);


				tMat.strTexturePath = szFullPath;					// 텍스쳐 이름이랑 확장자만 저장
				tMat.eTexType = static_cast<aiTextureType>(j);		// 텍스쳐타입 뭐였는지 저장하시고
				tMaterialData.push_back(tMat);

			}
		}
		_uint numSRVs = static_cast<_uint>(tMaterialData.size());
		ofs.write(reinterpret_cast<const char*>(&numSRVs), sizeof(_uint));						// 머테리얼 안에 srv가 몇개 있는지 저장해요
		for (auto matdata : tMaterialData)
		{
			_uint pathLength = static_cast<_uint>(matdata.strTexturePath.size());
			ofs.write(reinterpret_cast<const char*>(&pathLength), sizeof(_uint));				// 문자열 길이 저장해요
			ofs.write(matdata.strTexturePath.data(), pathLength);								// 문자열 저장해요

			_uint texType = static_cast<_uint>(matdata.eTexType);
			ofs.write(reinterpret_cast<const char*>(&texType), sizeof(_uint));					// 텍스쳐 타입 저장해요
		}
	}

	return S_OK;
}

HRESULT CConvertTool::Write_AnimationData(const vector<FBX_BONEDATA>& Bones, ostream& ofs)
{
	/**********애니메이션 저장**********/
	// Ready_Animations
	_uint iNumAnimations = m_pAIScene->mNumAnimations;
	ofs.write(reinterpret_cast<const char*>(&iNumAnimations), sizeof(_uint));			// 애니메이션 몇갠지 저장해요
	for (size_t i = 0; i < iNumAnimations; i++) 
	{
		//  CAnimation::Create(m_pAIScene->mAnimations[i], m_Bones);
		aiAnimation* pAIAnimation = m_pAIScene->mAnimations[i];

		_float tickspersec = (_float)pAIAnimation->mTicksPerSecond;
		_float duration = (_float)pAIAnimation->mDuration;
		string AnimationName = pAIAnimation->mName.C_Str(); 

		_uint nameLength = static_cast<_uint>(AnimationName.length());
		ofs.write(reinterpret_cast<const char*>(&nameLength), sizeof(_uint));        // 문자열 길이 먼저 저장
		ofs.write(AnimationName.c_str(), nameLength);

		ofs.write(reinterpret_cast<const char*>(&tickspersec), sizeof(_float));					// 뭐라해야할까 초당틱
		ofs.write(reinterpret_cast<const char*>(&duration), sizeof(_float));					// 애니메이션 총 길이

		_uint iNumChannels = pAIAnimation->mNumChannels;
		ofs.write(reinterpret_cast<const char*>(&iNumChannels), sizeof(_uint));			// 이 애니메이션이 컨트롤해야하는 뼈의 갯수
		for (size_t j = 0; j < iNumChannels; j++)
		{
			// CChannel::Create(pAIAnimation->mChannels[i], Bones);
			aiNodeAnim* pAIChannel = pAIAnimation->mChannels[j];



			/* 이거만 저장하면 되는거 아님? 아님말고 */
/**/		_uint m_iNumKeyFrames = {};				// 총 키프레임 수 (밑에 놈 갯수)
/**/		vector<KEYFRAME> m_KeyFrames;			// 키프레임 쌐쌐 모은 벡터 컨테이너
/**/		_uint m_iBoneIndex = {};				// 이 채널이 전체 뼈 배열에서 몇번째 인덱스인 친구였는지 게또다제



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


			ofs.write(reinterpret_cast<const char*>(&m_iNumKeyFrames), sizeof(_uint));			// 총 키프레임 수 
			ofs.write(reinterpret_cast<const char*>(m_KeyFrames.data()), sizeof(KEYFRAME) * m_iNumKeyFrames);			// 총 키프레임 (데이터)
			ofs.write(reinterpret_cast<const char*>(&m_iBoneIndex), sizeof(_uint));				// 이 채널이 전체 뼈 배열에서 몇번째 인덱스인 친구였는지
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
