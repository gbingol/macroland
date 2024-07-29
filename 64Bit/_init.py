
"""
When designing wxPython apps do not use app.MainLoop in your application
since there can only be one main loop in wxPython in a single process. 

Attempting to use app.MainLoop elsewhere will crash the whole system 
unless a subprocess is used.
"""

import os
import pkgutil
import sys as _sys

from __SCISUIT import GUI as _gui # type: ignore


def installcrucialpkg(name:str):
	
	x = pkgutil.iter_modules()
	for i in x:
		if i.ispkg==True and i.name == name:
			return
	
	PyHome = _sys.exec_prefix
	PyExe = PyHome + os.sep + "python.exe"
	Cmd = "\"" + PyExe + "\""
	Cmd += " -m pip install scisuit wxPython" #install both in one go

	Msg = name + " is missing which is crucial for MacroLand App. Wanna install? \n \n"
	Msg += "Choosing Yes will launch the terminal and installation process using the following command: \n \n"	
	Msg += Cmd + "\n \n"
	Msg += "If you choose No, you might have to manually install to the above-shown path."

	YesNo = _gui.messagebox(Msg, "Install " + name + "?", yesno=True)
	if not YesNo:
		return

	os.system(Cmd)


installcrucialpkg("scisuit")
installcrucialpkg("wx")

import wx
app=wx.App(useBestVisual = True)
app.MainLoop() #there can only be one main loop in wxPython