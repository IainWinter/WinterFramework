#pragma once

#include <vector>

enum MeshAttribute
{
	MeshAttributePosition  = 1 << 1,
	MeshAttributeTextureUV = 1 << 2,
	MeshAttributeNormals   = 1 << 3,
	MeshAttributeTangents  = 1 << 4,
	MeshAttributeColors    = 1 << 5,
};

enum MeshTopology
{
	MeshTopologyTriangles,
	MeshTopologyLines,
	MeshTopologyLoops,

	MeshTopologyTriangleStrip,
};

enum MeshDataType
{
	MeshDataTypeFloat1,
	MeshDataTypeFloat2,
	MeshDataTypeFloat3,
	MeshDataTypeFloat4,
	MeshDataTypeInt1,
	MeshDataTypeInt2,
	MeshDataTypeInt3,
	MeshDataTypeInt4,
};

//	Defines the layout of data in a mesh's buffer
//
struct MeshLayout
{
	int channels;
	bool hasIndex;

	int elementCount;
	int indexCount;

	int bytesPerElement;
	int bytesPerIndex;

	int NumberOfBytes() const;
};