import wx
import numpy as np

from _sci import Workbook, Framework



def Save(param):
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