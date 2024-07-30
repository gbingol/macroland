local btnCreateVar = std.Button.new{
	title = "Create Variable...", 
	py = ACTIVEDIR.."/frmCreateVar.py", 
	img = ACTIVEDIR.."/icons/py_logo32.png"}


local btnImportVar = std.Button.new{
	title = "Process and/or Import...", 
	py = ACTIVEDIR.."/importvar.py", 
	img = ACTIVEDIR.."/icons/py_logo32.png"}


std.append() --menu separator
std.append(btnCreateVar)
std.append(btnImportVar)



local MDL_EXPORT = "extensions."..ACTIVEDIR_STEM..".export"

--Export Menu
local btnExport_Txt = std.Button.new{
	title = "Text File", 
	module = MDL_EXPORT,
	call = "Save",
	param = "txt"}

local btnExport_CSV = std.Button.new{
	title = "CSV File", 
	module = MDL_EXPORT,
	call = "Save",
	param = "csv"}

local menuExport = std.Menu.new{
	title = "Export", 
	img = ACTIVEDIR.."/icons/export.png"}

menuExport:add(btnExport_CSV)
menuExport:add(btnExport_Txt)


std.append() 
std.append(menuExport)