import wx
import wx.grid
import warnings

from _sci import Workbook, Frame, parent_path, messagebox


class InvalidSelectionWarning(Warning):
	"""When selection contains empty cells"""
	pass


class frmDelimText ( Frame ):

	def __init__( self, parent ):
		super().__init__ ( parent, 
			title = u"Text to Columns", 
			style = wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP )
		
		IconPath = parent_path(__file__) / "icons" / "txt_to_cols.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_IDs = {"Colon": [wx.NewIdRef(), ":"],
			"Comma": [wx.NewIdRef(),","],
			"Equals sign": [wx.NewIdRef(),"="],
			"Semicolon": [wx.NewIdRef(),";"],
			"Space": [wx.NewIdRef()," "],
			 "Tab": [wx.NewIdRef(),"\t"] }
		
		#2D list 
		self.m_List:list[list[str]] = []

		ws = Workbook().activeworksheet()
		self.m_Rng = ws.selection()
		assert self.m_Rng != None, "A selection must be made."
		assert self.m_Rng.ncols() == 1, "Only a single column can be selected."

		StrList = self.m_Rng.tolist() #1D list
		StrList = [str(i) for i in StrList if i!=None] #remove None, convert to str
		assert len(StrList)>0, "Selection does not contain any data"

		if len(StrList) != self.m_Rng.nrows():
			warnmsg = """
			Selection contains cells which does not have data.
			
			At the end of parsing text, the first row of parsed text will be 
			written to selection's first row!! 
			
			Contents might shift...
			"""
			warnings.warn(warnmsg, InvalidSelectionWarning)
		
		#grid
		self.m_Grid = wx.grid.Grid( self)
		self.m_Grid.CreateGrid( numRows=len(StrList), numCols=1 )

		row = 0
		for Lst in StrList:
			if Lst == None:
				continue
			self.m_List.append([Lst])
			self.m_Grid.SetCellValue(row, 0, Lst)
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
		self.m_BtnOK = wx.Button( self, label = "Overwrite as of Selection")
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



	def _WriteToGrid(self):
		NAvailableCols = self.m_Grid.GetNumberCols()

		MaxCols = 0
		for l in self.m_List:
			MaxCols = max(len(l), MaxCols)	


		NCols = NAvailableCols - MaxCols
		if NCols<0:
			self.m_Grid.AppendCols(abs(NCols))
		elif NCols>0:
			self.m_Grid.DeleteCols(numCols=NCols)
		

		self.m_Grid.ClearGrid()

		i, j = 0, 0
		for lst in self.m_List:
			for e in lst:
				self.m_Grid.SetCellValue(i, j, e)
				j += 1
			
			i += 1
			j = 0
		



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
				IsLastCol:bool = (TL_Col == len(self.m_List[i]) -1)

				Tokens = Txt.split(sep = Sep)

				if len(Tokens) == 1:
					continue

				self.m_List[i][TL_Col] = Tokens[0]
				
				if IsLastCol:	
					for j in range(1, len(Tokens)):
						self.m_List[i].append(Tokens[j])
				else:
					del Tokens[0]
					Tokens.reverse()
					for j in range(len(Tokens)): 
						self.m_List[i].insert(TL_Col+1, Tokens[j])
		
		else:
			TL_Row, TL_Col = self.m_Grid.GetGridCursorCoords()
			Txt:str = self.m_Grid.GetCellValue(row=TL_Row, col=TL_Col)
			Tokens = Txt.split(sep = Sep)

			if len(Tokens) == 1:
				return
			
			self.m_List[TL_Row][TL_Col] = Tokens[0]

			IsLastCol:bool = (TL_Col == len(self.m_List[TL_Row]) -1)
				
			if IsLastCol:	
				for j in range(1, len(Tokens)):
					self.m_List[int(TL_Row)].append(Tokens[j])
			else:
				del Tokens[0]
				Tokens.reverse()
				for j in range(len(Tokens)): 
					self.m_List[int(TL_Row)].insert(TL_Col+1, Tokens[j])
		
		self._WriteToGrid()


	def __OnOK( self, event ):
		ws = self.m_Rng.parent()
		TL, BR = self.m_Rng.coords()

		self.m_Rng.clear()

		r, c = TL[0], TL[1]
		for lst in self.m_List:
			for Elem in lst:
				ws[r, c] = str(Elem)
				c += 1
				if c>=ws.ncols():
					ws.appendcols()
			
			#selected row numbers are not affected, therefore no need to append
			r += 1
			c = TL[1]

		self.Close()


	def __OnCancel( self, event ):
		self.Close()






if __name__ == "__main__":
	try:
		with warnings.catch_warnings(record=True) as w:
			frm = frmDelimText(None)
			frm.Show()
			if len(w)>0:
				messagebox(str(w[0].message), str(w[0].category.__name__))
	except Exception as e:
		messagebox(str(e))