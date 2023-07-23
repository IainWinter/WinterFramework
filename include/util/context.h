#pragma once

// This is required to pass global C++ state between DLL boundaries

template<typename _t>
class wContextStore;

class wContext
{
private:
	int next_location = 0;

	template<typename _t>
	friend class wContextStore;

	int  _get_next_location() const { return next_location; }
	void _inc_next_location() { next_location += 1; }

public:
	// Realloc, should use location
	virtual void Realloc(int location) {}
};

// would be nice if there was a way to not have to store the location in the context
// itself. But that requires another struct to pass an int pointer

// _t must be a wContext
template<typename _t>
class wContextStore
{
	_t* current = nullptr;

	// this is an id that counts up every time we set the current context
    // gets used to identify the current module
	int location = 0;

public:
	void Create()
	{
		Destroy();
		SetCurrent(new _t());
	}

	void Destroy()
	{
		delete current;
		current = nullptr;
	}

	void SetCurrent(_t* context)
	{
		current = context;

		location = context->_get_next_location();
		context->_inc_next_location();
	}

	void Realloc()
	{
		current->Realloc(location);
	}

	_t* GetCurrent() const
	{
		return current;
	}

	int GetLocation() const
	{
		return location;
	}

	// this allows for not changing any code between this and a regular pointer
	//
	_t* operator->() const
	{
		return GetCurrent();
	}
};

#define wContextDecl(type)  void  CreateContext();                   \
							void  DestroyContext();                  \
							void  SetCurrentContext(type* context);  \
					  const type* GetContext();                      \
							int   GetContextLocation();              \
							void  ContextRealloc();

#define wContextImpl(type) 	wContextStore<type> ctx;                                           \
							void  CreateContext() { ctx.Create(); }                             \
							void  DestroyContext() { ctx.Destroy(); }                           \
							void  SetCurrentContext(type* context) { ctx.SetCurrent(context); } \
					  const type* GetContext() { return ctx.GetCurrent(); }                    \
							int   GetContextLocation() { return ctx.GetLocation(); }             \
							void  ContextRealloc() { ctx.Realloc(); }
