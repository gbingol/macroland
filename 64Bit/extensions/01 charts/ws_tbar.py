import pathlib

import _sci.extension as ext
from _sci.framework import Framework


def run(x):
	pass


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	i = -1

	btnScatter = ext.Button("scatter", str(CurFolder/"icons/scatter.png"), CurFile, run, i:=i+1)
	btnBubble = ext.Button("bubble", str(CurFolder / "icons/bubble.png"), CurFile, run, i:=i+1)
	btnHistogram = ext.Button("histogram", str(CurFolder/"icons/histogram.png"), CurFile, run, i:=i+1)
	btnBoxWhisker = ext.Button("box-whisker", str(CurFolder / "icons/boxwhisker.png"), CurFile, run, i:=i+1)
	btnColClusterChrt = ext.Button("column", str(CurFolder/"icons/col_cluster.png"), CurFile, run, i:=i+1)
	btnColStackChrt = ext.Button("column-stacked", str(CurFolder/"icons/col_stk.png"), CurFile, run, i:=i+1)

	btnQQ = ext.Button("Q-Q", str(CurFolder/"icons/qqchart.png"), CurFile, run, i:=i+1)
	
	btnPsyChart = ext.Button("psychrometry", str(CurFolder/"icons/psychart.png"), CurFile, run, i:=i+1)
	btnMoody = ext.Button("Moody", str(CurFolder/"icons/moody.png"), CurFile, run, i:=i+1)

	btnColCharts = ext.HybridButton(btnColClusterChrt)
	btnColCharts.add(btnColStackChrt)

	page = ext.Page("Charts")
	page.add(btnScatter)
	page.add(btnBubble)
	page.add(btnColCharts)
	page.add(btnHistogram)
	page.add(btnBoxWhisker)
	page.add(btnQQ)
	page.add(btnPsyChart)
	page.add(btnMoody)

	Framework().ToolBar_AddPage(page)