import wx

import scisuit.stats as stat

from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, 
				  parent_path, prettify)



def ApplyTests(data, AndersonDarling, KolmogorovSmirnov, ShapiroWilkinson):
	Output = []
	if AndersonDarling:
		res = stat.anderson(data)
		Output.append(["Anderson-Darling"])
		Output.append(["p-value", "A2"])
		Output.append([res.pvalue, res.A2])
	
	if KolmogorovSmirnov:
		res = stat.ks_1samp(data)
		if len(Output)>0:
			Output.append([None])

		Output.append(["Kolmogorov-Smirnov"])
		Output.append(["pvalue", "D", "D_loc", "D_sign"])
		Output.append([res.pvalue, res.D, res.D_loc, res.D_sign])
	
	if ShapiroWilkinson:
		res = stat.shapiro(data)
		if len(Output)>0:
			Output.append([None])

		Output.append(["Shapiro-Wilkinson"])
		Output.append(["pvalue", "W"])
		Output.append([res.pvalue, res.W])
	
	return Output



class frmTestNormality ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title ="Test Normality")
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" / "testnorm.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))

		lblData = wx.StaticText( self, label ="Data:")
		self.m_txtData = GridTextCtrl( self)

		self.m_chkApplyCols = wx.CheckBox( self, label="Apply to each column")

		WS = Workbook().activeworksheet()
		rng:Range = WS.selection()

		if rng != None:
			self.m_txtData.SetValue(str(rng))

		self.m_chkAndersonDarling = wx.CheckBox( self, label="Anderson-Darling")
		self.m_chkKolmogorovSmirnov = wx.CheckBox( self, label="Kolmogorov-Smirnov")
		self.m_chkShapiroWilkinson = wx.CheckBox( self, label="Shapiro-Wilkinson")

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, label="Inspect Selected Data" ) )
		BtnBoxPlot = wx.Button( sbSizer.GetStaticBox(), label="Box-Whisker Plot" )
		sbSizer.Add( BtnBoxPlot, 0, wx.ALL, 5 )


		fgSizer = wx.FlexGridSizer( 0, 2, 5, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( lblData, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		fgSizer.Add( self.m_txtData, 0, wx.ALL|wx.EXPAND, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)	

		sdbSzr = wx.StdDialogButtonSizer()
		btnOK = wx.Button( self, wx.ID_OK, label="Compute" )
		sdbSzr.AddButton( btnOK )
		btnCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		sdbSzr.AddButton( btnCancel )
		sdbSzr.Realize()

		line = wx.StaticLine( self)
		
		szrMn = wx.BoxSizer( wx.VERTICAL )
		szrMn.Add( fgSizer, 0, wx.ALL |wx.EXPAND, 5 )
		szrMn.Add( self.m_chkApplyCols, 0, wx.ALL | wx.EXPAND, 5 )
		szrMn.Add( line, 0, wx.ALL |wx.EXPAND, 5 )  
		szrMn.Add( self.m_chkAndersonDarling, 0, wx.ALL |wx.EXPAND, 5)
		szrMn.Add( self.m_chkKolmogorovSmirnov, 0, wx.ALL |wx.EXPAND, 5)
		szrMn.Add( self.m_chkShapiroWilkinson, 0, wx.ALL |wx.EXPAND, 5)
		szrMn.Add( sbSizer, 0, wx.ALL |wx.EXPAND, 5 )
		szrMn.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 10 )
		szrMn.Add( sdbSzr, 0, wx.EXPAND, 5 )
		
		self.SetSizerAndFit( szrMn )
		self.Layout()

		self.Centre( wx.BOTH )

		
		BtnBoxPlot.Bind(wx.EVT_BUTTON, self.__OnPlot)
		btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )
		btnOK.Bind( wx.EVT_BUTTON, self.__OnOK )



	def __OnPlot(self, event):
		import scisuit.plot as plt

		try:
			Responses = self.__GetResponseList()
			if(Responses == None):
				return
			
			for Lst in Responses:
				if len(Lst) <=2:
					continue
				plt.boxplot(Lst)
			
			plt.show()

		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")



	def __OnCancel( self, event ):
		self.Close()
		event.Skip()


	def __OnOK( self, event ):
		try:
			txt = self.m_txtData.GetValue()
			assert txt != "", "Selection expected for Data"

			#Apply columns separately
			ColsSep = self.m_chkApplyCols.GetValue()

			data = Range(txt).tolist(axis= (0 if ColsSep else -1))
			
			Output = []
			_ad = self.m_chkAndersonDarling.GetValue()
			_ks = self.m_chkKolmogorovSmirnov.GetValue()
			_sw = self.m_chkShapiroWilkinson.GetValue()
			
			if not ColsSep:
				data = [elem for elem in data if isinstance(elem, float|int)]
				Output = ApplyTests(data, _ad, _ks, _sw)
			else:
				for i, dt in enumerate(data):
					dt = [elem for elem in dt if isinstance(elem, float|int)]
					Output.append([f"Col {i+1}"])
					Output += [*ApplyTests(dt, _ad, _ks, _sw)]
					
					Output.append([None])
					Output.append([None])
			
			
			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Ouput Options: Selected range is invalid."
			prtfy = self.m_pnlOutput.Prettify()

			# --- Ouput results ---
				
			Row, Col = WS.writelist2d(Output, Row, Col, pretty=prtfy)
		

		except Exception as e:
			wx.MessageBox(str(e), "Error")




if __name__ == "__main__":
	try:
		frm = frmTestNormality(None)
		frm.Show()
		
	except Exception as e:
		wx.MessageBox(str(e), "Error")