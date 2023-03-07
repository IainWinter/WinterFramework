#pragma once

#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>

template<typename _t>
struct graph
{
	// the list of nodes
	std::vector<_t> nodes;

	// an map from node to its index
	std::unordered_map<_t, int> index;

	// map of edges
	// from -> [to1, to2, to3, ...]
	std::unordered_map<int, std::unordered_set<int>> adj;

	// add a node and return its index
	int add_node(const _t& node)
	{
		nodes.push_back(node);

		int i = nodes.size() - 1;

		index[node] = i;
		return i;
	}

	// return the index of a node from its value
	int node_index(const _t& node) const
	{
		return index.count(node) == 0 ? -1 : index.at(node);
	}

	// connect two nodes from their values
	void add_edge(const _t& from, const _t& to)
	{
		add_edge(node_index(from), node_index(to));
	}

	// connect two nodes from their indices
	void add_edge(int from, int to)
	{
		adj[from].emplace(to);
	}

	// return the depth of a node from its value
	int depth(const _t& node) const
	{
		return depth(node_index(node));
	}

	// return the depth of a node from its index
	int depth(int index) const
	{
		int depth = 0;

		std::stack<std::pair<int, int>> above; // index, depth
		above.push({index, 0});

		while (above.size() > 0)
		{
			const auto& [i, d] = above.top(); above.pop();

			if (adj.count(i))
			for (int next : adj.at(i))
			{
                int d1 = d + 1;
                
				above.push({ next, d1 });
                depth = d1 > depth ? d1 : depth;
			}
		}

		return depth;
	}
};
