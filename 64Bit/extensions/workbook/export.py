import wx
import numpy as np

from _sci import activeworksheet



def Save(param):
	wildcard = "Text files (*.txt)|*.txt"  if param =="txt" else "CSV files (*.csv)|*.csv"
	dlg = wx.FileDialog(None, style=wx.FD_SAVE, wildcard= wildcard)
	dlg.ShowModal()
	
	path = str(dlg.GetPath())
	if path =="":
		return
	
	ws = activeworksheet()
	rng = ws.selection()
	MainLst = rng.tolist(axis = 1)
	
	arr = np.array(MainLst)
	arr[arr == None] = ""
	
	np.savetxt(path, arr, delimiter="\t" if param == "txt" else ",", fmt="%s")