import numbers
import wx

from scisuit.stats import test_t, test_t2_result
import _sci as _se


def _ParseStackedData(var1:list, var2:list)->tuple:
	unique_subs = set(var2)	
	assert len(unique_subs) == 2, "Exactly 2 types of samples expected."
				
	#convert to list for [] access
	unique_list = list(unique_subs)
	
	xdata, ydata = [], []

	j = 0
	for elem in var1:
		if(var2[j] == unique_list[0]):
			xdata.append(elem)
		else:
			ydata.append(elem)
		j += 1
	
	return (xdata, ydata)



def _ParseData(var1:list, var2:list, IsStacked = False)->tuple:
	xdata, ydata = None, None
	if IsStacked == False:
		xdata, ydata = var1, var2
	else:
		xdata, ydata = _ParseStackedData(var1, var2)
	
	xdata = [i for i in xdata if isinstance(i, numbers.Real)]
	ydata = [i for i in ydata if isinstance(i, numbers.Real)]

	return (xdata, ydata)



class frmtestt_2sample ( _se.Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Two-sample t-test")

		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "t_test2sample.png"
		self.SetIcon(wx.Icon(str(IconPath)))	

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar1 = wx.StaticText( self, label = "Variable #1:")
		self.m_txtVar1 = _se.GridTextCtrl( self)

		self.m_stVar2 = wx.StaticText( self, label = "Variable #2:")
		self.m_txtVar2 = _se.GridTextCtrl( self)

		WS = _se.activeworksheet()
		rng:_se.Range = WS.selection()

		if rng != None and rng.ncols() == 2:
			rng1 = rng.subrange(0, 0, -1, 1)
			rng2= rng.subrange(0, 1, -1, 1)
			self.m_txtVar1.SetValue(str(rng1))
			self.m_txtVar2.SetValue(str(rng2))
		
		self.m_stMeanDiff = wx.StaticText( self, label = "Mean difference:")
		self.m_txtMeanDiff = _se.NumTextCtrl( self, wx.ID_ANY, u"0.0")
		
		self.m_stConfLevel = wx.StaticText( self, wx.ID_ANY, u"Confidence Level:")
		self.m_txtConfLevel = _se.NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)

		self.m_stAlternative = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
		self.m_chAlternative = wx.Choice( self, choices = [ u"less than", u"not equal", u"greater than" ])
		self.m_chAlternative.SetSelection( 1 )
		
		self.m_chkStacked = wx.CheckBox( self, wx.ID_ANY, u"Data Stacked")
		self.m_chkEqualVar = wx.CheckBox( self, wx.ID_ANY, u"Equal variances")

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar1, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar1, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stVar2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar2, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stMeanDiff, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtMeanDiff, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConfLevel, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConfLevel, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlternative, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chAlternative, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chkStacked, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chkEqualVar, 0, wx.ALL, 5 )

		sbSzr = wx.StaticBoxSizer( wx.StaticBox( self, wx.ID_ANY, u"Inspect Selected Data" ), wx.HORIZONTAL )
		self.m_BtnBoxPlot = wx.Button( sbSzr.GetStaticBox(), label = u"Box-Whisker Plot" )
		sbSzr.Add( self.m_BtnBoxPlot, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self )
		
		sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = u"Compute" )
		sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = u"Close" )
		sdbSizer.AddButton( self.m_sdbSizerCancel )
		sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( sbSzr, 0, wx.ALL|wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 5 )
		mainSizer.Add( sdbSizer, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
	
		self.m_chkStacked.Bind( wx.EVT_CHECKBOX, self.__OnCheckBox )
		self.m_BtnBoxPlot.Bind(wx.EVT_BUTTON, self.__OnBtnBoxWhiskerPlot)
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelBtn )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKBtn )



	def __del__( self ):
		pass


	
	def __OnCheckBox( self, event ):
		if(event.IsChecked() == True):
			self.m_stVar1.SetLabel("Samples:")
			self.m_stVar2.SetLabel("Subscripts:")
		else:
			self.m_stVar1.SetLabel("Variable #1:")
			self.m_stVar2.SetLabel("Variable #2:")

		event.Skip()

	

	def __OnBtnBoxWhiskerPlot(self, event):
		import scisuit.plot as plt
		
		try:
			assert self.m_txtVar1.GetValue() != "", "Made a selection for (var #1)?"
			assert self.m_txtVar2.GetValue() != "", "Made a selection for (var #2)?"	

			var1 = _se.Range(self.m_txtVar1.GetValue()).tolist()
			var2 = _se.Range(self.m_txtVar2.GetValue()).tolist()

			xdata, ydata = _ParseData(var1, var2, self.m_chkStacked.GetValue())

			plt.boxplot(xdata)
			plt.boxplot(ydata)

			plt.show()
			
		except Exception as e:
			wx.MessageBox(str(e), "Plot Error")
			return



	def __OnCancelBtn( self, event ):
		self.Close()

	
	def __PrintValues(self, Vals:list, WS:_se.Worksheet, Row:int, Col:int):
		pval = Vals[0]
		R:test_t2_result = Vals[1]
		
		ListVals = [
			["Observation", R.n1, R.n2],
			["Mean", R.xaver, R.yaver],
			["Std Deviation", R.s1, R.s2],
			[None, None, None],
			["t-critical", R.tcritical, None],
			["p-value", pval, None]]
		
		if(self.m_chkEqualVar.GetValue()):
			ListVals.insert(3, ["Pooled variance", R.sp, None])
			
		for List in ListVals:
			if(List[0] == None):
				Row += 1
				continue
				
			WS[Row, Col] = List[0] 
			WS[Row, Col+1]=List[1]
			
			if(List[2] != None):
				WS[Row, Col+2] = List[2]
			
			Row += 1
		

		WS[Row + 1, Col] = self.m_txtConfLevel.GetValue() + \
			"% Confidence Interval for " + \
			"(" + str(round(R.CI_lower, 4)) + ", " + str(round(R.CI_upper, 4)) + ")"
		
		


	def __OnOKBtn( self, event ):
		try:
			assert self.m_txtVar1.GetValue() != "", "Selection expected (var #1)"
			assert self.m_txtVar2.GetValue() != "", "Selection expected (var #2)"		
			assert self.m_txtMeanDiff.GetValue() != "", "Value expected for assumed mean difference."
			
			conflevel = float(self.m_txtConfLevel.GetValue())/100
			assert conflevel>0 and conflevel<1, "Confidence interval must be in (0, 100)"

			MeanDiff = float(self.m_txtMeanDiff.GetValue())

			AlterOpt = ["less", "two.sided", "greater"]
			Alternative = AlterOpt[self.m_chAlternative.GetSelection()]

			var1 = _se.Range(self.m_txtVar1.GetValue()).tolist()
			var2 = _se.Range(self.m_txtVar2.GetValue()).tolist()

			xdata, ydata = _ParseData(var1, var2, self.m_chkStacked.GetValue())
			
			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."
			
			EqualVars = self.m_chkEqualVar.GetValue()

			pval, Result = test_t(x=xdata, y=ydata, mu=MeanDiff, 
				varequal = EqualVars, alternative = Alternative, conflevel = conflevel)
				
			self.__PrintValues([pval, Result], WS, row, col)

		except Exception as e:
			wx.MessageBox(str(e))
			return



if __name__ == "__main__":
	frm = frmtestt_2sample(None)
	frm.Show()