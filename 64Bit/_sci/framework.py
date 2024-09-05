from __SCISUIT import GUI as _gui # type: ignore
from __SCISUIT import EXTENSION as _extension # type: ignore

import types as _types

from .extension import Page, Button
from .events import StatusBarEvent, RightClickEvent

from pathlib import Path


class Framework():
	@staticmethod
	def messagebox(msg:str, caption:str = "Message", yesno:bool = False)->None:
		"""
		Displays a messagebox. 

		Unlike wx.MessageBox, the owner of the messagebox is the framework and therefore 
		when its shown it blocks the framework.

		---
		Note that wx.MessageBox can cause crashes in the framework, so unless absolutely 
		needed, try using this messagebox function.
		---

		msg: Message to be shown.  
		caption: Caption of the messagebox dialog.  
		yesno: Depending on the value shows different messageboxes.  
		1) if True then shows a messagebox with Yes and No options. 
		If user selects Yes then returns 1 otherwise returns 0.
		2) If False, then shows a messagebox with only OK button. 
		"""
		assert isinstance(msg, str), "msg must be string."
		assert isinstance(caption, str), "caption must be string."
		assert isinstance(yesno, bool), "yesno must be bool."
		return _gui.messagebox(msg, caption, yesno)


	@staticmethod
	def Enable(enable=True):
		"""Enable/disable whole framework (CommandWindow and Workbook)"""
		_gui.enable(enable)
	

	@staticmethod
	def RunPyFile(path:str|Path):
		"""
		Runs a Python file.  
		Although the built-in exec command can be used, it fails when the script
		imports functions from _sci library.
		"""
		assert isinstance(path, str | Path), "path must be str | Path."
		_extension.runpythonfile(str(path))





class ToolBar():
	@staticmethod
	def AddPage(page:Page):
		"""
		Adds a new page to the main toolbar.  
		`page` must be _sci.extension.Page object
		"""
		assert isinstance(page, Page), "page must be Page object."
		_extension.addtoolbarpage(dict(page))





class StatusBar():
	@staticmethod
	def bind(event:StatusBarEvent, func:_types.FunctionType, n:int, *args)->None:
		"""
		Binds a callback function to the nth field in the status bar 

		---

		event: WorkbookEvent  
		func: A function that will be called when event happens  
		n: field number
		args: Any parameter of the func.
		"""
		assert isinstance(event, StatusBarEvent), "event argument must StatusBarEvent"
		assert isinstance(func, _types.FunctionType), "func argument must be function"
		assert isinstance(n, int), "n must be int."
		assert n>=0, "n>=0 expected."

		_gui.statbar_Bind(str(event) + f"_{n}", func, args)
	
	
	@staticmethod
	def unbind(event:StatusBarEvent, n:int, func:_types.FunctionType)->None:
		"""unbinds the function that was bound with given signature"""
		assert isinstance(event, StatusBarEvent), "event argument must StatusBarEvent"
		assert isinstance(func, _types.FunctionType), "func argument must be function"
		assert isinstance(n, int), "n must be int."
		assert n>=0, "n>=0 expected."

		_gui.statbar_Unbind(str(event) + f"_{n}", func)


	@staticmethod
	def writetext(text:str, n:int)->None:
		"""
		Writes the text to nth field in the status bar  
		Note that n starts from 0.
		"""
		assert isinstance(text, str), "text must be string."
		assert isinstance(n, int), "n must be int."

		_gui.statbar_write(text, n)

	@staticmethod
	def readtext(n:int)->str|None:
		"""
		Gets the text of nth field in the status bar  
		Note that n starts from 0.
		"""
		assert isinstance(n, int), "n must be int."
		assert n>=0, "n>=0 expected."

		return _gui.statbar_gettext(n)


	@staticmethod
	def AppendMenuItem(field:int, button:Button|None = None):
		"""
		Appends a button to statusbar's context menu shown in a field.  
		`field`: A number between 1-3.
		`button` must be _sci.extension.Button object

		---
		Note:  
		1) Statusbar is currently divided into 3 regions (fields).
		2) If button is None, then appends a menu seperator.
		"""
		assert isinstance(field, int), "field must be int."
		assert 1<=field<=3, "1<=field<=3 expected."
		assert isinstance(button, Button), "button must be Button object."
		_extension.statbar_contextmenu_append(field, dict(button) if button!=None else None)
	
		