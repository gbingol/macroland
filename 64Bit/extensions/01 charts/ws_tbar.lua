local pageCharts = std.ToolBarPage.new("Charts")

local btnScatter = std.Button.new{
	title = "scatter", 
	py = ACTIVEDIR.."/scatter.py", 
	img = ACTIVEDIR.."/icons/scatter.png"
}

local btnHistogram = std.Button.new{
	title = "histogram", 
	py = ACTIVEDIR.."/histogram.py", 
	img = ACTIVEDIR.."/icons/histogram.png"
}

local btnBoxWhisker = std.Button.new{
	title = "box-whisker", 
	py = ACTIVEDIR.."/boxwhisker.py", 
	img = ACTIVEDIR.."/icons/boxwhisker.png"
}

local btnQQ = std.Button.new{
	title = "Q-Q", 
	py = ACTIVEDIR.."/qqchart.py", 
	img = ACTIVEDIR.."/icons/qqchart.png"
}

local btnPsyChart = std.Button.new{
	title = "psychrometry", 
	py = ACTIVEDIR.."/psychart.py", 
	img = ACTIVEDIR.."/icons/psychart.png"
}

local btnMoody = std.Button.new{
	title = "Moody", 
	py = ACTIVEDIR.."/moody.py", 
	img = ACTIVEDIR.."/icons/moody.png"
}



pageCharts:add(btnScatter)
pageCharts:add(btnHistogram)
pageCharts:add(btnBoxWhisker)
pageCharts:add(btnQQ)
pageCharts:add(btnPsyChart)
pageCharts:add(btnMoody)

pageCharts:start()