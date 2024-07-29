"""
It is recommended NOT to change or to edit this file

This file is run when the system starts, so anything wrong here will 
either prevent system from starting or will break things
"""



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

def assert_pkg(name:str, pip:str)->bool:
	
	#package already installed
	x = pkgutil.iter_modules()
	for i in x:
		if i.ispkg==True and i.name == name:
			return True
	
	PyHome = _sys.exec_prefix
	PyExe = PyHome + os.sep + "python.exe"
	Cmd = "\"" + PyExe + "\""
	Cmd += " -m pip install " + pip

	Msg = name + " is missing which is crucial for MacroLand App. Wanna install? \n \n"
	Msg += "Choosing Yes will launch the terminal and installation process using the following command: \n \n"	
	Msg += Cmd + "\n \n"
	Msg += "If you choose No, you might have to manually install the package to run the requiring app."

	YesNo = _gui.messagebox(Msg, "Install " + name + "?", yesno=True)
	if not YesNo:
		return

	os.system(Cmd)



assert_pkg("scisuit", "scisuit")
assert_pkg("wx", "wxPython")

import wx
app=wx.App(useBestVisual = True)
app.MainLoop() #there can only be one main loop in wxPython