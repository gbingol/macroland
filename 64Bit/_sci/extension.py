import pathlib

class Button:
	def __init__(self, 
					title:str, #title of the button 
					ImagePath:str, #Image of the button
					click:callable) -> None:
		
		self._Title = title
		self._ImagePath = ImagePath

		filePath = pathlib.Path(__file__)
		
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

		self._click = click


	def __iter__(self):
		return iter([
			("title",self._Title),
			("img", self._ImagePath),
			("click", self._click),
			("module", self._ModulePath), #relative to extensions folder
			("type", "button")])





class DropButton:
	def __init__(self,
					title:str, #title of the button 
					ImagePath:str) -> None:
		
		self._Title = title
		self._ImagePath = ImagePath
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
		self._mainButton = mainButton
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
					ImagePath:str = None) -> None:
		
		self._Title = title
		self._ImagePath = ImagePath
		self._ButtonList = []

	def add(self, btn:Button):
		assert isinstance(btn, Button), "A Button is expected"
		self._ButtonList.append(dict(btn))

	def __iter__(self):
		return iter([
			("title",self._Title),
			("img", self._ImagePath),
			("list", self._ButtonList),
			("type", "menu")])
    




class Page:
	def __init__(self, title:str) -> None:
		self._Title = title
		self._ButtonList = []

	def add(self, btn:Button|DropButton|HybridButton):
		assert isinstance(btn, Button|DropButton|HybridButton), "Button|DropButton|HybridButton is expected"
		self._ButtonList.append(dict(btn))

	def __iter__(self):
		return iter([
			("title",self._Title),
			("list", self._ButtonList),
			("type", "page")])