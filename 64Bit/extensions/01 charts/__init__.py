import pathlib

import _sci.extension as ext
from _sci.framework import Framework


def run(x):
	Framework().RunPyFile(x)

if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnScatter = ext.Button("scatter", 
						CurFolder/"icons/scatter.png", 
						CurFile, 
						run, CurFolder / "scatter.py")
	
	btnBubble = ext.Button("bubble", 
						CurFolder / "icons/bubble.png", 
						CurFile, 
						run, CurFolder / "bubble.py")
	
	btnHistogram = ext.Button("histogram", 
						   CurFolder/"icons/histogram.png", 
						   CurFile, 
						   run, CurFolder / "histogram.py")
	
	btnBoxWhisker = ext.Button("box-whisker", 
							CurFolder / "icons/boxwhisker.png", 
							CurFile, 
							run, CurFolder / "boxwhisker.py")
	
	btnColClusterChrt = ext.Button("column", 
								CurFolder/"icons/col_cluster.png", 
								CurFile, 
								run, CurFolder / "col_cluster.py")
	
	btnColStackChrt = ext.Button("column-stacked", 
							  CurFolder/"icons/col_stk.png", 
							  CurFile, 
							  run, CurFolder / "col_stk.py")

	btnQQ = ext.Button("Q-Q", 
					CurFolder/"icons/qqchart.png", 
					CurFile, 
					run, CurFolder / "qqchart.py")
	
	btnPsyChart = ext.Button("psychrometry", 
						  CurFolder/"icons/psychart.png", 
						  CurFile, 
						  run, CurFolder / "psychart.py")
	
	btnMoody = ext.Button("Moody", 
					   CurFolder/"icons/moody.png", 
					   CurFile, 
					   run, CurFolder / "moody.py")

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