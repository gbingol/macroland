import wx

import scisuit.stats as stat

from _sci import (Frame, GridTextCtrl, NumTextCtrl, pnlOutputOptions,
				  Workbook, Range, 
				  parent_path, prettify)



class frmanova_singlefactor ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"One-Way ANOVA (Stacked)")
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" / "anovasinglefactor.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_lblFactors = wx.StaticText( self, label = u"Factors:")
		self.m_txtFactors = GridTextCtrl( self)

		self.m_lblResponses = wx.StaticText( self, label = u"Response:")
		self.m_txtResponses = GridTextCtrl( self)

		WS = Workbook().activeworksheet()
		rng = WS.selection()

		if rng != None and rng.ncols() == 2:
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, 1)
			self.m_txtFactors.SetValue(str(rng1))
			self.m_txtResponses.SetValue(str(rng2))

		self.m_lblConfidence = wx.StaticText( self, label = u"Confidence Level:")
		self.m_txtConfidence = NumTextCtrl( self, val = u"95", minval=0.0, maxval=100.0)
	
		self.m_chkTukeyTest = wx.CheckBox( self, label = u"Tukey's Test")
		self.m_chkTukeyTest.SetValue(True)

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, u"Inspect Selected Data" ), wx.HORIZONTAL )
		self.m_BtnBoxPlot = wx.Button( sbSizer.GetStaticBox(), label = u"Box-Whisker Plot" )
		sbSizer.Add( self.m_BtnBoxPlot, 0, wx.ALL, 5 )


		fgSizer = wx.FlexGridSizer( 0, 2, 5, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_lblFactors, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtFactors, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_lblResponses, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtResponses, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_lblConfidence, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtConfidence, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add(self.m_chkTukeyTest, 0, wx.ALL, 5 )

		self.m_pnlOutput = pnlOutputOptions( self)	

		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( sbSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 10 )
		mainSizer.Add( m_sdbSizer, 0, wx.EXPAND, 5 )
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )

		self.m_BtnBoxPlot.Bind(wx.EVT_BUTTON, self.__OnBtnBoxWhiskerPlot)
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )



	def __OnBtnBoxWhiskerPlot(self, event):
				
		import scisuit.plot as plt

		try:
			Responses = self.__GetResponseList()
			if Responses == None:
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



	def __GetResponseList(self)->list:	
		assert self.m_txtResponses.GetValue() != "", "A range must be selected for response"	
		assert self.m_txtFactors.GetValue(), "Factors range cannot be empty, a selection must be made"

		rngResponses = Range(self.m_txtResponses.GetValue())
		rngFactors = Range(self.m_txtFactors.GetValue())
		assert rngResponses.ncols() == 1, "Responses must be in a single column"
		assert rngFactors.ncols() == 1, "Factors must be in a single column"

		ResponseList = rngResponses.tolist()	
		ListFactors = rngFactors.tolist()

		FactorsSet = set(ListFactors)
		UniqueFactors = list(FactorsSet)
		
		Responses:list[list] = [] #2D List
		for i in UniqueFactors:
			Responses.append([])
		
		for i in range(len(ListFactors)):
			for j in range(len(UniqueFactors)):
				if ListFactors[i] == UniqueFactors[j]:
					Responses[j].append(ResponseList[i])
					break
		
		return Responses



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




if __name__ == "__main__":
	try:
		frm = frmanova_singlefactor(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error")