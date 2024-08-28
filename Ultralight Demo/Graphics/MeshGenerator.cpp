#include "PCH.h"
#include "MeshGenerator.h"
#include "Renderer.h"
#include "Texture.h"

shared_ptr<Mesh> MeshGenerator::GenerateCube(MeshGenerationOption options)
{
    //TODO: Maybe add error checking but this shouldn't be called if engine hasn't been initialized
	//This is expecting D3DClass to have been initialized

	shared_ptr<Primitive> prim = make_shared<Primitive>();
	prim->m_Name = "CubePrimitive";
	prim->m_Topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	vector<uint32_t> indices =
	{
		0, 1, 2,    //Front  ◤
		2, 1, 3,    //Front  ◢
		4, 5, 6,    //Left   ◤
		6, 5, 7,    //Left   ◢
		8, 9, 10,   //Right  ◤
		10, 9, 11,  //Right  ◢
		12, 13, 14, //Back   ◤
		14, 13, 15, //Back   ◢
		16, 17, 18, //Top    ◤
		18, 17, 19, //Top    ◢
		20, 21, 22, //Bottom ◤
		22, 21, 23, //Bottom ◢
	};
	if (!prim->m_IndexBuffer.Initialize(indices))
	{
		return nullptr;
	}

	auto& attributes = prim->m_PrimaryPrimitiveAttributeSet;
	float length = 1;
	vector<Vector3> positions =
	{
		//Front Face
		Vector3(-length, length, length), //Front-Top-Left
		Vector3(length, length, length), //Front-Top-Right
		Vector3(-length, -length, length), //Front-Bottom-Left
		Vector3(length, -length, length), //Front-Bottom-Right
		//Left Face
		Vector3(-length, length, -length), //Left-Top-Back
		Vector3(-length, length, length), //Left-Top-Front
		Vector3(-length, -length, -length), //Left-Bottom-Back
		Vector3(-length, -length, length), //Left-Bottom-Front
		//Right Face
		Vector3(length, length, length), //Right-Top-Front
		Vector3(length, length, -length), //Right-Top-Back
		Vector3(length, -length, length), //Right-Bottom-Front
		Vector3(length, -length, -length), //Right-Bottom-Back
		//Back Face
		Vector3(length, length, -length), //Back-Top-Right
		Vector3(-length, length, -length), //Back-Top-Left
		Vector3(length, -length, -length), //Back-Bottom-Right
		Vector3(-length, -length, -length), //Back-Bottom-Left
		//Top Face
		Vector3(-length, length, -length), //Back-Top-Left
		Vector3(length, length, -length), //Back-Top-Right
		Vector3(-length, length, length), //Front-Top-Left
		Vector3(length, length, length), //Front-Top-Right
		//Bottom Face
		Vector3(-length, -length, length), //Front-Bottom-Left
		Vector3(length, -length, length), //Front-Bottom-Right
		Vector3(-length, -length, -length), //Back-Bottom-Left
		Vector3(length, -length, -length), //Back-Bottom-Right
	};
	
	if (!attributes->m_Positions.Initialize(positions))
	{
		return nullptr;
	}

	{//Normals
		vector<Vector3> normals_per_face;
		normals_per_face.push_back({ 0, 0, 1 }); //Front verts
		normals_per_face.push_back({ -1, 0, 0}); //Left verts
		normals_per_face.push_back({ 1, 0, 0 }); //Right verts
		normals_per_face.push_back({ 0, 0, -1 }); //Back verts
		normals_per_face.push_back({ 0, 1, 0 }); //Top verts
		normals_per_face.push_back({ 0, -1, 0 }); //Bottom verts

		vector<Vector3> normals;
		for (int i = 0; i < normals_per_face.size(); i++)
		{
			for (int v = 0; v < 4; v++) //4 verts per face
			{
				normals.push_back(normals_per_face[i]);
			}
		}

		if (!attributes->m_Normals.Initialize(normals))
		{
			return nullptr;
		}
	}

	if (options == MeshGenerationOption::COLOREDVERTICES)
	{
		vector<Vector4> colors;
		for (int f = 0; f < 6; f++) //6 faces
		{
			//4 verts per face
			colors.push_back({ 1.0, 0, 0, 1 });   //red
			colors.push_back({ 1.0, 1.0, 0, 1 }); //yellow
			colors.push_back({ 0, 1.0, 0, 1 });   //green
			colors.push_back({ 0, 0, 1.0, 1 });   //blue
		}
		if (!attributes->m_Colors.Initialize(colors))
		{
			return nullptr;
		}
	}

	shared_ptr<EZGLTF::Material> material = nullptr;

	if (options == MeshGenerationOption::TEXTUREDVERTICES)
	{
		vector<Vector2> texcoords;
		for (int f = 0; f < 6; f++) //6 faces
		{
			//topleft
			texcoords.push_back({ 0, 0 });
			//topright
			texcoords.push_back({ 1, 0 });
			//bottomleft
			texcoords.push_back({ 0, 1 });
			//bottomright
			texcoords.push_back({ 1, 1 });
		}
		if (!attributes->m_TexCoords.Initialize(texcoords))
		{
			return nullptr;
		}
		material = make_shared<EZGLTF::Material>();
		material->BaseColorTexture = make_shared<Texture>();
		if (material->BaseColorTexture->Initialize(DirectoryHelper::GetAssetsDirectoryA() + "AIBowser.png") == false)
		{
			return nullptr;
		}
	}

	shared_ptr<PrimitiveRef> primRef = make_shared<PrimitiveRef>(prim, material);

	shared_ptr<Mesh> cubeMesh = make_shared<Mesh>();
	cubeMesh->m_Name = "CubeMesh";
	cubeMesh->m_PrimitiveRefs.push_back(primRef);

	return cubeMesh;
}
