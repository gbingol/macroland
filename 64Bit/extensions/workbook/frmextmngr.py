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
	data:str #json data in the form of dictionary
	path:Path


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
			event.Skip()
			return
		
		ReadMePath = self._Extensions[item].path / self._Extensions[item].data["readme"]

		if Path.exists(ReadMePath):
			self.m_HTMLWin.LoadFile(str(ReadMePath))

		self._SelectedIndex = item
		
		#if wx.GetKeyState(wx.WXK_ALT):
		menu = wx.Menu()
		ShowItem = wx.MenuItem(menu, wx.ID_ANY, "Show in Explorer...", wx.EmptyString, wx.ITEM_NORMAL )
		ShowItem.SetBitmap(wx.ArtProvider.GetBitmap(wx.ART_FIND))
		menu.Append( ShowItem )

		menu.Bind(wx.EVT_MENU, self.__OnShowExplorer, id=ShowItem.GetId())

		self.m_LWExt.PopupMenu(menu)

	
	def __OnShowExplorer(self, event:wx.CommandEvent):
		index = self._SelectedIndex
		pth = self._Extensions[index].path	
		subprocess.run(["explorer", "/select,", str(pth)])



	def _LoadExtensions(self):
		PyPath =  Path(sys.exec_prefix)
		ExtensionPath = PyPath.parent / "extensions"

		self._Extensions:list[Extension] = []
		for d in os.scandir(str(ExtensionPath)):
			if d.is_dir():
				manifest = Path(d) / "manifest.json"
				f = open(manifest)
				self._Extensions.append(Extension(data=json.load(f), path=manifest.parent))
		
		for index, extension in enumerate(self._Extensions):
			self.m_LWExt.InsertItem(index, extension.data["extname"])
			self.m_LWExt.SetItem(index, 1, extension.data["version"])
			self.m_LWExt.SetItem(index, 2, extension.data["developer"])
			self.m_LWExt.SetItem(index, 3, extension.data["short_desc"])




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
		
		Widths = [ 2, 1, 2, 5 ]
		SumW = sum(Widths)
		for i, w in enumerate(Widths):
			frm.m_LWExt.SetColumnWidth(i, int((frmWidth*w)/SumW))
		
		frm.Layout()
		frm.Refresh()

	except Exception as e:
		wx.MessageBox(str(e), "Error")