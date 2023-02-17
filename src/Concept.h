#pragma once

// A more functional style scheduler for state update

// State
//	 Information
//   Work Queue
//   Can add work to queue
//   Can get next state, which returns a copy with the work applied

// Work
//   What is reads (data handles)
//   What is writes (data handles)
//   A function pointer

// Data handle
//   An integer representing a point of data

#include <vector>
#include <unordered_set>
#include <functional>

#include "entt/entt.hpp"

class State
{
	template<typename... _t>
	entt::view<entt::type_list<const _t...>> Get() const
	{
		return data.view<const _t...>();
	}

	template<typename _t>
	void GetSetView(entt::entity entity, const _t& value)
	{


		data.emplace_or_replace(entity, value);
	}

private:
	entt::registry data;
};

class IWork
{
public:
	virtual void CallFunction(const State* read, State* write) const = 0;
};

template<typename... _t>
class Work : public IWork
{
public:
	void CallFunction(const State* read, State* write) const override
	{
		auto get = read->Get<_t...>().each();
		auto set = next.GetSetView<_t...>().each();

		auto get_itr = get.begin();
		auto set_itr = get.begin();

		for (std::tuple<entt::entity, const _t&...> e : get.each())
		{
			std::apply(func, e);
		}
	}
};

// Holds multiple frame states
// this is all entity data
class EntityDataStore
{
	State* readOnly; // read
	State* next;    // write

	std::vector<IWork*> workQueue;



	// swaps frames

	void CommitWork()
	{
		for (IWork* work : workQueue)
		{
			work->CallFunction(readOnly, next);
		}

		std::swap(readOnly, next);
	}
};






















//namespace tuple_helpers
//{
//	// these cant be generic because with refs we need to use tie, but anything else would want
//	// to use make_tuple, so keep these as private instead of global helper functions...
//	// https://stackoverflow.com/a/39101723/6772365
//
//	// splitting EntityQuery made these now have to be free, so I'll just put them in a namespace :)
//
//	template <typename T, typename Tuple>
//	auto tpush_front(const T& t, const Tuple& tuple)
//	{
//		return std::tuple_cat(std::make_tuple(t), tuple);
//	}
//
//	template <typename Tuple, std::size_t ... Is>
//	auto tpop_front_impl(const Tuple& tuple, std::index_sequence<Is...>)
//	{
//		return std::tie(std::get<1 + Is>(tuple)...);
//	}
//
//	template <typename Tuple>
//	auto tpop_front(const Tuple& tuple)
//	{
//		return tpop_front_impl(tuple, std::make_index_sequence<std::tuple_size<Tuple>::value - 1>());
//	}
//}
//
//
//
//class State;
//
//class IWork
//{
//public:
//	virtual void CallFunction(const State& state, State& next) = 0;
//};
//
//class State
//{
//public:
//	//void AddWork(Work work)
//	//{
//	//	workQueue.push_back(work); 
//	//}
//
//	State GetNextState()
//	{
//		// Create graph of work based on what it reads / writes
//
//		State next; // get copy of only information
//
//		// create all entities in next
//
//		// copy all entity ids
//		data.each([&next](entt::entity e) { next.data.create(e); });
//
//		for (IWork* work : workQueue)
//		{
//			work->CallFunction(*this, next);
//		}
//
//		return next;
//	}
//
//	template<typename... _t>
//	entt::view<entt::type_list<const _t...>> Get() const
//	{
//		return data.view<const _t...>();
//	}
//
//	template<typename _t>
//	void GetSetView(entt::entity entity, const _t& value)
//	{
//		data.emplace_or_replace(entity, value);
//	}
//
//	template<typename _t>
//	const _t& Get(int dataHandle) const
//	{
//		static _t t;
//		return t;
//	}
//
//	entt::registry data;
//private:
//	std::vector<IWork*> workQueue;
//};
//
//template<typename... _components>
//class tWork : IWork
//{
//public:
//	tWork(const std::function<void(const _components&...)>& func)
//		: func (func)
//	{}
//
//	void SetWrites(const std::unordered_set<int>& writes) { this->writes = writes; }
//	void SetReads(const std::unordered_set<int>& reads) { this->reads = reads; }
//
//	// function
//
//	void SetFunction(const std::function<void(const State&)>& func) { this->func = func; }
//	void CallFunction(const State& state) { func(state); }
//
//public:
//	void CallFunction(const State& state, State& next) override
//	{
//		auto get = state.Get<_components...>().each();
//		auto set = next .GetSetView<_components...>().each();
//
//		auto get_itr = get.begin();
//		auto set_itr = get.begin();
//
//		for (std::tuple<entt::entity, const _components&...> e : get.each())
//		{
//			std::apply(func, tuple_helpers::tpop_front(e));
//		}
//	}
//
//private:
//	std::unordered_set<int> writes;
//	std::unordered_set<int> reads;
//	std::function<void(const _components&...)> func;
//};
//
//struct Position
//{
//	float x, y, z;
//};
//
//void RunTest()
//{
//	int xPositionDataHandle = 0;
//
//	State state;
//
//	tWork<Position> updatePosition = tWork<Position>([](const Position& position)
//	{
//		return position;
//	});
//
//	updatePosition.CallFunction(state, state);
//
//	//tWork<Position> updatePosition;
//	//updatePosition.SetFunctiob
//
//	//Work updatePosition;
//	//updatePosition.SetReads({ xPositionDataHandle });
//	//updatePosition.SetWrites({ xPositionDataHandle });
//	//updatePosition.SetFunction([xPositionDataHandle](const State& state)
//	//{
//	//	float x = state.Get<float>(xPositionDataHandle);
//	//	return x + 3;
//	//});
//
//	//state.AddWork(updatePosition);
//
//	state = state.GetNextState();
//}