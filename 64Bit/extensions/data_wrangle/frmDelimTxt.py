import wx
import wx.grid


from _sci import activeworksheet, Frame, makeicon, parent_path



class frmDelimText ( Frame ):

	def __init__( self, parent ):
		super().__init__ ( parent, 
			title = u"Text to Columns", 
			style = wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )
		
		IconPath = parent_path(__file__) / "icons" / "txt_to_cols.png"
		self.SetIcon(makeicon(IconPath))

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
		
		#delimiter choices
		self.m_stTxt = wx.StaticText( self, label = "Select Delimiter:")
		self.m_stTxt.Wrap( -1 )

		self.m_Choices = [
			["Colon", "Comma", "Equals sign", "Semicolon", "Space", "Tab"],
			[":", ",", "=", ";", " ", "\t"]]
		self.m_ChcDelims = wx.Choice( self, choices = self.m_Choices[0])
		self.m_ChcDelims.SetSelection( 2 ) #equals sign
		
		szrDelim = wx.BoxSizer( wx.HORIZONTAL )
		szrDelim.Add( self.m_stTxt, 0, wx.ALL, 5 )
		szrDelim.Add( self.m_ChcDelims, 0, wx.ALL, 5 )

		#grid
		self.m_Grid = wx.grid.Grid( self)
		self.m_Grid.CreateGrid( numRows=self.m_Rng.nrows(), numCols=1 )

		self.m_StrList = self._Tokenize(StrList, "=") #2D List
		row = 0
		for Lst in self.m_StrList:
			self._Write(row, 0, Lst)
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
		szrMain.Add( szrDelim, 0, wx.EXPAND, 5 )
		szrMain.Add( self.m_Grid, 1, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( szrBtn, 0, wx.ALIGN_RIGHT, 5 )

		self.Layout()
		self.SetSizer( szrMain )
		self.Centre( wx.BOTH )

		
		self.m_ChcDelims.Bind( wx.EVT_CHOICE, self._OnChcDelimiters )
		self.m_Grid.Bind( wx.grid.EVT_GRID_CELL_RIGHT_CLICK, self.OnGridCellRightDown )
		self.m_BtnOK.Bind( wx.EVT_BUTTON, self.OnOK )
		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.OnCancel )



	def _OnChcDelimiters( self, event ):
		NCols = self.m_Grid.GetNumberCols()
		self.m_Grid.DeleteCols(numCols = NCols - 1)

		index = self.m_ChcDelims.GetSelection()
		Sep = self.m_Choices[1][index]

		StrList = self.m_Rng.tolist()
		self.m_StrList = self._Tokenize(StrList, Sep) #2D List
		row = 0
		for Lst in self.m_StrList:
			self._Write(row, 0, Lst)
			row += 1


	def OnGridCellRightDown( self, event ):
		menu = wx.Menu()
		for name, lst in self.m_IDs.items():
			menu.Append(lst[0], name)
			menu.Bind(wx.EVT_MENU, self._OnMenu)
		self.m_Grid.PopupMenu(menu)
		
		event.Skip()
	

	def _OnMenu(self, event):
		ID = event.GetId()
		Sep =""
		for name, lst in self.m_IDs.items():
			if lst[0] == ID:
				Sep = lst[1]
				break

		i, j = self.m_Grid.GetGridCursorCoords()
		Txt:str = self.m_StrList[i][j]
		Tokens = Txt.split(sep = Sep)

		del self.m_StrList[i][j] #delete element at i,j
		self.m_StrList[i][j:j] = Tokens #insert elements of list (not as list)

		self._Write(i, 0, self.m_StrList[i])


	def OnOK( self, event ):
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


	def OnCancel( self, event ):
		self.Close()

	
	
	def _Write(self, row:int,  col:int, Tokens:list):
		"""
		row, col: starting row and column pos
		Tokens: 1D list containing tokens (words)
		"""
		NTokens = len(Tokens)
		ColsAvailable = self.m_Grid.GetNumberCols() - col
		ColsMissing = NTokens - ColsAvailable
		if ColsMissing>0:
			self.m_Grid.AppendCols(numCols = ColsMissing)
		
		i=0
		for tok in Tokens:
			self.m_Grid.SetCellValue(row, col + i, tok)
			i += 1



	def _Tokenize(self, TxtList:list, Sep:str)->list:
		"""
		TxtList: 1D list containing strings (row by row) to be tokenized
		"""
		RetList = []
		for txt in TxtList:
			Tokens = txt.split(sep = Sep)
			RetList.append(Tokens)
		return RetList




if __name__ == "__main__":
	try:
		frm = frmDelimText(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e))