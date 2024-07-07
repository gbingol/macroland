local pageCharts = std.ToolBarPage.new("Charts")

local btnScatter = std.Button.new{
	title = "scatter", 
	py = ACTIVEDIR.."/scatter.py", 
	img = ACTIVEDIR.."/icons/sort.jpg"
}



pageCharts:add(btnScatter)


pageCharts:start()