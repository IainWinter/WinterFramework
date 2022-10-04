Main goal of this project is to simplify what I have been doing in IwEngine. Before I had a folder with 10+ files for each of the major systems. I didn't use many libraries in IwEngine because my goal was to learn how to create them, and now I see how much work it takes to harden and maintain libraries like EnTT and Box2d. Therefore, in this project I make full use of as many libraries as possible and mainly focus on tying them together and making simple & clean APIs.

Focusing on making it as small and understandable as possible. I have a feeling that each of the libraries from before could just be a single file.
Most of the functionality gets included no matter what, so a single file makes it simpler to use. Also, a single file lets you know the level of abstraction as you get deeper into the file as things need to be defined before they can be used. Looking at a folder of many files does not give you this info at a glance.

| Piece of framework | Description |
| --- | --- |
| Audio | An interface to talk to underlying audio engines through handles. Currently implements FMOD |
| Entity | A wrapper around EnTT exposing the API through an entity class |
| Event | An events system based on composing std::functions to member functions. |
| Physics | A wrapper around box2d that ties into the entity system to automatically maintain the physics world |
| Rendering | Taking ideas from CUDA about host and device memory, this follows the same naming scheme through a unified interface for each OpenGL object. Currently supports Texture/Target, Buffer/Mesh and ShaderProgram |
| Windowing | Uses SDL to create a window and pump messages. Translates them into framework events and sends them to the main event bus |
| App | Ties everything together to create an application framework. Can load Worlds and attach Systems to update the state. Each World stores its own entities, and multiple can be loaded at once to compose scenes. |