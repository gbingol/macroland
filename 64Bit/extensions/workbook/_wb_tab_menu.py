import pathlib

import _sci.extension as ext
from _sci import Framework, Workbook


def runfile(x):
	Framework().RunPyFile(x)


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btndlgImport = ext.Button("Import", 
					  CurFolder/"icons/import.png", 
					  CurFile, 
					  runfile, CurFolder / "frmImport.py")

	Workbook().AppendTabMenuItem(btndlgImport)