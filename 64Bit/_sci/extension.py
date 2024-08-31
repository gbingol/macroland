import pathlib
from types import FunctionType

class Button:
	def __init__(self, 
					title:str, #title of the button 
					image:str, #Image of the button
					file:str,
					click:FunctionType|str,
					*args) -> None:
		"""
		title: Title to be shown on the button/menu ...  
		image: Absolute full path of the image file (.jpg, .png, .bmp, .jpeg)  
		file: The absolute full path of the file where `click` calls the function. In most cases 
		equal to `str(__file__)`  
		click: The function or the function name to be called when button/menu item is clicked.  
		args: Parameter(s) to `click` function
		"""

		assert isinstance(title, str), "title must be string"
		assert isinstance(image, str | pathlib.Path), "image must be string | Path"
		assert isinstance(file, str), "file must be string"
		assert isinstance(click, FunctionType | str), "click must be callable or str"
		#assert isinstance(args, tuple), "args must be tuple."
		
		self._Title = title
		self._ImagePath = image if isinstance(image, str) else str(image)
		self._click = click
		self._args = args

		filePath = pathlib.Path(file)
		
		parentFold = filePath.parent
		StemList = [filePath.stem, parentFold.stem]

		depth, MAXDEPTH = 0, 5
		while parentFold.stem != "extensions": #top-level extensions folder
			parentFold = parentFold.parent
			StemList.append(parentFold.stem)
			
			depth += 1
			if depth == MAXDEPTH:
				raise RuntimeError("Cannot find modules location (must be in extensions folder)") 


		StemList.reverse()
		self._ModulePath = ".".join(StemList)
		


	def __iter__(self):
		return iter([
			("title",self._Title),
			("img", self._ImagePath),
			("click", self._click.__name__ if isinstance(self._click, FunctionType) else self._click),
			("module", self._ModulePath), #relative to extensions folder
			("args", self._args),
			("type", "button")])





class DropButton:
	def __init__(self,
				title:str, #title of the button 
				image:str) -> None:
		
		assert isinstance(title, str), "title must be string"
		assert isinstance(image, str), "image must be string"
		
		self._Title = title
		self._ImagePath = image
		self._ButtonList = []

	def add(self, btn:Button):
		assert isinstance(btn, Button), "A Button is expected"
		self._ButtonList.append(dict(btn))

	def __iter__(self):
		return iter([
			("title",self._Title),
			("img", self._ImagePath),
			("list", self._ButtonList),
			("type", "dropbutton")])




class HybridButton:
	def __init__(self, mainButton:Button) -> None:
		assert isinstance(mainButton, Button), "mainButton must be Button"

		self._mainButton = dict(mainButton)
		self._ButtonList = []

	def add(self, btn:Button):
		assert isinstance(btn, Button), "A Button is expected"
		self._ButtonList.append(dict(btn))

	def __iter__(self):
		return iter([
			("mainbutton",self._mainButton),
			("list", self._ButtonList),
			("type", "hybridbutton")])





class Menu:
	def __init__(self,
					title:str, #title of the menu 
					image:str = None) -> None:
		
		assert isinstance(title, str), "title must be string"
		assert isinstance(image, str), "image must be string"
		
		self._Title = title
		self._ImagePath = image
		self._ButtonList = []

	def add(self, btn:Button):
		assert isinstance(btn, Button), "A Button object is expected."
		self._ButtonList.append(dict(btn))

	def __iter__(self):
		return iter([
			("title",self._Title),
			("img", self._ImagePath),
			("list", self._ButtonList),
			("type", "menu")])
    




class Page:
	def __init__(self, title:str) -> None:
		assert isinstance(title, str), "title must be string"

		self._Title = title
		self._ButtonList = []

	def add(self, btn:Button|DropButton|HybridButton):
		assert isinstance(btn, Button|DropButton|HybridButton), "Button|DropButton|HybridButton object is expected."
		self._ButtonList.append(dict(btn))

	def __iter__(self):
		return iter([
			("title",self._Title),
			("list", self._ButtonList),
			("type", "page")])