#pragma once

#include "Leveling.h"
#include "imgui/imgui.h"
#include "imgui/implot.h"

struct System_Metrics : SystemBase
{
	metrics_log* log;

	void Init()
	{
		log = GetLevel()->GetApp()->GetMetrics();
	}

	void UI()
	{
		id = 0;

		ImGui::Begin("Metrics");

		for (metrics_log::series* series : log->get_series()) // iterate root level
		{
			UIDrawSeriesLevel(series);
		}

		ImGui::End();
	}

private:

	int id = 0;

	void UIDrawSeriesLevel(const metrics_log::series* series)
	{
		if (series->durations.size() == 0) return;

		if (ImGui::TreeNode((void*)id, "%s - % .2fms", series->name, series->durations.back().time * 1000.f))
		{
			for (metrics_log::series* child : series->children)
			{
				UIDrawSeriesLevel(child);
			}

			ImGui::TreePop();
		}

		id += 1;
	}
};