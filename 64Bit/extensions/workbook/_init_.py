import pathlib

import _sci.extension as ext
from _sci import Framework, Workbook, RightClickEvent, TabRightClick


def runfile(x):
	Framework().RunPyFile(x)



def Save(param):
	import numpy as np
	import wx

	try:
		ws = Workbook().activeworksheet()
		rng = ws.selection()
		if rng.ncols()<2:
			raise RuntimeError("At least 2 columns of data required")
		
		MainLst = rng.tolist(axis = 1)

		Framework.Enable(False)
		
		wildcard = "Text files (*.txt)|*.txt"  if param =="txt" else "CSV files (*.csv)|*.csv"
		dlg = wx.FileDialog(None, style=wx.FD_SAVE, wildcard= wildcard)
		dlg.ShowModal()

		Framework.Enable(True)
		
		path = str(dlg.GetPath())
		if path =="":
			return
		
		arr = np.array(MainLst)
		arr[arr == None] = ""
		
		np.savetxt(path, arr, delimiter="\t" if param == "txt" else ",", fmt="%s")
	except Exception as e:
		Framework().messagebox(str(e),  "Export Error")



def _rightclick(file):
	CurFolder = pathlib.Path(file).parent

	btnCreateVar = ext.Button("Create Variable...", 
					  CurFolder/"icons/py_logo32.png",  
					  runfile, CurFolder / "frmCreateVar.py")

	btnImportVar = ext.Button("Process and/or Import...", 
					CurFolder/"icons/py_logo32.png",  
					runfile, CurFolder / "processimport.py")


	Workbook().AppendMenuItem() #menu separator
	Workbook().AppendMenuItem(btnCreateVar)
	Workbook().AppendMenuItem(btnImportVar)


	btnExport_Txt = ext.Button("Text File", 
					"",  
					Save, "txt")

	btnExport_CSV = ext.Button("CSV File", 
					"",  
					Save, "csv")
	

	menuExport = ext.Menu("Export", CurFolder / "icons/export.png")

	menuExport.add(btnExport_CSV)
	menuExport.add(btnExport_Txt)

	Workbook().AppendMenuItem(menuExport)



def _tabrightclick(file):
	CurFolder = pathlib.Path(file).parent

	btndlgImport = ext.Button("Import", 
					  CurFolder/"icons/import.png", 
					  runfile, CurFolder / "frmImport.py")

	Workbook().AppendTabMenuItem(btndlgImport)



def setuppage(file):
	CurFolder = pathlib.Path(file).parent

	btnSort = ext.Button("Sort", 
					  CurFolder/"icons/sort.jpg",  
					  runfile, CurFolder / "frmSort.py")
	
	btnRemoveDups = ext.Button("Remove Duplicates", 
							CurFolder/"icons/remove_duplicates.png",  
							runfile, CurFolder / "removedups.py")
	
	btnTxt2Cols = ext.Button("Text to Columns", 
						  CurFolder/"icons/txt_to_cols.png",  
						  runfile, CurFolder / "frmDelimTxt.py")
	
	
	page = ext.Page("Home")
	page.add(btnSort)
	page.add(btnRemoveDups)
	page.add(btnTxt2Cols)

	Framework().ToolBar_AddPage(page)



if __name__ == "__main__":
	setuppage(__file__)
	Workbook().bind(RightClickEvent(), _rightclick, __file__)
	Workbook().bind(TabRightClick(), _tabrightclick, __file__)
