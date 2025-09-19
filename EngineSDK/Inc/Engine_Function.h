#pragma once

namespace Engine
{
	template<typename T>
	void Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete[] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	unsigned int Safe_AddRef(T& Instance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != Instance)
			iRefCnt = Instance->AddRef();

		return iRefCnt;
	}

	template<typename T>
	unsigned int Safe_Release(T& Instance)
	{
		unsigned int iRefCnt = { 0 };

		if (nullptr != Instance)
		{
			iRefCnt = Instance->Release();
			if (0 == iRefCnt)
				Instance = nullptr;
		}
		return iRefCnt;
	}

	static string WStringToString(const _wstring& wstr)
	{
		if (wstr.empty()) return {};
		int sizeNeeded = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int)wstr.size(), &result[0], sizeNeeded, nullptr, nullptr);
		return result;
	}

	static wstring StringToWString(const string& str)
	{
		if (str.empty()) return {};

		int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), nullptr, 0);
		wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int)str.size(), &result[0], sizeNeeded);
		return result;
	}

	static string WStringToStringU8(const _wstring& wstr)
	{
		if (wstr.empty()) return {};
		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		string result(sizeNeeded, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], sizeNeeded, nullptr, nullptr);
		return result;
	}

	static wstring StringToWStringU8(const string& str)
	{
		if (str.empty()) return {};

		int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
		wstring result(sizeNeeded, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &result[0], sizeNeeded);
		return result;
	}

	static PxVec3 VectorToPxVec3(const _vector& vec)
	{
		PxVec3 pxVec3(XMVectorGetX(vec), XMVectorGetY(vec), XMVectorGetZ(vec));
		return pxVec3;
	}

	static _vector PxVec3ToVector(const PxVec3& pxVec)
	{
		_vector vec = XMVectorSet(pxVec.x, pxVec.y, pxVec.z, 1.f);
		return vec;
	}

	static _vector VecSetW(const vector<float>& v, float w = 0.f) {
		return XMVectorSet(
			v.size() > 0 ? v[0] : 0.f,
			v.size() > 1 ? v[1] : 0.f,
			v.size() > 2 ? v[2] : 0.f,
			w
		);
	}
	static _float3 VecToFloat3(const vector<float>& v) {
		return _float3{
			v.size() > 0 ? v[0] : 0.f,
			v.size() > 1 ? v[1] : 0.f,
			v.size() > 2 ? v[2] : 0.f
		};
	}

	static _vector Float3ToVec(const _float3& f, float w = 0.f) {
		return XMVectorSet(f.x, f.y, f.z, w);
	}

#include <filesystem>
#include <string>
	namespace fs = std::filesystem;
	// 절대 경로 -> 상대 경로 변환
	static std::string ToRelativePath(const std::string& absolutePath)
	{
		fs::path absPath = fs::absolute(absolutePath);
		fs::path currentDir = fs::current_path();

		// 상대 경로 반환
		return fs::relative(absPath, currentDir).string();
	}
}
