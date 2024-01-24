local btnFoodDB = std.Button.new{
	title = "Food DB", 
	py = ACTIVEDIR.."/frmFoodDatabase.py", 
	img = ACTIVEDIR.."/icons/fooddatabase.jpg"}

local btnThermalProc = std.Button.new{
	title = "Food Thermal Proc", 
	py = ACTIVEDIR.."/frmFoodThermalProc.py", 
	img = ACTIVEDIR.."/icons/thermalprocessing.jpg"}


local btnPsychrometry = std.Button.new{
	title = "Psychrometry", 
	py = ACTIVEDIR.."/frmPsychrometry.py", 
	img = ACTIVEDIR.."/icons/psycalc.png"}


local pageEng = std.ToolBarPage.new("Process Eng") 
pageEng:add(btnFoodDB)
pageEng:add(btnThermalProc)
pageEng:add(btnPsychrometry)

pageEng:start()