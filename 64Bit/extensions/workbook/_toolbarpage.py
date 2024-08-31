import pathlib

import _sci.extension as ext
from _sci.framework import Framework


def runfile(x):
	Framework().RunPyFile(x)


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnSort = ext.Button("Sort", 
					  CurFolder/"icons/sort.jpg", 
					  CurFile, 
					  runfile, CurFolder / "frmSort.py")
	
	btnRemoveDups = ext.Button("Remove Duplicates", 
							CurFolder/"icons/remove_duplicates.png", 
							CurFile, 
							runfile, CurFolder / "removedups.py")
	
	btnTxt2Cols = ext.Button("Text to Columns", 
						  CurFolder/"icons/txt_to_cols.png", 
						  CurFile, 
						  runfile, CurFolder / "frmDelimTxt.py")
	
	btnExtensionMngr = ext.Button("Extensions", 
						  CurFolder/"icons/extensions.png", 
						  CurFile, 
						  runfile, CurFolder / "frmextmngr.py")
	
	
	page = ext.Page("Home")
	page.add(btnSort)
	page.add(btnRemoveDups)
	page.add(btnTxt2Cols)
	page.add(btnExtensionMngr)

	Framework().ToolBar_AddPage(page)
