#pragma once
#include "Base.h"

class TerrainEditor : public Transform
{
private:
	typedef VTXMESH VertexType;
	//
public:
	class BrushBuffer : public ConstBuffer
	{
	public:
		struct Data
		{
			int type;
			Float3 location;
			float range;
			Float3 color;
		}data;
		//
		BrushBuffer() : ConstBuffer(&data, sizeof(Data))
		{
			data.type = 0;
			data.location = Float3(0, 0, 0);
			data.range = 10.0f;
			data.color = Float3(0.0f, 0.5f, 0.0f);
		}
	};
	//
	struct InputDesc
	{
		UINT index;
		Float3 v0, v1, v2;
	};
	//
	struct OutputDesc
	{
		int picked;
		float u, v, distance;
	};
	////////////////////////////////////
	Material* material;
	Mesh* mesh;
	////////////////////////////////////
	vector<VertexType> vertices;
	vector<UINT> indices;
	////////////////////////////////////
	UINT width, height;
	Texture* heightMap;
	////////////////////////////////////
	ComputeShader* computeShader;
	RayBuffer* rayBuffer;
	StructuredBuffer* structuredBuffer;
	InputDesc* input;
	OutputDesc* output;
	//
	UINT size;
	////////////////////////////////////
	BrushBuffer* brushBuffer;
	//
	bool isRaise;
	float adjustValue;
	////////////////////////////////////
	bool onDebugWindow;
	////////////////////////////////////
public:
	TerrainEditor(UINT width = 100, UINT height = 100);
	~TerrainEditor();

	void Update();
	void Render();
	void PostRender();

	bool ComputePicking(OUT Vector3* position);
	//
	void AdjustY(Vector3 position);
	//
	void Save();
	void Load();
	////////////////////////////////////
private:
	void CreateMesh();
	void CreateNormal();
	void CreateTangent();
	void CreateCompute();
};
////////////////////////////////////////////////////////////////////////
#include "Framework.h"
#include "TerrainEditor.h"

TerrainEditor::TerrainEditor(UINT width, UINT height)
	: width(width), height(height),
	isRaise(true), adjustValue(50.0f),
	onDebugWindow(false)
{
	material = new Material(L"TerrainBrush");
	material->SetDiffuseMap(L"Textures/Landscape/Dirt2.png");

	heightMap = Texture::Add(L"Textures/HeightMaps/HeightMap256.png");
	//
	CreateMesh();
	CreateCompute();
	//
	brushBuffer = new BrushBuffer;
}

TerrainEditor::~TerrainEditor()
{
	delete brushBuffer;
	//
	delete material;
	delete mesh;

	delete rayBuffer;
	delete structuredBuffer;

	delete[] input;
	delete[] output;
}

void TerrainEditor::Update()
{
	if (onDebugWindow == false)
	{
		if (KEY_PRESS(VK_LBUTTON))
			AdjustY(brushBuffer->data.location);
		if (KEY_UP(VK_LBUTTON))
		{
			CreateNormal();
			CreateTangent();
			mesh->UpdateVertex(vertices.data(), vertices.size());
		}
	}
	//
	UpdateWorld();
}

void TerrainEditor::Render()
{
	mesh->IASet();
	worldBuffer->SetVSBuffer(0);
	brushBuffer->SetPSBuffer(10);
	material->Set();
	//
	DC->DrawIndexed((UINT)indices.size(), 0, 0);
}

void TerrainEditor::PostRender()
{
	/* Debug */
	if(ImGui::IsWindowFocused())
		onDebugWindow = true;
	else
		onDebugWindow = false;
	//
	ImGui::Checkbox("onDebugWindow", &onDebugWindow);
	//
	Vector3 temp{};
	ComputePicking(&temp);
	brushBuffer->data.location = temp;
	//
	ImGui::Spacing();
	ImGui::Text("[Terrain Editor]");
	ImGui::SliderInt("Type",
		&brushBuffer->data.type, 0, 1);
	ImGui::SliderFloat("Range",
		&brushBuffer->data.range, 1.0f, 50.0f);
	ImGui::ColorEdit3("Color",
		(float*)&brushBuffer->data.color);
	ImGui::Checkbox("Raise", &isRaise);
	//
	if (ImGui::Button("Save", { 64.0f, 32.0f }))
		Save();
	if (ImGui::Button("Load", { 64.0f, 32.0f }))
	{
		Load();
		//
		CreateNormal();
		CreateTangent();
		mesh->UpdateVertex(vertices.data(), vertices.size());
	}
}

bool TerrainEditor::ComputePicking(OUT Vector3* position)
{
	Ray ray = CAMERA->ScreenPointToRay(MOUSEPOS);

	rayBuffer->data.position = ray.position;
	rayBuffer->data.direction = ray.direction;
	rayBuffer->data.size = size;
	//
	computeShader->Set();
	rayBuffer->SetCSBuffer(0);
	//
	DC->CSSetShaderResources(0, 1, &structuredBuffer->GetSRV());
	DC->CSSetUnorderedAccessViews(0, 1, &structuredBuffer->GetUAV(), nullptr);

	UINT x = ceil((float)size / 1024.0f);

	DC->Dispatch(x, 1, 1);

	structuredBuffer->Copy(output, sizeof(OutputDesc) * size);

	float minDistance = FLT_MAX;
	int minIndex = -1;

	for (UINT i = 0; i < size; i++)
	{
		OutputDesc temp = output[i];
		if (temp.picked)
		{
			if (minDistance > temp.distance)
			{
				minDistance = temp.distance;
				minIndex = i;
			}
		}
	}

	if (minIndex >= 0)
	{
		*position = ray.position + ray.direction * minDistance;
		return true;
	}

	return false;
}

void TerrainEditor::AdjustY(Vector3 position)
{
	switch (brushBuffer->data.type)
	{
	case 0:
	{
		for (VertexType& vertex : vertices)
		{
			Vector3 p1 = Vector3(vertex.position.x, 0, vertex.position.z);
			Vector3 p2 = Vector3(position.x, 0, position.z);
			//
			float distance = (p2 - p1).Length();
			//
			float cosValue = cos(XM_PIDIV2 * distance / brushBuffer->data.range);
			float temp = adjustValue * max(0, cosValue);
			//
			if (distance <= brushBuffer->data.range)
			{
				if (isRaise)
					vertex.position.y += temp * DELTA;
				else
					vertex.position.y -= temp * DELTA;
			}
		}
	}
	break;
	case 1:
	{
		for (VertexType& vertex : vertices)
		{
			Vector3 p1 = Vector3(vertex.position.x, 0, vertex.position.z);
			Vector3 p2 = Vector3(position.x, 0, position.z);
			//
			float distX = abs(p2.x - p1.x);
			float distZ = abs(p2.z - p1.z);
			//
			if (distX <= brushBuffer->data.range
				&& distZ <= brushBuffer->data.range)
			{
				if (isRaise)
					vertex.position.y += adjustValue * DELTA;
				else
					vertex.position.y -= adjustValue * DELTA;
			}
		}
	}
	break;
	default:
		break;
	}
	//
	mesh->UpdateVertex(vertices.data(), vertices.size());
}

void TerrainEditor::Save()
{
	/* Create Filesystem */
	HANDLE file;
	DWORD write; // unsigned long
	file = CreateFile(L"MapFiles/mapData.map", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	/* Save Cubes */
	{
		int count = vertices.size();
		V(WriteFile(file, &count, sizeof(int), &write, nullptr));
		//
		Vector3 data{};
		for (int i = 0; i < count; ++i)
		{
			data = vertices[i].position;
			V(WriteFile(file, &data, sizeof(Vector3), &write, nullptr));
		}
	}
	/* Save End */
	CloseHandle(file);
}
void TerrainEditor::Load()
{
	/* Create Filesystem */
	HANDLE file;
	DWORD read; // unsigned long
	file = CreateFile(L"MapFiles/mapData.map", GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	/* Load Cube Data */
	{
		int count{};
		V(ReadFile(file, &count, sizeof(int), &read, nullptr));
		//
		Vector3 data{};
		for (int i = 0; i < count; ++i)
		{
			V(ReadFile(file, &data, sizeof(Vector3), &read, nullptr));
			vertices[i].position = data;
		}
	}
	/* Load End */
	CloseHandle(file);
}

void TerrainEditor::CreateMesh()
{
	//Vertices
	for (UINT z = 0; z <= height; z++)
	{
		for (UINT x = 0; x <= width; x++)
		{
			VertexType vertex;
			vertex.position = Float3((float)x, 0.0f, (float)z);
			vertex.uv = Float2(x / (float)width, 1.0f - z / (float)height);

			UINT index = (width + 1) * z + x;

			vertices.emplace_back(vertex);
		}
	}

	//Indices
	for (UINT z = 0; z < height; z++)
	{
		for (UINT x = 0; x < width; x++)
		{
			indices.emplace_back((width + 1) * z + x);//0
			indices.emplace_back((width + 1) * (z + 1) + x);//1
			indices.emplace_back((width + 1) * (z + 1) + x + 1);//2

			indices.emplace_back((width + 1) * z + x);//0
			indices.emplace_back((width + 1) * (z + 1) + x + 1);//2
			indices.emplace_back((width + 1) * z + x + 1);//3
		}
	}

	size = indices.size() / 3;

	input = new InputDesc[size];
	for (UINT i = 0; i < size; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		input[i].v0 = vertices[index0].position;
		input[i].v1 = vertices[index1].position;
		input[i].v2 = vertices[index2].position;

		input[i].index = i;
	}

	CreateNormal();
	CreateTangent();

	mesh = new Mesh(vertices.data(), sizeof(VertexType), (UINT)vertices.size(),
		indices.data(), (UINT)indices.size());
}

void TerrainEditor::CreateNormal()
{
	for (UINT i = 0; i < indices.size() / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		Vector3 v0 = vertices[index0].position;
		Vector3 v1 = vertices[index1].position;
		Vector3 v2 = vertices[index2].position;

		Vector3 A = v1 - v0;
		Vector3 B = v2 - v0;

		Vector3 normal = Vector3::Cross(A, B).Normal();

		vertices[index0].normal = normal + vertices[index0].normal;
		vertices[index1].normal = normal + vertices[index1].normal;
		vertices[index2].normal = normal + vertices[index2].normal;
	}

	for (VertexType& vertex : vertices)
		vertex.normal = Vector3(vertex.normal).Normal();
}

void TerrainEditor::CreateTangent()
{
	for (UINT i = 0; i < indices.size() / 3; i++)
	{
		UINT index0 = indices[i * 3 + 0];
		UINT index1 = indices[i * 3 + 1];
		UINT index2 = indices[i * 3 + 2];

		VertexType vertex0 = vertices[index0];
		VertexType vertex1 = vertices[index1];
		VertexType vertex2 = vertices[index2];

		Vector3 p0 = vertex0.position;
		Vector3 p1 = vertex1.position;
		Vector3 p2 = vertex2.position;

		Float2 uv0 = vertex0.uv;
		Float2 uv1 = vertex1.uv;
		Float2 uv2 = vertex2.uv;

		Vector3 e0 = p1 - p0;
		Vector3 e1 = p2 - p0;

		float u0 = uv1.x - uv0.x;
		float u1 = uv2.x - uv0.x;
		float v0 = uv1.y - uv0.y;
		float v1 = uv2.y - uv0.y;

		Vector3 tangent = (v1 * e0 - v0 * e1);

		vertices[index0].tangent = tangent + vertices[index0].tangent;
		vertices[index1].tangent = tangent + vertices[index1].tangent;
		vertices[index2].tangent = tangent + vertices[index2].tangent;
	}

	for (VertexType& vertex : vertices)
	{
		Vector3 t = vertex.tangent;
		Vector3 n = vertex.normal;

		Vector3 temp = (t - n * Vector3::Dot(n, t)).Normal();

		vertex.tangent = temp;
	}
}

void TerrainEditor::CreateCompute()
{
	computeShader = Shader::AddCS(L"ComputePicking");

	structuredBuffer
		= new StructuredBuffer(input, sizeof(InputDesc), size,
			sizeof(OutputDesc), size);

	rayBuffer = new RayBuffer;
	output = new OutputDesc[size];