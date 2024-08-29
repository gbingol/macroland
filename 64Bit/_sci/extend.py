class Button:
	def __init__(self, 
					title:str, #title of the button 
					ImagePath:str, #Image of the button
					OnClick:callable) -> None:
		
		self._Title = title
		self._ImagePath = ImagePath
		self._OnClick = OnClick

	def __iter__(self):
		return iter([
			("title",self._Title),
			("img", self._ImagePath),
			("onclick", self._OnClick),
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
		self._ButtonList.append(vars(btn))

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
		self._ButtonList.append(vars(btn))

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
		self._ButtonList.append(vars(btn))

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
		assert isinstance(vars(btn), Button|DropButton|HybridButton), "Button|DropButton|HybridButton is expected"
		self._ButtonList.append(btn)

	def __iter__(self):
		return iter([
			("title",self._Title),
			("list", self._ButtonList),
			("type", "page")])