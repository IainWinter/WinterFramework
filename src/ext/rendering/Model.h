#pragma once

#include "Rendering.h"
#include "util/Transform3D.h"

class Material : public IHasMaterialProperties
{
public:
	void Set(const std::string& name, const int& x) override;
	void Set(const std::string& name, const u32& x) override;
	void Set(const std::string& name, const f32& x) override;
	void Set(const std::string& name, const glm::fvec2& x) override;
	void Set(const std::string& name, const glm::fvec3& x) override;
	void Set(const std::string& name, const glm::fvec4& x) override;
	void Set(const std::string& name, const glm::ivec2& x) override;
	void Set(const std::string& name, const glm::ivec3& x) override;
	void Set(const std::string& name, const glm::ivec4& x) override;
	void Set(const std::string& name, const glm::fmat2& x) override;
	void Set(const std::string& name, const glm::fmat3& x) override;
	void Set(const std::string& name, const glm::fmat4& x) override;
	void Set(const std::string& name, const Color& color) override;
	void Set(const std::string& name, r<Texture> texture) override;
	void SetArray(const std::string& name, const int* x, int count) override;
	void SetArray(const std::string& name, const u32* x, int count) override;
	void SetArray(const std::string& name, const f32* x, int count) override;

	void SetShader(r<ShaderProgram> program);

	// Set all properties from this material on another, or a specific shader
	void Apply(r<IHasMaterialProperties> other);

	// Set all properties from this material onto it's shader
	void Apply();

private:
	template<typename _t>
	void _Set(const std::string& name, const _t& value)
	{
		m_params[name] = mkr<Param<_t>>(value);
	}

	template<typename _t>
	void _SetArray(const std::string& name, const _t* value, int count)
	{
		m_params[name] = mkr<ParamArray<_t>>(value, count);
	}

private:
	enum ParamType
	{
		_int, _u32, _f32,

		_fvec1, _fvec2, _fvec3, _fvec4,
		_ivec1, _ivec2, _ivec3, _ivec4,
		
		_fmat2, _fmat3, _fmat4,

		_color,

		_texture_r, _texture
	};

	struct IParam
	{
		virtual void Apply(const std::string& name, r<IHasMaterialProperties> other) const = 0;
	};

	template<typename _t>
	struct Param : IParam
	{
		Param(const _t& value)
		{
			this->value = value;
		}

		void Apply(const std::string& name, r<IHasMaterialProperties> other) const override
		{
			other->Set(name, value);
		}

	private:
		_t value = {};
	};

	template<typename _t>
	struct ParamArray : IParam
	{
		ParamArray(const _t* values, int count)
		{
			this->values = std::vector<_t>(values, values + count);
		}

		void Apply(const std::string& name, r<IHasMaterialProperties> other) const override
		{
			other->SetArray(name, values.data(), (int)values.size());
		}

	private:
		std::vector<_t> values;
	};

	std::unordered_map<std::string, r<IParam>> m_params;
	r<ShaderProgram> m_program;
};


// keep materials, but should prob just use ecs to store the models cus its already recursive with transforms
// need to define a system of how they would be linked and maybe do a transform cache type thing (functional style state update would do this for free)

struct MeshReference
{
	r<Mesh> mesh;
	r<Material> material;

	Transform localTransform;
};

// A collection of meshes with material properties
class Model
{
public:
	// Add a reference to a mesh
	//
	// @transform
	//		A local transform inside the model
	// 
	// @mesh
	//		The mesh to reference
	// 
	// @material
	//		The material to reference. If nullptr, then no material is set,
	//		and when the model is drawn whatever material is set will be used
	//
	void AddMesh(const Transform& transform, r<Mesh> mesh, r<Material> material);

	//	Remove a reference to a mesh and its material if no other meshes reference it
	//
	void RemoveMesh(const r<Mesh>& mesh);

	const std::vector<MeshReference>& GetMeshes() const;
	std::vector<MeshReference>& GetMeshes();

	//	Draw the model immediately
	//
	void Draw();

private:
	std::vector<MeshReference> meshes;
};
