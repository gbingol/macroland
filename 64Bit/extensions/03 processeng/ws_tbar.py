import pathlib

import _sci.extension as ext
from _sci.framework import Framework


def runfile(x):
	from .frmFoodThermalProc import run as ftrun
	from .frmFoodDatabase import run as fdrun
	from .frmPsychrometry import run as psrun

	[fdrun, ftrun, psrun][x]()


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnFoodDB = ext.Button("Food DB", str(CurFolder/"icons/fooddatabase.jpg"), CurFile, runfile, 0)
	btnThermalProc = ext.Button("Food Thermal Proc", str(CurFolder/"icons/thermalprocessing.jpg"), CurFile, runfile, 1)
	btnPsychrometry = ext.Button("Psychrometry", str(CurFolder/"icons/psycalc.png"), CurFile, runfile, 2)
	
	page = ext.Page("Process Eng 2")
	page.add(btnFoodDB)
	page.add(btnThermalProc)
	page.add(btnPsychrometry)

	Framework().ToolBar_AddPage(page)

