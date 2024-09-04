import wx
import time

from scisuit.eng import psychrometry
from _sci import Frame, Worksheet, parent_path, NumTextCtrl



class frmPsychrometry ( Frame ):

	def __init__( self, parent):
		super().__init__ (parent, title = u"Psychrometry")

		self.m_Digits = 3 #show results with 3 decimal points
		self.m_NChecks = 0
 
		IconPath = parent_path(__file__) / "icons" / "psycalc.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 0, 242, 242 ) )

		self.m_chkP = wx.CheckBox( self, wx.ID_ANY, "P")
		self.m_chkP.SetToolTip( u"Pressure" )
		self.m_txtP = NumTextCtrl( self)
		self.m_txtP.SetToolTip( u"Pressure" )
		self.m_lblP = wx.StaticText( self, wx.ID_ANY, "kPa")

		self.m_chkTdb = wx.CheckBox( self, wx.ID_ANY, u"Tdb")
		self.m_chkTdb.SetToolTip( u"dry-bulb temperature" )
		self.m_txtTdb = NumTextCtrl( self)
		self.m_lblTdb = wx.StaticText( self, wx.ID_ANY, "°C")

		self.m_chkTwb = wx.CheckBox( self, wx.ID_ANY, u"Twb")
		self.m_chkTwb.SetToolTip( u"wet-bulb temperature" )
		self.m_txtTwb = NumTextCtrl( self)
		self.m_lblTwb = wx.StaticText( self, wx.ID_ANY, "°C")

		self.m_chkTdp = wx.CheckBox( self, wx.ID_ANY, u"Tdp")
		self.m_chkTdp.SetToolTip( u"dew-point temperature" )
		self.m_txtTdp = NumTextCtrl( self)
		self.m_lblTdp = wx.StaticText( self, wx.ID_ANY, "°C")

		self.m_chkW = wx.CheckBox( self, wx.ID_ANY, "W")
		self.m_txtW = NumTextCtrl( self)	
		self.m_lblW = wx.StaticText( self, wx.ID_ANY, "kg/kgda")

		self.m_chkH = wx.CheckBox( self, wx.ID_ANY, "H")
		self.m_txtH =  NumTextCtrl( self)
		self.m_lblH = wx.StaticText( self, wx.ID_ANY, "kJ/kgda")

		self.m_chkRH = wx.CheckBox( self, wx.ID_ANY, "RH")
		self.m_txtRH = NumTextCtrl( self)
		self.m_lblRH = wx.StaticText( self, wx.ID_ANY, "%")

		self.m_chkV = wx.CheckBox( self, wx.ID_ANY, "v")
		self.m_txtV = NumTextCtrl( self)
		self.m_lblV = wx.StaticText( self, wx.ID_ANY, "m3/kg")

		fgSzr_L = wx.FlexGridSizer( 0, 3, 0, 0 )
		fgSzr_L.SetFlexibleDirection( wx.BOTH )
		fgSzr_L.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr_L.Add( self.m_chkP, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtP, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblP, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkTdb, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtTdb, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblTdb, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkTwb, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtTwb, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblTwb, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkTdp, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtTdp, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblTdp, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkW, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtW, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblW, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkH, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtH, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblH, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkRH, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtRH, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblRH, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_chkV, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_txtV, 0, wx.ALL, 5 )
		fgSzr_L.Add( self.m_lblV, 0, wx.ALL, 5 )


		#Collapsible pane

		self.m_colPane = wx.CollapsiblePane( self, label="More Info")
		self.m_stxtPw = wx.StaticText( self.m_colPane.GetPane(), wx.ID_ANY, "Pw")
		self.m_txtPw = wx.TextCtrl( self.m_colPane.GetPane(), style= wx.TE_READONLY )
		self.m_lblPw = wx.StaticText( self.m_colPane.GetPane(), wx.ID_ANY, "kPa")

		self.m_stxtPws = wx.StaticText( self.m_colPane.GetPane(), wx.ID_ANY, u"Pws")
		self.m_txtPws = wx.TextCtrl( self.m_colPane.GetPane(), style= wx.TE_READONLY )
		self.m_lblPws = wx.StaticText( self.m_colPane.GetPane(), wx.ID_ANY, "kPa")

		self.m_stxtWs = wx.StaticText( self.m_colPane.GetPane(), wx.ID_ANY, "Ws")
		self.m_txtWs = wx.TextCtrl( self.m_colPane.GetPane(),style = wx.TE_READONLY )
		self.m_lblWs = wx.StaticText( self.m_colPane.GetPane(), wx.ID_ANY, "kg/kgda")
		
		fgSzr = wx.FlexGridSizer( 0, 3, 0, 0 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stxtPw, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtPw, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_lblPw, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_stxtPws, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtPws, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_lblPws, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_stxtWs, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtWs, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_lblWs, 0, wx.ALL, 5 )

		self.m_colPane.GetPane().SetSizer( fgSzr )
		self.m_colPane.GetPane().Layout()


		#Buttons

		self.m_btnCalc = wx.Button( self, wx.ID_ANY, "Compute")
		self.m_btnCalc.Enabled=False

		self.m_btnShowGraph = wx.Button( self, wx.ID_ANY, "Show Me")
		self.m_btnShowGraph.Enabled=False

		szrButtons = wx.BoxSizer( wx.HORIZONTAL )
		szrButtons.Add(self.m_btnCalc, 1, wx.EXPAND, 5 )
		szrButtons.Add(( 10, 0), 0, wx.EXPAND, 5)
		szrButtons.Add(self.m_btnShowGraph, 1, wx.EXPAND, 5 )
		

		#Menus
		
		self.m_menuFile = wx.Menu()
		menuExport = wx.MenuItem(self.m_menuFile, -1, "Export to Worksheet")
		self.m_menuFile.Append(menuExport)

		self.m_menuFile.Bind( wx.EVT_MENU, self._Export, id = menuExport.GetId() )
		
		self.m_menuDigits = wx.Menu()	
		self.m_Item2Digs = wx.MenuItem( self.m_menuDigits, -1, "2 Digits","", wx.ITEM_RADIO)
		self.m_menuDigits.Append( self.m_Item2Digs )

		self.m_Item3Digs = wx.MenuItem( self.m_menuDigits, -1, "3 Digits", "", wx.ITEM_RADIO )
		self.m_menuDigits.Append( self.m_Item3Digs )
		self.m_Item3Digs.Check(True)

		self.m_Item4Digs = wx.MenuItem( self.m_menuDigits, -1, "4 Digits", "", wx.ITEM_RADIO )
		self.m_menuDigits.Append( self.m_Item4Digs )

		self.m_menubar = wx.MenuBar( 0 )
		self.m_menubar.Append( self.m_menuFile, "File" )
		self.m_menubar.Append( self.m_menuDigits, "Digits" )
		self.SetMenuBar( self.m_menubar )


		#----  Main sizer -----
		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add(fgSzr_L, 1, wx.EXPAND, 5 )
		mainSizer.Add(self.m_colPane, 0, wx.EXPAND, 5 )
		mainSizer.Add(( 0, 20), 0, wx.EXPAND, 5 )
		mainSizer.Add(szrButtons, 0, wx.ALL|wx.ALIGN_CENTER_HORIZONTAL, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )

		#----- Events -----
		self.Bind( wx.EVT_CHECKBOX, self.__OnCheckBox )
		self.m_btnCalc.Bind( wx.EVT_BUTTON, self.__OnBtnCompute )
		self.m_btnShowGraph.Bind( wx.EVT_BUTTON, self.__OnShowGraph )
		
		self.Bind( wx.EVT_MENU, self.__OnMenuDigits, id = self.m_Item2Digs.GetId() )
		self.Bind( wx.EVT_MENU, self.__OnMenuDigits, id = self.m_Item3Digs.GetId() )
		self.Bind( wx.EVT_MENU, self.__OnMenuDigits, id = self.m_Item4Digs.GetId() )
		
		self.m_Controls=[
			[self.m_chkP, self.m_txtP, "kPa", "P"],
			[self.m_chkTdb, self.m_txtTdb, "°C", "Tdb"],
			[self.m_chkTwb, self.m_txtTwb, "°C", "Twb"],
			[self.m_chkTdp, self.m_txtTdp, "°C", "Tdp"],
			[self.m_chkW, self.m_txtW, "kg/kg da", "W"],
			[self.m_chkH, self.m_txtH, "kJ/kgda", "H"],
			[self.m_chkRH, self.m_txtRH, "%", "RH"],
			[self.m_chkV, self.m_txtV, "m3/kg", "V"],
			[None, self.m_txtPw, "kPa", "Pw"],
			[None, self.m_txtPws, "kPa", "Pws"],
			[None, self.m_txtWs, "kg/kgda", "Ws"]
		]

		self.m_CheckBoxes:list[wx.CheckBox] = [e[0] for e in self.m_Controls if e[0]!=None]

	
    
	def __OnCheckBox( self, event:wx.CommandEvent ):
		obj:wx.Object = event.GetEventObject()
		if event.IsChecked():
			adder = 1
			self.m_CheckBoxes.remove(obj)
		else:
			adder = -1
			self.m_CheckBoxes.append(obj)
		
		self.m_NChecks += adder

		for chkBox in self.m_CheckBoxes:
			chkBox.Enable(self.m_NChecks < 3)
		
		self.m_btnCalc.Enabled = (self.m_NChecks >= 3)
				
	
	
	def __OnMenuDigits( self, event:wx.CommandEvent ): 
		id = event.GetId()
		
		if id == self.m_Item2Digs.GetId():
			self.m_Digits=2
		elif id == self.m_Item3Digs.GetId():
			self.m_Digits=3
		else:
			self.m_Digits=4



	def __OnBtnCompute( self, event ):
		try:
			PsyParams = dict()

			#Collect the check entries and values in a dictionary
			for Entry in self.m_Controls:
				if Entry[0] and Entry[0].GetValue():
					assert Entry[1].GetValue() !="", "A numeric value must be entered for " + Entry[3]  

					#note that we use NumTextCtrl so this will always succeed
					PsyParams[Entry[3]] = float(Entry[1].GetValue()) 
		
		
			result = psychrometry(**PsyParams)

			for Entry in self.m_Controls:
				value = getattr(result, Entry[3])
				value = round(value, self.m_Digits)
				Entry[1].SetValue(str(value))
			
			if 10<=result.Tdb<=90:
				self.m_btnShowGraph.Enable()

		except Exception as e:
			wx.MessageBox(str(e))

		


	def __OnShowGraph( self, event ):
	
		try:
			P = float(self.m_txtP.GetValue())
			Tdb = float(self.m_txtTdb.GetValue())
			W = float(self.m_txtW.GetValue())

			#Instead of using theoretical limits we are using the practical ones
			assert 70<P<120, "P [70, 120] kPa expected."
			assert 0<W<1, "Absolute humidity (0, 1) expected."
			assert -0<Tdb<90, "Tdb [0, 90]"

			import scisuit.plot as plt
			import scisuit.plot.gdi as gdi

			plt.psychrometry(P=P*1000)
			gdi.marker(xy=(Tdb, W), size=5, fc="#000000")

			"""
			the code pauses at plt.show(), therefore
			we disable the button right before showing the graph
			"""
			self.m_btnShowGraph.Enable(False)

			plt.show()

		except Exception as e:
			wx.MessageBox(str(e))

	

	def _Export(self, evt):
		inputs, outputs = [], []

		for Entry in self.m_Controls:
			lst = [Entry[3], Entry[1].GetValue(), Entry[2]]
			if Entry[0] in self.m_CheckBoxes or Entry[0] == None:
				outputs.append(lst)
			else:
				inputs.append(lst)	
		
		t = time.localtime()
		wsname = f"{t.tm_mon}{t.tm_mday} {t.tm_hour}{t.tm_min}{t.tm_sec}"

		ws = Worksheet(wsname)
			
		ws.writelist2d(inputs, 0, 0)
		ws.writelist2d(outputs, len(inputs) + 1, 0)



if __name__ == "__main__":
	frm = frmPsychrometry(None)
	frm.Show()