import os
import pkgutil
import sys as _sys

from __SCISUIT import GUI as _gui # type: ignore
from __SCISUIT import COMMANDWINDOW as _cmd # type: ignore


#Version of scisuit Python package with which this version of MacroLand Framework tested
_SCISUIT_PKG_VERSION = "1.3.6"



def PyExe()->str:
	return _sys.exec_prefix + os.sep + "python.exe"


def installcrucialpkg(name:str):
	
	x = pkgutil.iter_modules()
	for i in x:
		if i.ispkg and i.name == name:
			return
	
	Cmd = "\"" + PyExe() + "\""
	Cmd += " -m pip install scisuit=="+_SCISUIT_PKG_VERSION + " wxPython" #install both packages in one go

	Msg = f"""{name} Python package is missing. Wanna install?

Choosing Yes will start the installation process using the following command:	

{Cmd}

If you choose No, then package(s) must be manually installed to the above-shown path."""

	YesNo = _gui.messagebox(Msg, f"Install {name}?", yesno=True)
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


class SYS_StdOutput:
	def __init__(self):
		self.value = ''
	def write(self, txt):
		self.value += txt

SYSCATCHSTDOUTPUT = SYS_StdOutput()
_sys.stdout = SYSCATCHSTDOUTPUT
_sys.stderr = SYSCATCHSTDOUTPUT

_cmd.__dict__["SYSCATCHSTDOUTPUT"] = SYSCATCHSTDOUTPUT

