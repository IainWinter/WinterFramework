#pragma once

#include <vector>

// This should create a scene graph which renders itself instead of being a
// thing that gets submitted to by the program

// that makes much more sense if the entities only store handles into this, just like the
// physics system

// and onky make this for exactly Regolith btw

class v2Resource {
	char* data;

};

class v2Texture {
};



class v2RenderNode {
public:
	virtual void Draw() = 0;
};

class v2RootRenderNode : public v2RenderNode {
public:
	void Draw() override {

	}

private:
	std::vector<v2RenderNode*> nodes;
};

class v2RenderGraph {
	v2RenderNode* root;
};