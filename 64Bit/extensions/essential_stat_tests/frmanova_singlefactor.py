import wx

import scisuit.stats as stat
import _sci as _se



class frmanova_singlefactor ( _se.Frame ):

	def __init__( self, parent ):
		_se.Frame.__init__ ( self, parent, title = u"One-Way ANOVA")
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "anovasinglefactor.png"
		self.SetIcon(_se.makeicon(IconPath))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_lblResponses = wx.StaticText( self, label = u"Response:")
		self.m_lblResponses.Wrap( -1 )
		self.m_txtResponses = _se.GridTextCtrl( self)
		
		self.m_lblFactors = wx.StaticText( self, label = u"Factors:")
		self.m_lblFactors.Wrap( -1 )
		self.m_lblFactors.Enable( False )
		self.m_txtFactors = _se.GridTextCtrl( self)
		self.m_txtFactors.Enable( False )

		WS = _se.activeworksheet()
		rng = WS.selection()

		if rng != None:
			self.m_txtResponses.SetValue(str(rng))


		self.m_lblConfidence = wx.StaticText( self, label = u"Confidence Level:")
		self.m_lblConfidence.Wrap( -1 )
		self.m_txtConfidence = _se.NumTextCtrl( self, val = u"95", minval=0.0, maxval=100.0)

		self.m_chkStacked = wx.CheckBox( self, label = u"Data is stacked")	
		self.m_chkTukeyTest = wx.CheckBox( self, label = u"Tukey's Test")
		self.m_chkTukeyTest.SetValue(True)

		sbSizer = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, u"Inspect Selected Data" ), wx.HORIZONTAL )
		self.m_BtnBoxPlot = wx.Button( sbSizer.GetStaticBox(), label = u"Box-Whisker Plot" )
		sbSizer.Add( self.m_BtnBoxPlot, 0, wx.ALL, 5 )


		fgSizer = wx.FlexGridSizer( 0, 2, 5, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSizer.Add( self.m_lblResponses, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtResponses, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_lblFactors, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtFactors, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add( self.m_lblConfidence, 0, wx.ALL, 5 )
		fgSizer.Add( self.m_txtConfidence, 0, wx.ALL|wx.EXPAND, 5 )
		fgSizer.Add(self.m_chkStacked, 0, wx.ALL, 5 )
		fgSizer.Add(self.m_chkTukeyTest, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self)	

		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( sbSizer, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( m_sdbSizer, 0, wx.EXPAND, 5 )
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )

		
		self.m_chkStacked.Bind( wx.EVT_CHECKBOX, self.__chkStacked_OnCheckBox )
		self.m_BtnBoxPlot.Bind(wx.EVT_BUTTON, self.__OnBtnBoxWhiskerPlot)
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtnClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtnClick )



	def __OnBtnBoxWhiskerPlot(self, event):
		Responses = self.__GetResponseList()

		#issue the error (__GetResponseList handles it) and return
		if(Responses == None):
			return
		
		import scisuit.plot as plt

		ErrMsg = "Each response must have at least 3 data points"
		try:
			assert len(Responses[0]) >2, ErrMsg

			for i in range(len(Responses)):
				assert len(Responses[i]) >2, ErrMsg
				plt.boxplot(Responses[i])
			
			plt.show()

		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")
			return


	
	def __chkStacked_OnCheckBox( self, event ):
		"""
		If stacked, do NOT perform Tukey test since 
		Pairwise Diff and Difference columns in Tukey test refer to random 
		values in the stacked factors (utterly confusing)
		
		A remedy would be to sort the values in the UniqueList variable but
		the clearer one is to let user to unstack
		"""
		if(event.IsChecked() == True):
			self.m_chkTukeyTest.SetValue(False)
		
		self.m_lblFactors.Enable(event.IsChecked())
		self.m_txtFactors.Enable(event.IsChecked())
		
		self.m_chkTukeyTest.Enable(not event.IsChecked())
		
		event.Skip()



	def __OnCancelBtnClick( self, event ):
		self.Close()
		event.Skip()


	
	def __PrintValues(self, Vals:list, WS:_se.Worksheet, Row:int, Col:int):
		Tukey = None

		pval=Vals[0]
		Dict = Vals[1]
		if(self.m_chkTukeyTest.GetValue()):
			Tukey = Vals[2]

		Headers = [ "Source", "df","SS", "MS","F", "P"]
		for i in range(len(Headers)):
			WS[Row, Col + i] = Headers[i]
		
		Row += 1

		Treatment = Dict["Treatment"]
		Error = Dict["Error"]
		Total = Dict["Total"]

		ListVals = [
			["Treatment", Treatment["DF"], Treatment["SS"] , Treatment["MS"], Dict["Fvalue"], pval],
			["Error", Error["DF"], Error["SS"] , Error["MS"]],
			["Total", Total["DF"], Total["SS"] , Total["MS"]]]
		
		
		for List in ListVals:
			for i in range(len(List)):
				WS[Row, Col+i] = List[i] 
			Row += 1
		
		Row += 1
		
		if(Tukey != None):
			Headers = ["Pairwise Diff", "Difference (i-j)", "Tukey Interval"]
			for i  in range(len(Headers)):
				WS[Row, Col+i] = Headers[i] 
				
			Row += 1
			
			for CompCls in Tukey:
				WS[Row, Col] = str(CompCls.m_a + 1) + "-" + str(CompCls.m_b + 1)
				WS[Row, Col + 1] = str(round(CompCls.m_MeanValueDiff, 2))
				WS[Row, Col + 2] = str(round(CompCls.m_CILow, 2)) + ", " + str(round(CompCls.m_CIHigh, 2))
				
				Row += 1
		
		return



	def __GetResponseList(self)->list:
		IsStacked: bool = self.m_chkStacked.GetValue()

		try:
			assert self.m_txtResponses.GetValue() != "", "A range must be selected for response"	
			if(self.m_chkStacked.GetValue()):
				assert self.m_txtFactors.GetValue(), "Factors range cannot be empty, a selection must be made"

			Responses = [] #2D List

			rngResponses = None
			rngFactors = None #only if stacked

			rngResponses = _se.Range(self.m_txtResponses.GetValue())
			if(IsStacked):
				rngFactors = _se.Range(self.m_txtFactors.GetValue())
				assert rngResponses.ncols() == 1, "Responses must be in a single column"
				assert rngFactors.ncols() == 1, "Factors must be in a single column"


			ResponseList = rngResponses.tolist()

			if(not IsStacked):
				for i in range(rngResponses.ncols()):
					subRng = rngResponses.subrange(row=0, col=i, nrows = -1, ncols = 1)
					Responses.append(subRng.tolist())
			else:	
				ListFactors = rngFactors.tolist()
				FactorsSet = set(ListFactors)
				UniqueFactors = list(FactorsSet)
				
				for i in range(len(UniqueFactors)):
					Responses.append([])
				
				for i in range(len(ListFactors)):
					for j in range(len(UniqueFactors)):
						if(ListFactors[i] == UniqueFactors[j]):
							Responses[j].append(ResponseList[i])
							break
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return
		
		return Responses



	def __OnOKBtnClick( self, event ):
		try:
			TukeyList = None

			assert self.m_txtConfidence.GetValue() != "", "A value must be provided for confidence level"		
			conflevel = float(self.m_txtConfidence.GetValue())/100
			
			Alpha = 1 - conflevel
			assert Alpha>0 or Alpha<1, "Confidence level must be between (0, 100)"

			Responses = self.__GetResponseList()
			if(Responses == None):
				return

			cls = stat.aov(*Responses)
			pvalue, dic = cls.compute()
			
			if(self.m_chkTukeyTest.GetValue()):
				TukeyList = cls.tukey(Alpha)
			
			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."

			self.__PrintValues([pvalue, dic, TukeyList], WS, row, col)

		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return

		event.Skip()


if __name__ == "__main__":
	frm = frmanova_singlefactor(None)
	frm.Show()