local pageWB = std.ToolBarPage.new("Data Tools")

local btnSort = std.Button.new{
	title = "Sort", 
	py = ACTIVEDIR.."/frmSort.py", 
	img = ACTIVEDIR.."/icons/sort.jpg"
}


local btnRemoveDups = std.Button.new{
	title = "Remove Duplicates", 
	py = ACTIVEDIR.."/removedups.py", 
	img = ACTIVEDIR.."/icons/remove_duplicates.png"
}

local btnTxt2Cols = std.Button.new{
	title = "Text to Columns", 
	py = ACTIVEDIR.."/frmDelimTxt.py", 
	img = ACTIVEDIR.."/icons/txt_to_cols.png"
}


pageWB:add(btnSort)
pageWB:add(btnRemoveDups)
pageWB:add(btnTxt2Cols)

pageWB:start()