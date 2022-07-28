//#pragma once
//
//#include "app/System.h"
//#include "Rendering.h"
//
//struct System_RenderStage : SystemBase
//{
//private:
//	r<Target> target;
//	r<ShaderProgram> program;
//
//public:
//	System_RenderStage(r<Target> target, r<ShaderProgram> program)
//		: target  (target)
//		, program (program)
//	{}
//
//	void Update() override
//	{
//		UseTargetAndShader();
//		Draw();
//	}
//
//	virtual void Draw() = 0;
//
//private :
//	void UseTargetAndShader()
//	{
//		if (program) program->Use();
//		
//		if (target) target->Use();
//		else        Target::UseDefault();
//	}
//};
//
//struct Stage_ClearTarget : System_RenderStage
//{
//	Color color;
//
//	Stage_ClearTarget(r<Target> target, Color color)
//		: System_RenderStage (target, nullptr)
//		, color              (color)
//	{}
//
//	void Draw() override
//	{
//		Target::Clear(color);
//	}
//};