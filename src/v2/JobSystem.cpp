#include "v2/JobSystem.h"
#include "util/SimpleTrace.h"

int inc_if_not_greator_than(std::atomic<int>& x, int max)
{
	int idx = x.load();
	while (idx < 16 && !x.compare_exchange_weak(idx, idx + 1)) {}
	return idx;
}

void JobNode::AddContinuation(JobNode* node)
{
    if (this == node)
        return;

	int idx = inc_if_not_greator_than(continuationCount, JOB_MAX_CONTINUATIONS);

	if (idx >= JOB_MAX_CONTINUATIONS)
		return;

    continuations[idx] = node;
	node->dependencies += 1;
}

void JobNode::MoveContinuationsInto(JobNode* into)
{
	//std::unique_lock lockMe(mut);
	//std::unique_lock lockThem(into->mut);

	int size = continuationCount;

	into->continuationCount = size;
	continuationCount = 0;

	// seems like too many loops...

	for (int i = 0; i < size; i++)                   // Copy
		into->continuations[i] = continuations[i];

	for (int i = size; i < 16 - size; i++)           // Clear others
		into->continuations[i] = nullptr;

	for (int i = 0; i < size; i++)                   // Clear me
		continuations[i] = nullptr;
}

Job::Job(JobNode* node, JobTree* tree)
	: node (node)
	, tree (tree)
{}

Job& Job::SetName(const std::string & name)
{
	node->name = name;
	return *this;
}

Job& Job::ThenJob(Job& job)
{
	node->MoveContinuationsInto(job.node);
	node->AddContinuation(job.node);

	return job;
}

JobTree::JobTree(/* allocator */)
{}

JobTree::~JobTree()
{
	Cleanup();
}

std::vector<JobNode*> JobTree::GetRoots()
{
	std::vector<JobNode*> roots;

    for (JobNode* node : nodes)
        if (node->dependencies == 0)
            roots.push_back(node);
        
	return roots;
}

void JobTree::Cleanup()
{
	for (JobNode* node : nodes)
		_free_node(node);

	nodes.clear();
}

void JobTree::PrintGraphviz(std::ostream& o)
{
    // print all connections
        
    o << "digraph G {\n";
        
	for (JobNode* node : nodes) 
	{
		int count = node->continuationCount;

		for (int i = 0; i < count; i++) 
		{
			JobNode* cont = node->continuations[i];

            o << "\t\"" << node->name << "\""
                << "->"
                << "\"" << cont->name << "\"" << "\n";
		}
	}
        
    o << "}\n";
}

Job JobTree::CreateEmpty()
{
	return Job(_alloc_node(), this);
}

JobNode* JobTree::_alloc_node()
{
	JobNode* node = new JobNode();
	node->tree = this;
	node->id = jobIdNext.fetch_add(1);

	nodes.emplace(node);

	return node;
}

void JobTree::_free_node(JobNode* node)
{
	delete node;
}

JobExecutor::JobExecutor(int numberOfThreads)
{
	CreateThreads(numberOfThreads);
}

JobExecutor::~JobExecutor()
{
	DestroyThreads();
}

void JobExecutor::Run(JobTree& tree)
{
	for (JobNode* node : tree.GetRoots())
	{
		IncWaitCount(1);
		wavefront.push_front(node);
	}
}

void JobExecutor::WaitForAll()
{
	std::unique_lock lock(waitMutex);
	waitVar.wait(lock, [this]() { return workCount == 0; });
}

void JobExecutor::ThreadWork(JobThreadContext* ctx)
{
	while (true)
	{
		JobNode* node = wavefront.pop_back();

		if (!node) // stop the thread if nullptr is pushed
			break;

		//if (node->dependencies > 0) // this never gets tripped 
		//{							  // because only nodes with 0 dependencies ever get queued. Could remove
		//	wavefront.push_front(node);
		//	continue;
		//}
		
		if (node->work)
		{
			//auto t = wTimeScope(node->name.c_str());
			node->work(Job(node, node->tree));
		}

		int count = node->continuationCount.load(); // jobs can only add to themselves, so no need to lock
        for (int i = 0; i < count; i++)
        {
			JobNode* child = node->continuations[i];

            child->dependencies -= 1;
			if (child->dependencies == 0)
			{
				IncWaitCount(1);
				wavefront.push_front(child);
			}
        }

		IncWaitCount(-1);
		waitVar.notify_one();
	}
}

void JobExecutor::IncWaitCount(int c)
{
	std::unique_lock lock(waitMutex);
	workCount += c;
}

void JobExecutor::CreateThreads(int numberOfThreads)
{
	for (int i = 0; i < numberOfThreads; i++)
	{
		JobThreadContext* ctx = new JobThreadContext();
		ctx->index = i;

		JobThread thread = 
		{
			std::thread([this, ctx]() { ThreadWork(ctx); }),
			ctx
		};

		threads.emplace_back(std::move(thread));
	}
}

void JobExecutor::DestroyThreads()
{
	for (JobThread& th : threads)
		wavefront.push_back(nullptr);

	for (JobThread& th : threads)
		if (th.thread.joinable())
			th.thread.join();

	threads.clear();
}