import sqlite3 as sql

from scisuit.eng.fpe import Food
import wx
from _sci import Frame, NumTextCtrl, parent_path, CommandWindowDict


class pnlSearch ( wx.Panel ):

	def __init__( self, parent, size = wx.Size( 500,300 ) ):
		super().__init__ (parent, size = size )

		self.m_FirstLDown = True
		self.m_Connection = sql.connect(parent_path(__file__) / "USDANALSR28.db")
		self.m_Food = None	

		self.m_listSearch = wx.ListBox( self, choices=[])	

		self.m_txtSearch = wx.TextCtrl( self, value="Start Typing to Search")
		self.m_txtSearch.SetBackgroundColour( wx.Colour( 192, 192, 192 ) )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_listSearch, 1, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self.m_txtSearch, 0, wx.ALL|wx.EXPAND, 5 )
		self.SetSizerAndFit( szrMain )
		self.Layout()

		self.m_listSearch.Bind( wx.EVT_LISTBOX, self.__List_OnListBox )
		self.m_listSearch.Bind(wx.EVT_RIGHT_UP, self.__List_OnRightUp)
		self.m_txtSearch.Bind( wx.EVT_LEFT_DOWN, self.__txtSearch_OnLeftDown )
		self.m_txtSearch.Bind( wx.EVT_TEXT, self.__txtSearch_OnText )
		
	
	def GetFood(self):
		return self.m_Food

	
	def __List_OnListBox( self, event ):
		try:
			SelText = self.m_listSearch.GetStringSelection()
			QueryStr="SELECT * FROM Composition where FoodName= ?"

			cursor = self.m_Connection.cursor()

			PlaceHolderTxt = SelText
			rows = cursor.execute(QueryStr , (PlaceHolderTxt,)).fetchall() [0]
			
			water = float(rows[2])
			protein = float(rows[3])
			lipid = float(rows[4])
			cho = float(rows[5])
			ash = float(rows[6])
			self.m_Food = Food(water=water, protein = protein, lipid=lipid, cho= cho, ash=ash)

		except Exception as e:
			wx.MessageBox(str(e))
		
		event.Skip()


	def __List_OnRightUp(self, event):
		try:
			assert self.m_FirstLDown==False, "Made a search yet?"
			assert type(self.m_Food)!=type(None), "Made a selection yet?"
		
			varName:str = wx.GetTextFromUser("Enter a valid variable name (conforms to Python)","Variable name")
			if varName!="":
				assert varName.isidentifier(), "Invalid variable name"
				CommandWindowDict[varName] = self.m_Food
				
		except Exception as e:
			wx.MessageBox(str(e))

		event.Skip() 



	def __txtSearch_OnLeftDown( self, event ):
		if self.m_FirstLDown:
			self.m_txtSearch.SetValue("")
			self.m_FirstLDown = False

		event.Skip()



	def __txtSearch_OnText( self, event ):
		Txt=self.m_txtSearch.GetValue()
		Txt=Txt.strip()

		if len(Txt)<2:	return
		
		cursor = self.m_Connection.cursor()

		#clear the list
		self.m_listSearch.Clear()
		
		#split the phrase based on empty character
		words=Txt.split()

		rows = None
		QueryStr = "SELECT * FROM Composition where FoodName like ?"
		if len(words) == 1:
			PlaceHolderTxt = "%" + Txt + "%"
			rows = cursor.execute(QueryStr , (PlaceHolderTxt,)).fetchall() 
		else:
			for _ in range(1, len(words)):
				QueryStr += " INTERSECT SELECT * FROM Composition where FoodName like ?"  
			
			PlaceHolderLst=[]
			for word in  words:
				w="%"+word+"%"
				PlaceHolderLst.append(w)
			
			rows = cursor.execute(QueryStr , PlaceHolderLst).fetchall() 

		for entry in rows:
			self.m_listSearch.Append(str(entry[1])) 

		event.Skip()




class pnlProperties ( wx.Panel ):

	def __init__( self, parent, 
		size = wx.Size( 500,300 ), 
		style = wx.TAB_TRAVERSAL):

		super().__init__ (parent, size = size, style = style)


		self.m_statWater = wx.StaticText( self, wx.ID_ANY, u"Water")
		self.m_statWater.Wrap( -1 )
		self.m_txtWater = wx.TextCtrl( self)
	
		self.m_statCHO = wx.StaticText( self, wx.ID_ANY, u"CHO")
		self.m_statCHO.Wrap( -1 )
		self.m_txtCHO = wx.TextCtrl( self)
		
		self.m_statProtein = wx.StaticText( self, wx.ID_ANY, u"Protein")
		self.m_statProtein.Wrap( -1 )
		self.m_txtProtein = wx.TextCtrl( self)
		
		self.m_statLipid = wx.StaticText( self, wx.ID_ANY, u"Lipid")
		self.m_statLipid.Wrap( -1 )
		self.m_txtLipid = wx.TextCtrl( self)
		
		self.m_statAsh = wx.StaticText( self, wx.ID_ANY, u"Ash")
		self.m_statAsh.Wrap( -1 )
		self.m_txtAsh = wx.TextCtrl( self)
		
		#--------------------------------

		self.m_statRho = wx.StaticText( self, wx.ID_ANY, u"\u03C1")
		self.m_statRho.Wrap( -1 )	
		self.m_txtRho = wx.TextCtrl( self)
		self.m_statRhoUnit = wx.StaticText( self, wx.ID_ANY, u"kg/m3")
		self.m_statRhoUnit.Wrap( -1 )

		self.m_statK = wx.StaticText( self, wx.ID_ANY, u"k")
		self.m_statK.Wrap( -1 )	
		self.m_txtK = wx.TextCtrl( self)
		self.m_statKUnit = wx.StaticText( self, wx.ID_ANY, u"W/mK")
		self.m_statKUnit.Wrap( -1 )

		self.m_statCp = wx.StaticText( self, wx.ID_ANY, u"Cp")
		self.m_statCp.Wrap( -1 )
		self.m_txtCp = wx.TextCtrl( self)
		self.m_statCpUnit = wx.StaticText( self, wx.ID_ANY, u"kJ/kg°C")
		self.m_statCpUnit.Wrap( -1 )

		self.m_statAlpha = wx.StaticText( self, wx.ID_ANY, u"\u03B1")
		self.m_statAlpha.Wrap( -1 )
		self.m_txtAlpha = wx.TextCtrl( self )
		self.m_staticAlphaUnit = wx.StaticText( self, wx.ID_ANY, u"m2/s")
		self.m_staticAlphaUnit.Wrap( -1 )

		#------------------------------------------

		fgSzr1 = wx.FlexGridSizer( 0, 2, 10, 0 )
		fgSzr1.AddGrowableCol( 1 )
		fgSzr1.SetFlexibleDirection( wx.BOTH )
		fgSzr1.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr1.Add( self.m_statWater, 0, wx.ALL, 5 )
		fgSzr1.Add( self.m_txtWater, 1, wx.ALL, 5 )
		fgSzr1.Add( self.m_statCHO, 0, wx.ALL, 5 )
		fgSzr1.Add( self.m_txtCHO, 1, wx.ALL, 5 )
		fgSzr1.Add( self.m_statProtein, 0, wx.ALL, 5 )
		fgSzr1.Add( self.m_txtProtein, 1, wx.ALL, 5 )
		fgSzr1.Add( self.m_statLipid, 0, wx.ALL, 5 )
		fgSzr1.Add( self.m_txtLipid, 1, wx.ALL, 5 )
		fgSzr1.Add( self.m_statAsh, 0, wx.ALL, 5 )
		fgSzr1.Add( self.m_txtAsh, 1, wx.ALL, 5 )

		fgSzr2 = wx.FlexGridSizer( 0, 3, 10, 0 )
		fgSzr2.AddGrowableCol( 1 )
		fgSzr2.SetFlexibleDirection( wx.BOTH )
		fgSzr2.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr2.Add( self.m_statRho, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_txtRho, 1, wx.ALL, 5 )
		fgSzr2.Add( self.m_statRhoUnit, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_statK, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_txtK, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_statKUnit, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_statCp, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_txtCp, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_statCpUnit, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_statAlpha, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_txtAlpha, 0, wx.ALL, 5 )
		fgSzr2.Add( self.m_staticAlphaUnit, 0, wx.ALL, 5 )


		sizerLR = wx.BoxSizer( wx.HORIZONTAL )
		sizerLR.Add( fgSzr1, 1, wx.EXPAND, 5 )
		sizerLR.Add( fgSzr2, 1, wx.EXPAND, 5 )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add(sizerLR, 1, wx.EXPAND, 5 )
		self.SetSizer( szrMain )
		self.Layout()


	def SetWater(self, water:float):
		self.m_txtWater.SetValue(str(water))

	def SetProtein(self, protein:float):
		self.m_txtProtein.SetValue(str(protein))

	def SetLipid(self, lipid:float):
		self.m_txtLipid.SetValue(str(lipid))
	
	def SetCHO(self, CHO:float):
		self.m_txtCHO.SetValue(str(CHO))

	def SetAsh(self, ash:float):
		self.m_txtAsh.SetValue(str(ash))
	
	def SetK(self, k:float):
		self.m_txtK.SetValue(str(k))
	
	def SetRho(self, rho:float):
		self.m_txtRho.SetValue(str(rho))
	
	def SetCp(self, cp:float):
		self.m_txtCp.SetValue(str(cp))

	def SetAlpha(self, alpha:float):
		self.m_txtAlpha.SetValue(str(alpha))





class frmFoodDatabase ( Frame ):
	"""
	1) The compositional data was downloaded from USDA NAL website (given below) as an Excel file.
	https://www.ars.usda.gov/northeast-area/beltsville-md/
	beltsville-human-nutrition-research-center/nutrient-data-laboratory/docs/sr28-download-files/
	
	2) Some of the characters such as & and , was replaced with empty characters for food names.
	3) Acronyms such as W/ and WO/ was replaced with with and without, respectively.
	4) Compositional Data (water, CHO, protein, total lipids, ash) was read from the 
	Excel file into database/USDANALSR28.db SQLite file.
	"""

	def __init__( self, parent ):
		super().__init__ (parent, 
            title = "Search Food Database File - SR 28 (Offline)",  
            style = wx.DEFAULT_FRAME_STYLE|wx.TAB_TRAVERSAL )

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
			
		IconPath = parent_path(__file__) / "icons" / "fooddatabase.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_notebook = wx.Notebook( self, wx.ID_ANY)
		self.m_pnlSearch = pnlSearch( self.m_notebook)
		self.m_pnlProps =pnlProperties( self.m_notebook)
		self.m_notebook.AddPage( self.m_pnlSearch, u"Search", False )
		self.m_notebook.AddPage( self.m_pnlProps, u"Thermo-Physical Props", False )

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( self.m_notebook, 1, wx.EXPAND |wx.ALL, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )

		self.m_notebook.Bind( wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnNotebookPageChanged )
	

	def OnNotebookPageChanged( self, event ):
		selPage = self.m_notebook.GetSelection()
		food = self.m_pnlSearch.GetFood()

		if selPage == 0 or type(food) == type(None):
			return
		
		
		self.m_pnlProps.SetWater(round(food.water, 2)) 
		self.m_pnlProps.SetProtein(round(food.protein, 2))
		self.m_pnlProps.SetLipid(round(food.lipid, 2))
		self.m_pnlProps.SetCHO(round(food.cho, 2))
		self.m_pnlProps.SetAsh(round(food.ash, 2))
		
		self.m_pnlProps.SetRho(round(food.rho(), 2))
		self.m_pnlProps.SetCp(round(food.cp(), 3))
		self.m_pnlProps.SetK(round(food.k(), 3))

		alpha=food.k() / (food.rho()*food.cp())
		self.m_pnlProps.SetAlpha(round(alpha, 5))

		event.Skip()
	


if __name__=="__main__":
	frm=frmFoodDatabase(None) 
	frm.Show()