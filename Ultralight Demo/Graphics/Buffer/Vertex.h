#pragma once
#include <PCH.h>

struct Vertex_2D
{
public:
	Vertex_2D() 
		:Position(0,0,0), TexCoord(0,0)
	{}
	Vertex_2D(float x, float y, float z,
			  float u, float v)
	{
		Position = DirectX::XMFLOAT3(x, y, z);
		TexCoord = DirectX::XMFLOAT2(u, v);
	}
	Vertex_2D(DirectX::XMFLOAT3 position,
			  DirectX::XMFLOAT2 texCoord)
		:Position(position),
		TexCoord(texCoord)
	{
	}
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TexCoord;
};