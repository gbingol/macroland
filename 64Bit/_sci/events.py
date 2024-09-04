from dataclasses import dataclass

class WorkbookEvent:
	pass

class WorksheetEvent:
	pass


@dataclass
class PageChangedEvent(WorkbookEvent):
	pagechanged:str = "pagechanged"
	def __str__(self):
		return self.pagechanged
	

@dataclass
class TabRightClick(WorkbookEvent):
	tabrightclick:str = "tabrightclick"
	def __str__(self):
		return self.tabrightclick


@dataclass
class SelectingEvent(WorkbookEvent, WorksheetEvent):
	selecting: str = "selecting"
	def __str__(self):
		return self.selecting


@dataclass
class SelectedEvent(WorkbookEvent, WorksheetEvent):
	selected: str = "selected"
	def __str__(self):
		return self.selected


@dataclass
class RightClickEvent(WorkbookEvent, WorksheetEvent):
	rightclick: str = "rightclick"
	def __str__(self):
		return self.rightclick
