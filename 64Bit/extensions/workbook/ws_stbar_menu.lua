
local MDL_OPNBROWS = "extensions."..ACTIVEDIR_STEM..".urls.openbrowser"

local btnSciSuit = std.Button.new{
	title = "PE Bytes", 
	module = MDL_OPNBROWS, 
	call = "Open",
	param = "https://www.pebytes.com",
	img = ACTIVEDIR.."/icons/pebytes.png"}


local btnPython = std.Button.new{
	title = "Python", 
	module = MDL_OPNBROWS, 
	call = "Open",
	param = "https://www.python.org/",
	img = ACTIVEDIR.."/icons/py_logo32.png"}

local btnNumpy = std.Button.new{
	title = "Numpy", 
	module = MDL_OPNBROWS, 
	call = "Open",
	param = "https://www.numpy.org",
	img = ACTIVEDIR.."/icons/numpy.png"}

local btnPandas = std.Button.new{
	title = "Pandas", 
	module = MDL_OPNBROWS, 
	call = "Open",
	param = "https://pandas.pydata.org/",
	img = ACTIVEDIR.."/icons/pandas.png"}



std.append(btnSciSuit, 1) -- append to 1st rectangle field
 --std.append(3) --separator

std.append(btnPython, 3) -- append to 3rd rectangle field
std.append(btnNumpy, 3)
std.append(btnPandas, 3)