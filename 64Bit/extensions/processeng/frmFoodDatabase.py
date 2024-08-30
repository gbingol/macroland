import sqlite3 as sql

from scisuit.eng.fpe import Food
import wx
from _sci import Frame, NumTextCtrl, parent_path, CommandWindowDict, Framework


class pnlSearch ( wx.Panel ):

	def __init__( self, parent):
		super().__init__ (parent )
		self.m_Parent = parent
		self.m_Connection = sql.connect(parent_path(__file__) / "USDANALSR28.db")
		self.m_Food = None	

		self.m_listSearch = wx.ListBox( self, choices=[])	

		self.m_txtSearch = wx.SearchCtrl( self)
		self.m_txtSearch.SetBackgroundColour( wx.Colour( 235, 239, 143 ) )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_txtSearch, 0, wx.ALL|wx.EXPAND, 5 )
		szrMain.Add( self.m_listSearch, 1, wx.ALL|wx.EXPAND, 5 )
		self.SetSizerAndFit( szrMain )
		self.Layout()

		self.m_listSearch.Bind(wx.EVT_RIGHT_UP, self.__List_OnRightUp)
		self.m_txtSearch.Bind( wx.EVT_TEXT, self.__txtSearch_OnText )
		


	def __List_OnRightUp(self, event):
		menu = wx.Menu()
		ExportItem = wx.MenuItem(menu, wx.ID_ANY, "Export as Python Variable...", wx.EmptyString, wx.ITEM_NORMAL )
		ExportItem.SetBitmap(wx.ArtProvider.GetBitmap(wx.ART_NEW))
		menu.Append( ExportItem )

		CopyClipbrd = wx.MenuItem( menu, wx.ID_ANY, "Copy to Clipboard", wx.EmptyString, wx.ITEM_NORMAL )
		CopyClipbrd.SetBitmap(wx.ArtProvider.GetBitmap(wx.ART_COPY))
		menu.Append( CopyClipbrd )

		menu.Bind( wx.EVT_MENU, self.__ExportAsPyVariable, id = ExportItem.GetId() )
		menu.Bind(wx.EVT_MENU, self.__CopyClipboard, id = CopyClipbrd.GetId() )

		self.m_listSearch.PopupMenu(menu)
		

		event.Skip() 




	def __ExportAsPyVariable(self, event):
		try:
			assert type(self.m_Food)!=type(None), "Made a selection yet?"
			
			Framework().Enable(False)
			varName:str = wx.GetTextFromUser("Enter a valid variable name (conforms to Python)","Variable name")
			Framework().Enable(True)

			if varName!="":
				assert varName.isidentifier(), "Invalid variable name"
				CommandWindowDict[varName] = self.m_Food
				
		except Exception as e:
			wx.MessageBox(str(e))
			



	def __CopyClipboard(self, event):
		try:
			assert type(self.m_Food)!=type(None), "Made a selection yet?"

			s = self.m_listSearch.GetStringSelection()
			s += "\n"
			s += str(self.m_Food)
			
			
			if wx.TheClipboard.Open():
				wx.TheClipboard.SetData(wx.TextDataObject(s))
				wx.TheClipboard.Close()
				wx.TheClipboard.Flush()

		except Exception as e:
			wx.MessageBox(str(e))




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

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_statWater = wx.StaticText( self, label="Water(%)")
		self.m_txtWater = wx.TextCtrl( self, style= wx.TE_READONLY)
	
		self.m_statCHO = wx.StaticText( self, wx.ID_ANY, u"CHO(%)")
		self.m_txtCHO = wx.TextCtrl( self, style= wx.TE_READONLY)
		
		self.m_statProtein = wx.StaticText( self, wx.ID_ANY, u"Protein(%)")
		self.m_txtProtein = wx.TextCtrl( self, style= wx.TE_READONLY)
		
		self.m_statLipid = wx.StaticText( self, wx.ID_ANY, u"Lipid(%)")
		self.m_txtLipid = wx.TextCtrl( self, style= wx.TE_READONLY)
		
		self.m_statAsh = wx.StaticText( self, wx.ID_ANY, u"Ash(%)")
		self.m_txtAsh = wx.TextCtrl( self, style= wx.TE_READONLY)
		
		#--------------------------------

		self.m_statRho = wx.StaticText( self, wx.ID_ANY, u"\u03C1")
		self.m_txtRho = wx.TextCtrl( self, style= wx.TE_READONLY)
		self.m_statRhoUnit = wx.StaticText( self, wx.ID_ANY, u"kg/m3")

		self.m_statK = wx.StaticText( self, wx.ID_ANY, u"k")
		self.m_txtK = wx.TextCtrl( self, style= wx.TE_READONLY)
		self.m_statKUnit = wx.StaticText( self, wx.ID_ANY, u"W/mK")

		self.m_statCp = wx.StaticText( self, wx.ID_ANY, u"Cp")
		self.m_txtCp = wx.TextCtrl( self, style= wx.TE_READONLY)
		self.m_statCpUnit = wx.StaticText( self, wx.ID_ANY, u"kJ/kg°C")

		self.m_statAlpha = wx.StaticText( self, wx.ID_ANY, u"\u03B1")
		self.m_txtAlpha = wx.TextCtrl( self, style= wx.TE_READONLY )
		self.m_staticAlphaUnit = wx.StaticText( self, wx.ID_ANY, u"m2/s")

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
            title = "Search Food Database File - SR 28 (Offline)")

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
			
		IconPath = parent_path(__file__) / "icons" / "fooddatabase.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))

		
		self.m_pnlSearch = pnlSearch( self)
		self.m_pnlProps =pnlProperties( self)
		
		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( self.m_pnlSearch, 1, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( self.m_pnlProps, 1, wx.EXPAND |wx.ALL, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )

		self.m_pnlSearch.m_listSearch.Bind( wx.EVT_LISTBOX, self.__List_OnListBox )

	
	def __List_OnListBox( self, event ):
		try:
			SelText = self.m_pnlSearch.m_listSearch.GetStringSelection()
			QueryStr="SELECT * FROM Composition where FoodName= ?"

			cursor = self.m_pnlSearch.m_Connection.cursor()

			PlaceHolderTxt = SelText
			rows = cursor.execute(QueryStr , (PlaceHolderTxt,)).fetchall() [0]
			
			water = float(rows[2])
			protein = float(rows[3])
			lipid = float(rows[4])
			cho = float(rows[5])
			ash = float(rows[6])

			food = Food(water=water, protein = protein, lipid=lipid, cho= cho, ash=ash)
			self.m_pnlSearch.m_Food = food

			self.m_pnlProps.SetWater(round(water, 2)) 
			self.m_pnlProps.SetProtein(round(protein, 2))
			self.m_pnlProps.SetLipid(round(lipid, 2))
			self.m_pnlProps.SetCHO(round(cho, 2))
			self.m_pnlProps.SetAsh(round(ash, 2))
			
			self.m_pnlProps.SetRho(round(food.rho(), 2))
			self.m_pnlProps.SetCp(round(food.cp(), 3))
			self.m_pnlProps.SetK(round(food.k(), 3))

			alpha=food.k() / (food.rho()*food.cp())
			self.m_pnlProps.SetAlpha(round(alpha, 5))

		except Exception as e:
			wx.MessageBox(str(e))
		
		event.Skip()



def run():
	try:
		frm=frmFoodDatabase(None) 
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e))