import pathlib

import _sci.extension as ext
from _sci.framework import Framework


def run(x):
	Framework().RunPyFile(x)


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnFoodDB = ext.Button("Food DB", 
							CurFolder/"icons/fooddatabase.jpg", 
							run, CurFolder/"frmFoodDatabase.py")
	
	btnThermalProc = ext.Button("Food Thermal Proc", 
							 	CurFolder/"icons/thermalprocessing.jpg", 
								run, CurFolder/"frmFoodThermalProc.py")
	
	btnPsychrometry = ext.Button("Psychrometry", 
							  	CurFolder/"icons/psycalc.png", 
								run, CurFolder/"frmPsychrometry.py")
	
	page = ext.Page("Process Eng")
	page.add(btnFoodDB)
	page.add(btnThermalProc)
	page.add(btnPsychrometry)

	Framework().ToolBar_AddPage(page)

