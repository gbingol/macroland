import os
import pkgutil
import sys as _sys

from __SCISUIT import GUI as _gui # type: ignore



def installcrucialpkg(name:str):
	
	x = pkgutil.iter_modules()
	for i in x:
		if i.ispkg and i.name == name:
			return
	
	PyHome = _sys.exec_prefix
	PyExe = PyHome + os.sep + "python.exe"
	Cmd = "\"" + PyExe + "\""
	Cmd += " -m pip install scisuit wxPython" #install both packages in one go

	Msg = f"""{name} is missing which is crucial for MacroLand App. Wanna install?

Choosing Yes will launch the terminal and installation process using the following command:	

{Cmd}

If you choose No, then you have to manually install package(s) to the above-shown path."""

	YesNo = _gui.messagebox(Msg, "Install " + name + "?", yesno=True)
	if not YesNo:
		return

	os.system(Cmd)


installcrucialpkg("scisuit")
installcrucialpkg("wx")


"""
As of this point it is assumed that wxPython and scisuit are already installed.
If not, MacroLand App should still start but most of the functionality will not work!
"""
import wx
app = wx.App(useBestVisual = True)
app.MainLoop() #there can only be one main loop