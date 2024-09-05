class WorkbookEvent:
	pass

class WorksheetEvent:
	pass

class StatusBarEvent:
	pass


class PageChangedEvent(WorkbookEvent):
	def __str__(self):
		return "pagechanged"
	

class TabRightClick(WorkbookEvent):
	"""
	A right-click made on the Workbook's tab
	"""
	def __str__(self):
		return "tabrightclick"


class SelectingEvent(WorkbookEvent, WorksheetEvent):
	"""
	Selecting is defined as:
	1) Mouse left button is pressed during selection,
	2) Selection is made using arrows + shift key
	"""
	def __str__(self):
		return "selecting"


class SelectedEvent(WorkbookEvent, WorksheetEvent):
	"""
	Selected is defined as:
	1) Mouse left button is release after a selection,
	2) Selection is made using arrows + shift key
	"""
	def __str__(self):
		return "selected"


class RightClickEvent(WorkbookEvent, WorksheetEvent, StatusBarEvent):
	def __str__(self):
		return "rightclick"
