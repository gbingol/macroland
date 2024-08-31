import pathlib

import _sci.extension as ext
from _sci import Framework, Workbook


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


	btnExport_Txt = ext.Button("Text File", 
					"", 
					CurFile, 
					Save, "txt")

	btnExport_CSV = ext.Button("CSV File", 
					"", 
					CurFile, 
					Save, "csv")
	
	

	menuExport = ext.Menu("Export", CurFolder / "icons/export.png")

	menuExport.add(btnExport_CSV)
	menuExport.add(btnExport_Txt)


	Workbook().AppendMenuItem(menuExport)