
import _sci.extension as ext
from _sci.framework import Framework
import wx

def dum():
    wx.MessageBox("dumdum")


if __name__ == "__main__":
	btn = ext.Button("dummy", "", str(__file__), dum)
	page = ext.Page("dummy page")
	page.add(btn)

	Framework().AddToolBarPage(page)
	