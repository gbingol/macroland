import json
import os
import sys
import subprocess
from pathlib import Path
from dataclasses import dataclass

import wx
import wx.html

from _sci import temporary


@dataclass
class Extension:
	path:Path #directory path
	ReadMe:Path
	Name:str
	Developer:str
	Short_Desc:str
	Version:str
	Enabled:bool
	GUID: str


class frmextensionmngr ( wx.Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, 
			title="Extension Manager",
			style=wx.CAPTION|wx.CLOSE_BOX|wx.MINIMIZE_BOX|wx.RESIZE_BORDER|wx.TAB_TRAVERSAL)

		IconPath = Path(__file__).parent / "icons" / "extensions.png"
		self.SetIcon(wx.Icon(str(IconPath)))
		
		self.m_split = wx.SplitterWindow(self, style= wx.SP_3D|wx.SP_LIVE_UPDATE)
		
		self.m_LWExt = wx.ListView(self.m_split, style=wx.LC_REPORT|wx.LC_SINGLE_SEL)
		self.m_LWExt.SetFont(wx.Font(11, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, "Arial"))
		ColumnHeader = wx.ItemAttr()
		ColumnHeader.SetFont(wx.Font(14, wx.FONTFAMILY_SWISS, wx.FONTSTYLE_NORMAL, wx.FONTWEIGHT_NORMAL, False, "Arial"))
		self.m_LWExt.SetHeaderAttr(ColumnHeader)

		self.m_LWExt.InsertColumn(0, "Name", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(1, "Version", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(2, "Developer", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(3, "Description", wx.LIST_FORMAT_LEFT)
		self.m_LWExt.InsertColumn(4, "Enabled", wx.LIST_FORMAT_LEFT)

		self.m_HTMLWin = wx.html.HtmlWindow(self.m_split)

		self.m_split.SplitHorizontally(self.m_LWExt, self.m_HTMLWin, 0)
		self.m_split.SetSashGravity(0.30)


		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( self.m_split, 1, wx.EXPAND |wx.ALL, 5 )
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )

		self._LoadExtensions()

		self.Bind(wx.EVT_CLOSE, self.__OnClose)
		self.m_LWExt.Bind(wx.EVT_LIST_ITEM_SELECTED, self.__OnListEntrySelected)
	



	def __OnClose(self, event:wx.CloseEvent):
		self.Hide()
		event.Skip()

	
	def __OnListEntrySelected(self, event:wx.ListEvent):
		item = self.m_LWExt.GetNextItem(-1, wx.LIST_NEXT_ALL, wx.LIST_STATE_SELECTED)

		if item<0:
			return
		
		ReadMePath = self._Extensions[item].path / self._Extensions[item].ReadMe

		if Path.exists(ReadMePath):
			self.m_HTMLWin.LoadFile(str(ReadMePath))

		self._SelectedIndex = item
		CurExt = self._Extensions[item]
		
		#if wx.GetKeyState(wx.WXK_ALT):
		menu = wx.Menu()
		ShowItem = wx.MenuItem(menu, id=wx.ID_ANY, text="Show in Explorer...")
		ShowItem.SetBitmap(wx.ArtProvider.GetBitmap(wx.ART_FIND))

		EnableItem = wx.MenuItem(menu, id=wx.ID_ANY, text="Disable" if CurExt.Enabled else "Enable")
		EnableItem.SetBitmap(wx.ArtProvider.GetBitmap(wx.ART_GO_UP))

		menu.Append( ShowItem )
		menu.Append( EnableItem )

		menu.Bind(wx.EVT_MENU, self.__OnShowExplorer, id=ShowItem.GetId())
		menu.Bind(wx.EVT_MENU, self.__OnEnable, id=EnableItem.GetId())

		self.m_LWExt.PopupMenu(menu)

	
	def __OnShowExplorer(self, event:wx.CommandEvent):
		index = self._SelectedIndex
		pth:Path = self._Extensions[index].path	
		subprocess.run(["explorer", "/select,", str(pth)])

	
	def __OnEnable(self, event:wx.CommandEvent):
		index = self._SelectedIndex
		pth:Path = self._Extensions[index].path	
		ParentFolder = pth.parent
		CurName = pth.name
		if CurName[0] == "_":
			TargetName = CurName[1:]
			self._Extensions[index].Enabled = True
		else:
			TargetName = "_" + CurName
			self._Extensions[index].Enabled = False

		os.rename(ParentFolder/CurName, ParentFolder/TargetName)
		self._Extensions[index].path = ParentFolder/TargetName

		TextCol = wx.Colour(0, 0, 0) if self._Extensions[index].Enabled else wx.Colour(255, 0, 0)
		self.m_LWExt.SetItem(index, 4, str(self._Extensions[index].Enabled))		
		self.m_LWExt.SetItemTextColour(index, TextCol)



	def _LoadExtensions(self):
		PyPath =  Path(sys.exec_prefix)
		ExtensionPath = PyPath.parent / "extensions"

		self._Extensions:list[Extension] = []
		for d in os.scandir(str(ExtensionPath)):
			if d.is_dir():
				manifest = Path(d) / "manifest.json"
				f = open(manifest)
				data:dict = json.load(f)
				e = Extension(
					path=manifest.parent,
					ReadMe=data.get("readme"),
					Developer=data.get("developer"),
					Short_Desc=data.get("short_desc"),
					Version=data.get("version"),
					GUID=data.get("guid"),
					Name=data.get("extname"),
					Enabled=d.name[0]!="_")

				self._Extensions.append(e)

		self.WriteExtensions()
		

	
	def WriteExtensions(self):
		self.m_LWExt.DeleteAllItems()
		for index, extension in enumerate(self._Extensions):
			self.m_LWExt.InsertItem(index, extension.Name)
			self.m_LWExt.SetItem(index, 1, extension.Version)
			self.m_LWExt.SetItem(index, 2, extension.Developer)
			self.m_LWExt.SetItem(index, 3, extension.Short_Desc)
			self.m_LWExt.SetItem(index, 4, str(extension.Enabled))

			TextCol = wx.Colour(0, 0, 0) if extension.Enabled else wx.Colour(255, 0, 0)
			
			self.m_LWExt.SetItemTextColour(index, TextCol)
		




if __name__ == "__main__":
	temp:dict = temporary.__dict__["SYS_APPINSTANCES"]
	try:
		if temp.get("frmextensionmngr") == None:
			frm = frmextensionmngr(None)
			temp["frmextensionmngr"] = frm
		else:
			frm = temp["frmextensionmngr"]
		frm.Maximize()
		frm.Show()

		frmWidth = frm.GetClientSize()[0]
		
		Widths = [ 2, 1, 2, 5, 1 ]
		SumW = sum(Widths)
		for i, w in enumerate(Widths):
			frm.m_LWExt.SetColumnWidth(i, int((frmWidth*w)/SumW))
		frm.m_LWExt.Refresh()

	except Exception as e:
		wx.MessageBox(str(e), "Error")