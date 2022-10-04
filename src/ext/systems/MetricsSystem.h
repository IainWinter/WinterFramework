#pragma once

#include "app/System.h"
#include "imgui/imgui.h"
#include "imgui/implot.h"
#include "util/metrics.h"

struct System_Metrics : SystemBase
{
	void Debug()
	{
		id = 0;

		ImGui::Begin("Metrics");

		for (met_series* series : met_get_series()) // iterate root level
		{
			UIDrawSeriesLevel(series);
		}

		ImGui::End();
	}

private:

	int id = 0;

	void UIDrawSeriesLevel(const met_series* series)
	{
		if (series->durations.size() == 0) return;

		if (ImGui::TreeNode((void*)id, "%s - % .2fms", series->name, series->durations.back().time * 1000.f))
		{
			for (met_series* child : series->children)
			{
				UIDrawSeriesLevel(child);
			}

			ImGui::TreePop();
		}

		id += 1;
	}
};