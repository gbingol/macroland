
from __SCISUIT import GUI as __gui # type: ignore
from __SCISUIT import COMMANDWINDOW as __cmd # type: ignore



CommandWindowDict:dict = __cmd.__dict__



def statbar_write(text:str, n:int)->None:
	"""
	Writes the text to nth field in the status bar \n
	Note that n starts from 0.
	"""
	assert isinstance(text, str), "text must be string."
	assert isinstance(n, int), "n must be int."

	__gui.statbar_write(text, n)
	


def messagebox(msg:str, caption:str)->None:
	"""
	Display a messagebox
	Unlike wx.MessageBox, owner of the messagebox is the framework
	"""
	return __gui.messagebox(msg, caption)




from .widgets import Frame, GridTextCtrl, pnlOutputOptions, NumTextCtrl
from .util import assert_pkg, parent_path, pyhomepath, colnum2label
from .icell import Workbook, Worksheet, Range