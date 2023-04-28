#include "ext/rendering/ImportMesh.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Log.h"

Model LoadModelFromFile(const std::string& filename)
{
	Assimp::Importer importer;
	
	const aiScene* scene = importer.ReadFile(filename,
		  aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		| aiProcess_PreTransformVertices      // easy hack for applying transforms, should remove if I ever make a proper scene graph
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

	Model model;

	for (size_t i = 0; i < scene->mNumMeshes; i++)
	{
		r<Mesh> mesh = mkr<Mesh>();
		model.AddMesh({}, mesh, {});

		const aiMesh* aimesh = scene->mMeshes[i];
		const char* ainame = aimesh->mName.C_Str();

		if (aimesh->HasPositions())
		{
			mesh->Add<vec3>(Mesh::aPosition)
				.Get(Mesh::aPosition)->Set(aimesh->mNumVertices, aimesh->mVertices);
		}

        if (aimesh->HasNormals())
        {
            mesh->Add<vec3>(Mesh::aNormal)
                .Get(Mesh::aNormal)->Set(aimesh->mNumVertices, aimesh->mNormals);
        }
        
		r<Buffer> indexBuffer = mesh->Add<int>(Mesh::aIndexBuffer).Get(Mesh::aIndexBuffer);

		for (unsigned f = 0, i = 0; f < aimesh->mNumFaces; f++)
		{
			const aiFace& face = aimesh->mFaces[f];
			indexBuffer->Push(3, face.mIndices); // 3 indices per face is enforced by aiProcess_Triangulate
		}
	}

	importer.FreeScene();

	return model;
}
