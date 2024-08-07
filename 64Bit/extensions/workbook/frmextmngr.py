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
		
		self._SelectedIndex = item
		CurExt = self._Extensions[item]

		#Get readme path (if exists) and show	
		ReadMe = CurExt.path / CurExt.ReadMe
		if Path.exists(ReadMe):
			self.m_HTMLWin.LoadFile(str(ReadMe))
		
		#prepare and show popup menu
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
		pth = self._Extensions[self._SelectedIndex].path	
		subprocess.run(["explorer", "/select,", str(pth)])

	
	def __OnEnable(self, event:wx.CommandEvent):
		try:
			row = self._SelectedIndex
			CurExtension = self._Extensions[row]

			pth:Path = CurExtension.path	
			ParentFolder = pth.parent

			#Current name (note that name will change)
			CurName = pth.name

			if CurName[0] == "_":
				TargetName = CurName[1:] #remove _ from folder name
			else:
				TargetName = "_" + CurName

			os.rename(ParentFolder/CurName, ParentFolder/TargetName)
			CurExtension.path = ParentFolder/TargetName
			CurExtension.Enabled = CurExtension.path.name[0] != "_"

			TextCol = wx.Colour(0, 0, 0) if CurExtension.Enabled else wx.Colour(255, 0, 0)
			self.m_LWExt.SetItem(row, 4, str(CurExtension.Enabled))		
			self.m_LWExt.SetItemTextColour(row, TextCol)
		
		except Exception as e:
			wx.MessageBox(str(e))



	def _LoadExtensions(self):
		PyPath =  Path(sys.exec_prefix)
		ExtensionsFolder = PyPath.parent / "extensions"

		self._Extensions:list[Extension] = []
		for d in os.scandir(str(ExtensionsFolder)):
			if not d.is_dir():
				continue

			manifest = Path(d) / "manifest.json"
			f = open(manifest)
			data:dict = json.load(f)
			e = Extension(
				Name = data.get("extname"),
				path = manifest.parent,
				ReadMe = data.get("readme"),
				Developer = data.get("developer"),
				Short_Desc = data.get("short_desc"),
				Version = data.get("version"),
				GUID = data.get("guid"),
				Enabled = d.name[0]!="_")

			self._Extensions.append(e)

		for i, ext in enumerate(self._Extensions):
			self.m_LWExt.InsertItem(i, ext.Name)
			self.m_LWExt.SetItem(i, 1, ext.Version)
			self.m_LWExt.SetItem(i, 2, ext.Developer)
			self.m_LWExt.SetItem(i, 3, ext.Short_Desc)
			self.m_LWExt.SetItem(i, 4, str(ext.Enabled))

			#red if disabled, black otherwise
			TextCol = wx.Colour(0 if ext.Enabled else 255, 0, 0)	
			self.m_LWExt.SetItemTextColour(i, TextCol)




if __name__ == "__main__":
	temp:dict = temporary.__dict__["SYS_APPINSTANCES"]
	try:
		#Show only a single instance
		if temp.get("frmextensionmngr") == None:
			frm = frmextensionmngr(None)
			temp["frmextensionmngr"] = frm
		else:
			frm = temp["frmextensionmngr"]
		frm.Maximize()
		frm.Show()

		#Re-adjust column sizes
		frmWidth = frm.GetClientSize()[0]
		
		Widths = [ 2, 1, 2, 5, 1 ]
		SumW = sum(Widths)
		for i, w in enumerate(Widths):
			frm.m_LWExt.SetColumnWidth(i, int((frmWidth*w)/SumW))
		frm.m_LWExt.Refresh()

	except Exception as e:
		wx.MessageBox(str(e), "Error")