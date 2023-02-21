#include "ext/rendering/ImportMesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Log.h"

std::vector<Mesh> LoadMeshFromFile(const std::string& filename)
{
	Assimp::Importer importer;
	
	const aiScene* scene = importer.ReadFile(filename,
		  aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		//| aiProcess_CalcTangentSpace
		//| aiProcess_SortByPType
		//| aiProcess_GenNormals
		//| aiProcess_FlipUVs
	);

	if (!scene)
	{
		log_io("w~Failed to load %s", filename.c_str());
		return {};
	}

	std::vector<Mesh> meshes;

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		Mesh& mesh = meshes.emplace_back();

		const aiMesh* aimesh = scene->mMeshes[i];
		const char* ainame = aimesh->mName.C_Str();

		if (aimesh->HasPositions())
		{
			mesh.Add<vec3>(Mesh::aPosition)
				.Get(Mesh::aPosition)
					->Set(aimesh->mNumVertices, aimesh->mVertices);
		}

		//MeshDescription description;

		//if (aimesh->HasPositions()) {
		//	description.DescribeBuffer(bName::POSITION, MakeLayout<float>(3));
		//}

		//if (aimesh->HasNormals()) {
		//	description.DescribeBuffer(bName::NORMAL, MakeLayout<float>(3));
		//}

		//if (aimesh->HasTangentsAndBitangents()) {
		//	description.DescribeBuffer(bName::TANGENT, MakeLayout<float>(3));
		//	description.DescribeBuffer(bName::BITANGENT, MakeLayout<float>(3));
		//}

		//// max of 8
		//for (unsigned i = 0; i < aimesh->GetNumUVChannels(); i++) {
		//	bName channel = (bName)((unsigned)bName::UV + i);
		//	description.DescribeBuffer(channel, MakeLayout<float>(aimesh->mNumUVComponents[i]));
		//}

		//// max of 8
		//for (unsigned i = 0; i < aimesh->GetNumColorChannels(); i++) { // todo: move back above uvs
		//	bName channel = (bName)((unsigned)bName::COLOR + i);
		//	description.DescribeBuffer(channel, MakeLayout<float>(4));
		//}

		//MeshData* data = new MeshData(description);

		//if (description.HasBuffer(bName::POSITION)) {
		//	data->SetBufferData(bName::POSITION, aimesh->mNumVertices, aimesh->mVertices);
		//}

		//if (description.HasBuffer(bName::NORMAL)) {
		//	data->SetBufferData(bName::NORMAL, aimesh->mNumVertices, aimesh->mNormals);
		//}

		//if (   description.HasBuffer(bName::TANGENT)
		//	&& description.HasBuffer(bName::BITANGENT))
		//{
		//	data->SetBufferData(bName::TANGENT, aimesh->mNumVertices, aimesh->mTangents);
		//	data->SetBufferData(bName::BITANGENT, aimesh->mNumVertices, aimesh->mBitangents);
		//}

		//for (unsigned c = 0; c < aimesh->GetNumColorChannels(); c++) {
		//	bName channel = (bName)((int)bName::COLOR + c);
		//	if (description.HasBuffer(channel)) {
		//		data->SetBufferData(channel, aimesh->mNumVertices, aimesh->mColors[c]);
		//	}
		//}

		//for (unsigned c = 0; c < aimesh->GetNumUVChannels(); c++) {
		//	bName channel = (bName)((unsigned)bName::UV + c);
		//	if (description.HasBuffer(channel)) {
		//		unsigned uvComponents = aimesh->mNumUVComponents[c];
		//		unsigned count        = aimesh->mNumVertices * uvComponents;

		//		float* buffer = new float[count];

		//		for (unsigned i = 0, j = 0; j < count; i++) {
		//			switch (uvComponents) {
		//				case 1: {
		//					buffer[j++] = aimesh->mTextureCoords[c][i].x;
		//					break;
		//				}
		//				case 2: {
		//					buffer[j++] = aimesh->mTextureCoords[c][i].x;
		//					buffer[j++] = aimesh->mTextureCoords[c][i].y;
		//					break;
		//				}
		//				case 3: {
		//					buffer[j++] = aimesh->mTextureCoords[c][i].x;
		//					buffer[j++] = aimesh->mTextureCoords[c][i].y;
		//					buffer[j++] = aimesh->mTextureCoords[c][i].z;
		//					break;
		//				}
		//			}
		//		}

		//		data->SetBufferData(channel, aimesh->mNumVertices, buffer);
		//			
		//		delete[] buffer;
		//	}
		//}

		r<Buffer> indexBuffer = mesh.Add<int>(Mesh::aIndexBuffer).Get(Mesh::aIndexBuffer);

		for (unsigned f = 0, i = 0; f < aimesh->mNumFaces; f++)
		{
			const aiFace& face = aimesh->mFaces[f];
			indexBuffer->Push(3, face.mIndices); // 3 indices per face is enforced by aiProcess_Triangulate
		}
	}

	importer.FreeScene();

	return meshes;
}
