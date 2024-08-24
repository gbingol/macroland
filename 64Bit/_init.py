import os
import pkgutil
import sys
import pathlib

import __SCISUIT #type: ignore


#Version of scisuit Python package with which this version of MacroLand Framework tested
_SCISUIT_PKG_VERSION = "1.3.7"



#In case a venv is used, we must ensure that the MacroLand folder's path is in sys.path
sys.path.insert(0, str(pathlib.Path(__file__).parent))



def PyExe()->str:
	return sys.exec_prefix + os.sep + "python.exe"



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

	YesNo = __SCISUIT.GUI.messagebox(Msg, f"Install {name}?", yesno=True)
	if not YesNo:
		return

	os.system(Cmd)


installcrucialpkg("scisuit")
installcrucialpkg("wx")



class SYS_StdOutput:
	def __init__(self):
		self.value = ''
	def write(self, txt):
		self.value += txt

SYSCATCHSTDOUTPUT = SYS_StdOutput()
sys.stdout = SYSCATCHSTDOUTPUT
sys.stderr = SYSCATCHSTDOUTPUT

__SCISUIT.__dict__["_SYSCATCHSTDOUTPUT"] = SYSCATCHSTDOUTPUT #type:ignore


"""
As of this point it is assumed that wxPython and scisuit are already installed.
If not, MacroLand App should still start but most of the functionality will not work!
"""
import wx
app = wx.App(useBestVisual = True)
app.MainLoop() #there can only be one main loop


