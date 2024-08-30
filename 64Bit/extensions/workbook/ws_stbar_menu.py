import pathlib

import _sci.extension as ext
from _sci.framework import Framework

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
	
	
	Framework().StatusBar().AppendMenuItem(1, btnSciSuit) 

	Framework().StatusBar().AppendMenuItem(3, btnPython) 
	Framework().StatusBar().AppendMenuItem(3, btnNumpy)
	Framework().StatusBar().AppendMenuItem(3, btnPandas)
