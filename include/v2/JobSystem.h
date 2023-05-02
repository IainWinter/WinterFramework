#pragma once

#include "util/tsque.h"
#include "tbb/concurrent_unordered_set.h"

#include <functional>
#include <vector>
#include <unordered_set>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <type_traits>
#include <string>
#include <iostream>

// JobNode
//	A single piece of work

// JobTree
//	Holds a dependency tree of JobNodes

// Job
//	A wrapper around JobNode/Tree for manipulating the JobTree

// JobExecutor
//	Executes a JobTree

class Job;
class JobTree;

constexpr int JOB_MAX_CONTINUATIONS = 16;

struct JobNode
{
	// Store a pointer to the owning tree so a Job can be passed to 'work' when called
	JobTree* tree;

	// When this node is finished, push these to work queue.
	JobNode* continuations[JOB_MAX_CONTINUATIONS]; // could use std::array
	std::atomic<int> continuationCount = 0;

	// Once this is 0, the job is ready to run
	std::atomic<int> dependencies = 0;

	// The work for the job. 
	// todo: use a template to remove need to pass 'Job' to each one
	std::function<void(Job)> work;
	
    std::string name = "";
	int id = 0;

	void AddContinuation(JobNode* node);
	void MoveContinuationsInto(JobNode* into);
};

class Job
{
public:
	Job(JobNode* node, JobTree* tree);

	Job& SetName(const std::string& name);

	// Add a job between this node and its continuations
	// Return the new job
	// 
	// Example:
	//       A -> B -> D
	//         -> C -^
	// 
	//       A.Then(E)
	// 
	//       A -> E -> B -> D
	//              -> C -^
	//
	Job& ThenJob(Job& job);

	// See ThenJob.
	// Create a new job and return it
	template<typename _f>
	Job Then(_f&& func);

	// Add multiple jobs between this node and its continuations
	// Return an empty job that joins the fork
	//
	// Example: 
	//       A -> B
	//
	//       A.Fork({C, D})
	// 
	//       A -> C -> [join] -> B
	//         -> D -^
	//
	template<typename _iterable>
	Job Fork(const _iterable& funcs);

	// See Fork.
	// Add a number of jobs which apply a lambda over each item of a collection
	// Return the join
	template<typename _iterable, typename _f>
	Job For(int batchSize, _iterable& iterable, _f&& perItem);

	// See For. 
	// Split the work over the max number of continuations, or size of iterable
	// Return the join
	template<typename _iterable, typename _f>
	Job For(_iterable& iterable, _f&& perItem);

private:
	JobNode* node;
	JobTree* tree;
};

class JobTree
{
public:
	JobTree(/* allocator */);

	~JobTree();

	// Return a list of all JobNodes with zero dependencies
	std::vector<JobNode*> GetRoots();

	// Create a Job with no work
	Job CreateEmpty();

	// Create a Job with a callable with signature 'void(Job)'
	template<typename _f>
	Job Create(_f&& work);

	// call this after a tree has been run to free its nodes
	void Cleanup();

	void PrintGraphviz(std::ostream& o);

private:
	JobNode* _alloc_node();
	void _free_node(JobNode* node);

private:
	// todo: allocator

	tbb::concurrent_unordered_set<JobNode*> nodes;
	std::atomic<int> jobIdNext;
};

class JobExecutor
{
public:
	JobExecutor(int numberOfThreads);
	~JobExecutor();

public:
	void Run(JobTree& tree);
	void WaitForAll();

private:
	struct JobThreadContext
	{
		int index;
	};

	struct JobThread
	{
		std::thread thread;
		JobThreadContext* ctx;
	};

	void ThreadWork(JobThreadContext* ctx);
	void IncWaitCount(int c);

	void CreateThreads(int numberOfThreads);
	void DestroyThreads();

private:
	tsque<JobNode*> wavefront;
	std::vector<JobThread> threads;

	std::condition_variable waitVar;
	std::mutex waitMutex;
	int workCount = 0;
};

//
//	Template impl
//

template<typename _f>
Job Job::Then(_f&& func)
{
	Job job = tree->Create(func);
	ThenJob(job);

	return job;
}

template<typename _iterable>
Job Job::Fork(const _iterable& funcs)
{
	// Create a fake node to join these
	Job join = tree->CreateEmpty();
	node->MoveContinuationsInto(join.node);

	for (auto&& func : funcs)
	{
		Job job = tree->Create(func);
		job.node->AddContinuation(join.node);

		node->AddContinuation(job.node);
	}

	return join;
}

template<typename _iterable, typename _f>
Job Job::For(int batchSize, _iterable& iterable, _f&& perItem)
{
	int size = iterable.size();
	auto begin = iterable.begin();

	if (size == 0 || batchSize <= 0) // edge case, return this job
		return *this;

	// Create a fake node to join the fork & transfer this jobs continuations
	Job join = tree->CreateEmpty();
	node->MoveContinuationsInto(join.node);

	for (int i = 0; i < size; i += batchSize)
	{
		int thisBlockSize = std::min(size - i, batchSize);

		auto batch = [=](Job _) 
		{
			auto itr = begin + i;
			auto end = itr + thisBlockSize;

			for (; itr != end; ++itr)
				perItem(*itr);
		};

		Job job = tree->Create(batch);
		job.node->AddContinuation(join.node);
			
		node->AddContinuation(job.node);
	}

	return join;
}

template<typename _iterable, typename _f>
Job Job::For(_iterable& iterable, _f&& perItem)
{
	int batchSize = (iterable.size() + JOB_MAX_CONTINUATIONS) / JOB_MAX_CONTINUATIONS;

	return For(batchSize, iterable, perItem);
}

template<typename _f>
Job JobTree::Create(_f&& work)
{
	JobNode* node = _alloc_node();
	node->work = work;
	return Job(node, this);
}
