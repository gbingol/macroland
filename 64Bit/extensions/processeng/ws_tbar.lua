local btnFoodDB = std.Button.new{
	title = "Food DB", 
	py = ACTIVEDIR.."/frmFoodDatabase.py", 
	img = ACTIVEDIR.."/icons/fooddatabase.jpg"}

local btnThermalProc = std.Button.new{
	title = "Food Thermal Proc", 
	py = ACTIVEDIR.."/frmFoodThermalProc.py", 
	img = ACTIVEDIR.."/icons/thermalprocessing.jpg"}




local pageEng = std.ToolBarPage.new("Process Eng") 
pageEng:add(btnFoodDB)
pageEng:add(btnThermalProc)

pageEng:start()