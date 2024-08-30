from __SCISUIT import EXTENSION as _extension # type: ignore
import _sci.extension as ext
import wx

def dum():
    wx.MessageBox("dumdum")

btn = ext.Button("dummy", "", dum)
page = ext.Page("dummy page")
page.add(btn)


_extension.addpage(dict(page))