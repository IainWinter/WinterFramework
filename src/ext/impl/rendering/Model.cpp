#include "ext/rendering/Model.h"

void Material::Set(const std::string& name, const int& x)        { _Set(name, x);       };
void Material::Set(const std::string& name, const u32& x)        { _Set(name, x);       };
void Material::Set(const std::string& name, const f32& x)        { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::fvec2& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::fvec3& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::fvec4& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::ivec2& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::ivec3& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::ivec4& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::fmat2& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::fmat3& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const glm::fmat4& x) { _Set(name, x);       };
void Material::Set(const std::string& name, const Color& color)  { _Set(name, color);   };
void Material::Set(const std::string& name, r<Texture> texture)  { _Set(name, texture); }

void Material::SetArray(const std::string& name, const int* x, int count) { _SetArray(name, x, count); }
void Material::SetArray(const std::string& name, const u32* x, int count) { _SetArray(name, x, count); }
void Material::SetArray(const std::string& name, const f32* x, int count) { _SetArray(name, x, count); }

void Material::SetShader(r<ShaderProgram> program)
{
	m_program = program;
}

void Material::Apply(r<IHasMaterialProperties> other)
{
	for (auto [name, param] : m_params)
	{
		param->Apply(name, other);
	}
}

void Material::Apply()
{
	Apply(m_program);
}

void Model::AddMesh(const Transform& transform, r<Mesh> mesh, r<Material> material)
{
	MeshReference ref;
	ref.mesh = mesh;
	ref.material = material;

	meshes.push_back(ref);
}

void Model::RemoveMesh(const r<Mesh>& mesh)
{
	auto test = [mesh](const MeshReference& ref)
	{
		return ref.mesh == mesh;
	};

	auto itr = std::find_if(meshes.begin(), meshes.end(), test);
	if (itr != meshes.end())
	{
		meshes.erase(itr);
	}
}

const std::vector<MeshReference>& Model::GetMeshes() const
{
	return meshes;
}

std::vector<MeshReference>& Model::GetMeshes()
{
	return meshes;
}

void Model::Draw()
{
	// could sort by shader, but really would just
	// add a command buffer and sort that
	// so this can be rough for now

	for (MeshReference& ref : meshes)
	{
		ref.material->Apply();
		ref.mesh->Draw();
	}
}
