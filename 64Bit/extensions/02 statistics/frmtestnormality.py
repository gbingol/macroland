import wx

import scisuit.stats as stat

from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, 
				  parent_path, prettify)



class frmTestNormality ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title ="Test Normality")
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" / "testnorm.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_lblResponses = wx.StaticText( self, label ="Data:")
		self.m_txtResponses = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng:Range = WS.selection()

		if rng != None:
			self.m_txtResponses.SetValue(str(rng))

		self.m_chkChiSq = wx.CheckBox( self, label="Chi-squared")
		self.m_chkAndersonDarling = wx.CheckBox( self, label="Anderson-Darling")
		self.m_chkKolmogorovSmirnov = wx.CheckBox( self, label="Kolmogorov-Smirnov")
		self.m_chkShapiroWilkinson = wx.CheckBox( self, label="Shapiro-Wilkinson")

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, label="Inspect Selected Data" ) )
		self.m_BtnBoxPlot = wx.Button( sbSizer.GetStaticBox(), label="Box-Whisker Plot" )
		sbSizer.Add( self.m_BtnBoxPlot, 0, wx.ALL, 5 )


		fgSizer = wx.FlexGridSizer( 0, 2, 5, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_lblResponses, 0, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		fgSizer.Add( self.m_txtResponses, 0, wx.ALL|wx.EXPAND, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)	

		sdbSzr = wx.StdDialogButtonSizer()
		self.m_btnOK = wx.Button( self, wx.ID_OK, label="Compute" )
		sdbSzr.AddButton( self.m_btnOK )
		self.m_btnCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		sdbSzr.AddButton( self.m_btnCancel )
		sdbSzr.Realize()

		
		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_chkChiSq, 0, wx.EXPAND, 5)
		mainSizer.Add( ( 0, 10), 1, wx.EXPAND, 5 )
		mainSizer.Add( self.m_chkAndersonDarling, 0, wx.EXPAND, 5)
		mainSizer.Add( ( 0, 10), 1, wx.EXPAND, 5 )
		mainSizer.Add( self.m_chkKolmogorovSmirnov, 0, wx.EXPAND, 5)
		mainSizer.Add( ( 0, 10), 1, wx.EXPAND, 5 )
		mainSizer.Add( self.m_chkShapiroWilkinson, 0, wx.EXPAND, 5)
		mainSizer.Add( ( 0, 10), 1, wx.EXPAND, 5 )
		mainSizer.Add( sbSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 10 )
		mainSizer.Add( sdbSzr, 0, wx.EXPAND, 5 )
		
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )

		
		self.m_BtnBoxPlot.Bind(wx.EVT_BUTTON, self.__OnBtnBoxWhiskerPlot)
		self.m_btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_btnOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )



	def __OnBtnBoxWhiskerPlot(self, event):
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



	def __OnCancelBtnClick( self, event ):
		self.Close()
		event.Skip()



	def __OnOKBtnClick( self, event ):
		try:
			assert self.m_txtConfidence.GetValue() != "", "A value must be provided for confidence level"		
			conflevel = float(self.m_txtConfidence.GetValue())/100
			
			Alpha = 1 - conflevel
			assert Alpha>0 or Alpha<1, "Confidence level must be between (0, 100)"

			Responses = self.__GetResponseList()
			if Responses == None:
				return

			cls = stat.aov(*Responses)
			pvalue, res = cls.compute()
			
			TukeyList = None
			if self.m_chkTukeyTest.GetValue():
				TukeyList = cls.tukey(Alpha)
			
			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Ouput Options: Selected range is invalid."
			prtfy = self.m_pnlOutput.Prettify()

			# --- Ouput results ---

			Headers = [ "Source", "df","SS", "MS","F", "P"]
			WS.writelist(Headers, Row, Col, rowmajor=False)
		
			Row += 1

			ListVals = [
				["Treatment", res.Treat_DF, res.Treat_SS, res.Treat_MS, res.Fvalue, pvalue],
				["Error", res.Error_DF, res.Error_SS, res.Error_MS],
				["Total", res.Total_DF, res.Total_SS , res.Total_MS]]
				
			Row, Col = WS.writelist2d(ListVals, Row, Col, pretty=prtfy)
			
			Row += 1
			
			if(TukeyList != None):
				Headers = ["Pairwise Diff", "(i-j)", "Interval"]
				WS.writelist(Headers, Row, Col, rowmajor=False)
					
				Row += 1
				
				for CompCls in TukeyList:
					WS[Row, Col] = f"{CompCls.m_a + 1} - {CompCls.m_b + 1}"
					WS[Row, Col + 1] = prettify(CompCls.m_MeanValueDiff, prtfy)
					WS[Row, Col + 2] = f"{prettify(CompCls.m_CILow, prtfy)} , {prettify(CompCls.m_CIHigh, prtfy)}"
					
					Row += 1

		except Exception as e:
			wx.MessageBox(str(e), "Error")
	


	def __GetResponseList(self)->list:	
		assert self.m_txtResponses.GetValue() != "", "A range must be selected for response"	
		rngResponses = Range(self.m_txtResponses.GetValue())
		return rngResponses.tolist(axis=0)





if __name__ == "__main__":
	try:
		frm = frmTestNormality(None)
		frm.Show()
		
	except Exception as e:
		wx.MessageBox(str(e), "Error")