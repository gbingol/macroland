import pathlib
import wx

import _sci.extension as ext
from _sci.framework import Framework

def run(x):
	wx.MessageBox(x)

if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnHello = ext.Button("Hello", CurFolder/"py_logo32.png", CurFile, run, "Hello World")
	
	page = ext.Page("My Page")
	page.add(btnHello)

	Framework().ToolBar_AddPage(page)

