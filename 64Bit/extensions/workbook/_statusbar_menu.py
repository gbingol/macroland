import pathlib

import _sci.extension as ext
from _sci import Framework, StatusBar

import wx

def Open(URL):
	wx.LaunchDefaultBrowser(URL)


if __name__ == "__main__":
	CurFile = str(__file__)
	CurFolder = pathlib.Path(__file__).parent

	btnSciSuit = ext.Button("PE Bytes", str(CurFolder/"icons/pebytes.png"), CurFile, Open, "https://www.pebytes.com")
	btnPython = ext.Button("Python", str(CurFolder/"icons/py_logo32.png"), CurFile, Open, "https://www.python.org/")
	btnNumpy = ext.Button("Numpy", str(CurFolder/"icons/numpy.png"), CurFile, Open, "https://www.numpy.org")
	btnPandas = ext.Button("Pandas", str(CurFolder/"icons/pandas.png"), CurFile, Open, "https://pandas.pydata.org/")
	
	
	StatusBar().AppendMenuItem(1, btnSciSuit) 

	StatusBar().AppendMenuItem(3, btnPython) 
	StatusBar().AppendMenuItem(3, btnNumpy)
	StatusBar().AppendMenuItem(3, btnPandas)
