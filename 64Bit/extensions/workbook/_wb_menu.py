import pathlib

import _sci.extension as ext
from _sci.framework import Framework
from _sci.icell import Workbook

def runfile(x):
	Framework().RunPyFile(x)


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnCreateVar = ext.Button("Create Variable...", 
					  CurFolder/"icons/py_logo32.png", 
					  CurFile, 
					  runfile, CurFolder / "frmCreateVar.py")

	btnImportVar = ext.Button("Process and/or Import...", 
					CurFolder/"icons/py_logo32.png", 
					CurFile, 
					runfile, CurFolder / "processimport.py")


	Workbook().AppendMenuItem() #menu separator
	Workbook().AppendMenuItem(btnCreateVar)
	Workbook().AppendMenuItem(btnImportVar)


"""
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

"""