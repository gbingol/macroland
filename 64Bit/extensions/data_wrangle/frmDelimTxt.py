import wx
import wx.grid


from _sci import activeworksheet, Frame, parent_path



class frmDelimText ( Frame ):

	def __init__( self, parent ):
		super().__init__ ( parent, 
			title = u"Text to Columns", 
			style = wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )
		
		IconPath = parent_path(__file__) / "icons" / "txt_to_cols.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_IDs = {"Colon": [wx.NewIdRef(), ":"],
			"Comma": [wx.NewIdRef(),","],
			"Equals sign": [wx.NewIdRef(),"="],
			"Semicolon": [wx.NewIdRef(),";"],
			"Space": [wx.NewIdRef()," "],
			 "Tab": [wx.NewIdRef(),"\t"] }

		ws = activeworksheet()
		self.m_Rng = ws.selection()
		assert self.m_Rng != None, "A selection must be made."
		assert self.m_Rng.ncols() == 1, "Only a single column can be selected."

		StrList = self.m_Rng.tolist() #1D list
		StrList = [str(i) for i in StrList if i!=None] #remove None, convert to str
		assert len(StrList)>0, "Selection does not contain any data"
		

		#grid
		self.m_Grid = wx.grid.Grid( self)
		self.m_Grid.CreateGrid( numRows=self.m_Rng.nrows(), numCols=1 )

		row = 0
		for Lst in StrList:
			self.m_Grid.SetCellValue(row, 0, str(Lst))
			row += 1

		self.m_Grid.EnableEditing( False )
		self.m_Grid.EnableGridLines( True )
		self.m_Grid.EnableDragGridSize( False )
		self.m_Grid.SetMargins( 0, 0 )

		self.m_Grid.AutoSizeColumns()
		self.m_Grid.EnableDragColMove( False )
		self.m_Grid.EnableDragColSize( True )
		self.m_Grid.SetColLabelSize( 30 )
		self.m_Grid.SetColLabelAlignment( wx.ALIGN_CENTER, wx.ALIGN_CENTER )

		self.m_Grid.EnableDragRowSize( True )
		self.m_Grid.SetRowLabelSize( 80 )
		self.m_Grid.SetRowLabelAlignment( wx.ALIGN_CENTER, wx.ALIGN_CENTER )
		self.m_Grid.SetDefaultCellAlignment( wx.ALIGN_LEFT, wx.ALIGN_TOP )
			

		#buttons
		self.m_BtnOK = wx.Button( self, label = "OK")
		self.m_btnCancel = wx.Button( self, label = "Cancel")	
		szrBtn = wx.BoxSizer( wx.HORIZONTAL )
		szrBtn.Add( self.m_BtnOK, 0, wx.ALL, 5 )
		szrBtn.Add( self.m_btnCancel, 0, wx.ALL, 5 )

		#main szr
		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_Grid, 1, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( szrBtn, 0, wx.ALIGN_RIGHT, 5 )

		self.Layout()
		self.SetSizer( szrMain )
		self.Centre( wx.BOTH )


		self.m_Grid.Bind( wx.grid.EVT_GRID_CELL_RIGHT_CLICK, self.__OnGridCellRightDown )
		self.m_BtnOK.Bind( wx.EVT_BUTTON, self.__OnOK )
		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )



	def _AppendCols(self, Tokens:list, CurColumn:int):
		NCols = self.m_Grid.GetNumberCols()

		NAvailableCols = (NCols-CurColumn)
		NRequiredCols = len(Tokens) - NAvailableCols

		if NRequiredCols>0:
			self.m_Grid.AppendCols(numCols=NRequiredCols)



	def __OnGridCellRightDown( self, event ):
		menu = wx.Menu()
		for name, lst in self.m_IDs.items():
			menu.Append(lst[0], name)
			menu.Bind(wx.EVT_MENU, self.__OnMenu)
		self.m_Grid.PopupMenu(menu)
		
		event.Skip()
	

	def __OnMenu(self, event):
		ID = event.GetId()
		Sep =""
		for _, lst in self.m_IDs.items():
			if lst[0] == ID:
				Sep = lst[1]
				break

		if self.m_Grid.IsSelection():
			TL_Row, TL_Col = self.m_Grid.GetSelectionBlockTopLeft()[0]
			BR_Row, BR_Col = self.m_Grid.GetSelectionBlockBottomRight()[0]

			assert (BR_Col - TL_Col + 1) == 1, "Only a single column can be selected"

			for i in range(TL_Row, BR_Row+1):
				Txt:str = self.m_Grid.GetCellValue(row=i, col=TL_Col)
				Tokens = Txt.split(sep = Sep)
				self._AppendCols(Tokens, TL_Col)

				for j in range(len(Tokens)):
					self.m_Grid.SetCellValue(i, TL_Col+j, Tokens[j])
		
		else:
			TL_Row, TL_Col = self.m_Grid.GetGridCursorCoords()
			Txt:str = self.m_Grid.GetCellValue(row=TL_Row, col=TL_Col)
			Tokens = Txt.split(sep = Sep)
			self._AppendCols(Tokens, TL_Col)

			for j in range(len(Tokens)):
				self.m_Grid.SetCellValue(TL_Row, TL_Col+j, Tokens[j])


	def __OnOK( self, event ):
		ws = self.m_Rng.parent()
		self.m_Rng.clear()
		TL, BR = self.m_Rng.coords()
		i, j = TL[0], TL[1]
		for lst in self.m_StrList:
			for Elem in lst:
				ws[i, j] = str(Elem)
				j += 1
				if j>=ws.ncols():
					ws.appendcols()
			
			#selected row numbers are not affected, therefore no need to append
			i += 1
			j = TL[1]

		self.Close()


	def __OnCancel( self, event ):
		self.Close()






if __name__ == "__main__":
	try:
		frm = frmDelimText(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e))