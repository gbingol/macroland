import pathlib
import wx

import _sci.extension as ext
from _sci.icell import Workbook

def run(x):
	wx.MessageBox(x)

if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnHello = ext.Button("Hello", CurFolder/"py_logo32.png", CurFile, run, "Hello World")
	
	Workbook().AppendMenuItem() #menu separator
	Workbook().AppendMenuItem(btnHello)