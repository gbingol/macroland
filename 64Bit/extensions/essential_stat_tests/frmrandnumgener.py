import wx

import scisuit.stats as stat
import scisuit.util as _util
from _sci import NumTextCtrl, makeicon, pnlOutputOptions, Frame



class pnlDist(wx.Panel):
	def __init__(self, parent):
		wx.Panel.__init__ ( self, parent)
	

	def GenerateRandNumbers(self, NVars, NRandNums):
		raise NotImplementedError("GenerateRandNumbers must be overridden before it can be called.")


class pnlBinom ( pnlDist ):

	def __init__( self, parent):
		pnlDist.__init__ ( self, parent)

		self.m_stPVal = wx.StaticText( self, wx.ID_ANY, u"p-value =")
		self.m_stPVal.Wrap( -1 )
		self.m_txtPVal = NumTextCtrl( self, minval=0.0, maxval=1.0)	

		self.m_stNTrials = wx.StaticText( self, wx.ID_ANY, u"Number of trials =")
		self.m_stNTrials.Wrap( -1 )
		self.m_txtNTrials = NumTextCtrl( self)

		fgSzrBinom = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrBinom.SetFlexibleDirection( wx.BOTH )
		fgSzrBinom.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrBinom.AddGrowableCol(1)
		fgSzrBinom.Add( self.m_stPVal, 0, wx.ALL, 5 )
		fgSzrBinom.Add( self.m_txtPVal, 0, wx.ALL, 5 )
		fgSzrBinom.Add( self.m_stNTrials, 0, wx.ALL, 5 )
		fgSzrBinom.Add( self.m_txtNTrials, 0, wx.ALL, 5 )

		self.SetSizer( fgSzrBinom )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtPVal.GetValue() != "" , "p-value cannot be blank"
			assert self.m_txtNTrials.GetValue()!="", "Number of trials cannot be blank"

			pval = float(self.m_txtPVal.GetValue())
			NTrials = int(self.m_txtNTrials.GetValue())
			assert NTrials>0, "Number of trials must be positive"

			retList =[]
			for i in range(NVars):
				retList.append(stat.rbinom(n=NRandNums, size=NTrials, prob=pval))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return




class pnlChisq ( pnlDist ):

	def __init__( self, parent):
		pnlDist.__init__ ( self, parent)

		self.m_stDF = wx.StaticText( self, wx.ID_ANY, u"Degrees of freedom =")
		self.m_stDF.Wrap( -1 )
		self.m_txtDF = NumTextCtrl( self)

		szrChisq = wx.BoxSizer( wx.HORIZONTAL )
		szrChisq.Add( self.m_stDF, 0, wx.ALL, 5 )
		szrChisq.Add( self.m_txtDF, 0, wx.ALL, 5 )

		self.SetSizer( szrChisq )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtDF.GetValue() != "" , "Degrees of freedom cannot be blank"
			
			DF = int(self.m_txtDF.GetValue())
			assert DF>0, "Degrees of freedom must be positive"

			retList =[]
			for i in range(NVars):
				retList.append(stat.rchisq(n=NRandNums, df=DF))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return



class pnlFdist ( pnlDist ):

	def __init__( self, parent):
		pnlDist.__init__ ( self, parent)

		self.m_stDF1 = wx.StaticText( self, wx.ID_ANY, u"DF1 =")
		self.m_stDF1.Wrap( -1 )
		self.m_txtDF1 = NumTextCtrl( self)

		self.m_stDF2 = wx.StaticText( self, wx.ID_ANY, u"DF2 =")
		self.m_stDF2.Wrap( -1 )
		self.m_txtDF2 = NumTextCtrl( self)

		fgSzrFDist = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrFDist.AddGrowableCol( 1 )
		fgSzrFDist.SetFlexibleDirection( wx.BOTH )
		fgSzrFDist.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrFDist.Add( self.m_stDF1, 0, wx.ALL, 5 )
		fgSzrFDist.Add( self.m_txtDF1, 1, wx.ALL, 5 )
		fgSzrFDist.Add( self.m_stDF2, 0, wx.ALL, 5 )
		fgSzrFDist.Add( self.m_txtDF2, 0, wx.ALL, 5 )

		self.SetSizer( fgSzrFDist )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtDF1.GetValue() != "" , "DF1 cannot be blank"
			assert self.m_txtDF2.GetValue() != "" , "DF2 cannot be blank"
			
			DF1 = int(self.m_txtDF1.GetValue())
			assert DF1>0, "DF1 must be positive"

			DF2 = int(self.m_txtDF2.GetValue())
			assert DF2>0, "DF2 must be positive"

			retList =[]
			for i in range(NVars):
				retList.append(stat.rf(n=NRandNums, df1=DF1, df2=DF2))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return



class pnlNorm ( pnlDist ):

	def __init__( self, parent):
		pnlDist.__init__ ( self, parent)

		self.m_stMean = wx.StaticText( self, wx.ID_ANY, u"Mean =")
		self.m_stMean.Wrap( -1 )
		self.m_txtMean = NumTextCtrl( self, val="0.0")
		
		self.m_stSD = wx.StaticText( self, wx.ID_ANY, u"Standard Deviation =")
		self.m_stSD.Wrap( -1 )
		self.m_txtSD = NumTextCtrl( self, val="1.0")

		fgSzrNorm = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrNorm.AddGrowableCol( 1 )
		fgSzrNorm.SetFlexibleDirection( wx.BOTH )
		fgSzrNorm.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrNorm.Add( self.m_stMean, 0, wx.ALL, 5 )
		fgSzrNorm.Add( self.m_txtMean, 1, wx.ALL, 5 )
		fgSzrNorm.Add( self.m_stSD, 0, wx.ALL, 5 )
		fgSzrNorm.Add( self.m_txtSD, 0, wx.ALL, 5 )

		self.SetSizer( fgSzrNorm )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtMean.GetValue() != "" , "Mean value cannot be blank"
			assert self.m_txtSD.GetValue() != "" , "Standard deviation cannot be blank"
			
			Mean = float(self.m_txtMean.GetValue())

			SD = float(self.m_txtSD.GetValue())
			assert SD>0, "SD must be positive"

			retList =[]
			for i in range(NVars):
				retList.append(stat.rnorm(n=NRandNums, mean=Mean, sd=SD))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return




class pnlPois ( pnlDist ):

	def __init__( self, parent):
		pnlDist.__init__ ( self, parent)

		self.m_stLambda = wx.StaticText( self, wx.ID_ANY, u"Lambda =")
		self.m_stLambda.Wrap( -1 )
		self.m_txtLambda = NumTextCtrl( self)
		
		szrPois = wx.BoxSizer( wx.HORIZONTAL )
		szrPois.Add( self.m_stLambda, 0, wx.ALL, 5 )
		szrPois.Add( self.m_txtLambda, 0, wx.ALL, 5 )

		self.SetSizer( szrPois )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtLambda.GetValue() != "" , "Lambda cannot be blank"
			
			Lambda = float(self.m_txtLambda.GetValue())
			assert Lambda>0, "Lambda must be positive"

			retList =[]
			for i in range(NVars):
				retList.append(stat.rpois(n=NRandNums, mu=Lambda))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return




class pnlTDist( pnlDist ):

	def __init__( self, parent):
		pnlDist.__init__ ( self, parent)

		self.m_stDF = wx.StaticText( self, wx.ID_ANY, u"Degrees of freedom =")
		self.m_stDF.Wrap( -1 )
		self.m_txtDF = wx.TextCtrl( self)

		szrTDist = wx.BoxSizer( wx.HORIZONTAL )
		szrTDist.Add( self.m_stDF, 0, wx.ALL, 5 )
		szrTDist.Add( self.m_txtDF, 0, wx.ALL, 5 )

		self.SetSizer( szrTDist )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtDF.GetValue() != "" , "Degrees of freedom cannot be blank"
			
			DF = int(self.m_txtDF.GetValue())
			assert DF>0, "Degrees of freedom must be positive"

			retList =[]
			for i in range(NVars):
				retList.append(stat.rt(n=NRandNums, df=DF))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return




class pnlUnif ( pnlDist ):

	def __init__( self, parent ):
		pnlDist.__init__ ( self, parent )

		self.m_stMin = wx.StaticText( self, wx.ID_ANY, u"Min =")
		self.m_stMin.Wrap( -1 )
		self.m_txtMin = NumTextCtrl( self)
		
		self.m_stMax = wx.StaticText( self, wx.ID_ANY, u"Max =")
		self.m_stMax.Wrap( -1 )
		self.m_txtMax = NumTextCtrl( self)

		fgSzrUnif = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrUnif.AddGrowableCol( 1 )
		fgSzrUnif.SetFlexibleDirection( wx.BOTH )
		fgSzrUnif.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrUnif.Add( self.m_stMin, 0, wx.ALL, 5 )
		fgSzrUnif.Add( self.m_txtMin, 1, wx.ALL, 5 )
		fgSzrUnif.Add( self.m_stMax, 0, wx.ALL, 5 )
		fgSzrUnif.Add( self.m_txtMax, 0, wx.ALL, 5 )

		self.SetSizer( fgSzrUnif )
		self.Layout()


	def GenerateRandNumbers(self, NVars, NRandNums):
		try:
			assert self.m_txtMin.GetValue() != "" , "Min value cannot be blank"
			assert self.m_txtMax.GetValue() != "" , "Max cannot be blank"
			
			varMin = float(self.m_txtMin.GetValue())
			varMax = float(self.m_txtMax.GetValue())
			assert varMax> varMin, "Min must be smaller than Max."

			retList =[]
			for i in range(NVars):
				retList.append(stat.runif(n=NRandNums, min=varMin, max=varMax))

			return retList	
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return



class frmRandNumGen (Frame ):

	def __init__( self, parent ):
		Frame.__init__ ( self, parent, title = u"Random Number Generation" )
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		ParentPath = _util.parent_path(__file__)
		IconPath = ParentPath / "icons" /  "randomnumgener.jpg"
		self.SetIcon(makeicon(IconPath))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		DistChoices = [ u"Binomial", u"Chisq", u"F-dist", u"Normal", u"Poisson", u"T-dist", u"Uniform" ]
		self.m_Panels = [pnlBinom, pnlChisq, pnlFdist, pnlNorm, pnlPois, pnlTDist, pnlUnif]

		self.m_pnlInput = wx.Panel( self, wx.ID_ANY)
		
		self.m_stNVars = wx.StaticText( self.m_pnlInput, label = u"Number of variables =")
		self.m_stNVars.Wrap( -1 )
		self.m_txtNVars = NumTextCtrl( self.m_pnlInput, val="1" )
		
		self.m_stNRandNums = wx.StaticText( self.m_pnlInput, label = u"Number of random numbers =" )
		self.m_stNRandNums.Wrap( -1 )
		self.m_txtNRandNums = NumTextCtrl( self.m_pnlInput, val="10")
		
		self.m_stDist = wx.StaticText( self.m_pnlInput, label = u"Distribution")
		self.m_stDist.Wrap( -1 )
		self.m_choiceDist = wx.Choice( self.m_pnlInput, choices = DistChoices)
		self.m_choiceDist.SetSelection( 0 )
		
		szrPnlInput = wx.FlexGridSizer( 0, 2, 0, 0 )
		szrPnlInput.SetFlexibleDirection( wx.BOTH )
		szrPnlInput.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		szrPnlInput.AddGrowableCol(1)
		szrPnlInput.Add( self.m_stNVars, 0, wx.ALL, 5 )
		szrPnlInput.Add( self.m_txtNVars, 1, wx.ALL|wx.EXPAND, 5 )
		szrPnlInput.Add( self.m_stNRandNums, 0, wx.ALL, 5 )
		szrPnlInput.Add( self.m_txtNRandNums, 1, wx.ALL|wx.EXPAND, 5 )
		szrPnlInput.Add( self.m_stDist, 0, wx.ALL, 5 )
		szrPnlInput.Add( self.m_choiceDist, 1, wx.ALL|wx.EXPAND, 5 )


		self.m_pnlInput.SetSizer( szrPnlInput )
		self.m_pnlInput.Layout()
		szrPnlInput.Fit( self.m_pnlInput )
		
		self.m_pnlDistribution = pnlBinom( self )
		self.m_pnlOutput = pnlOutputOptions( self)	

		self.m_pnlButtons = wx.Panel(self)
		self.m_BtnGenerate = wx.Button(self.m_pnlButtons, label=u"Generate")
		self.m_BtnClose = wx.Button(self.m_pnlButtons, label = u"Close")
		
		m_BtnSizer = wx.BoxSizer(wx.HORIZONTAL)
		m_BtnSizer.Add( self.m_BtnGenerate, 0, wx.ALL, 5 )
		m_BtnSizer.Add( self.m_BtnClose, 0, wx.ALL, 5 )
		self.m_pnlButtons.SetSizerAndFit( m_BtnSizer )
		self.m_pnlButtons.Layout()
		
		
		self.szrMain = wx.BoxSizer( wx.VERTICAL )
		self.szrMain.Add( self.m_pnlInput, 1, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add( self.m_pnlDistribution, 1, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add( self.m_pnlOutput, 1, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add( self.m_pnlButtons, 0, wx.EXPAND |wx.ALL, 5 )
		self.SetSizerAndFit( self.szrMain )
		self.Layout()

		self.Centre( wx.BOTH )

		self.m_choiceDist.Bind( wx.EVT_CHOICE, self.__OnChoiceDist )
		self.m_BtnGenerate.Bind(wx.EVT_BUTTON, self.__OnGenerate)
		self.m_BtnClose.Bind(wx.EVT_BUTTON, self.__OnClose)

	

	def __OnGenerate(self, event):
		try:
			assert self.m_txtNRandNums!="", "Number of random variables cannot be blank"
			assert self.m_txtNVars!="", "Number of random numbers cannot be blank"

			NVars = int(self.m_txtNVars.GetValue())
			assert NVars>0, "Number of variables must be greater than 0"

			NRandNums = int(self.m_txtNRandNums.GetValue())
			assert NRandNums>1, "Number of random numbers must be greater than 1"

			tbl = self.m_pnlDistribution.GenerateRandNumbers(NVars, NRandNums)

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."

			self.__PrintValues(tbl, WS, row, col)
		
		except Exception as e:
			wx.MessageBox(str(e), "Error")

	


	def __PrintValues(self, tbl, WS, row, col):
		j = 0
		for List in tbl:
			for i in range(len(List)):
				WS[row + i, col + j] = List[i] 
				i += 1
		
			j += 1



	def __OnClose(self, event):
		self.Close()
	
	

	def __OnChoiceDist( self, event ):
		self.szrMain.Detach(self.m_pnlDistribution)
		self.szrMain.Detach(self.m_pnlOutput)
		self.szrMain.Detach(self.m_pnlButtons)

		self.m_pnlDistribution.Destroy()

		SelPanel = self.m_Panels[event.GetSelection()]
		self.m_pnlDistribution = SelPanel(self)
		self.m_pnlDistribution.Layout()
		self.m_pnlDistribution.Refresh()

		self.szrMain.Add( self.m_pnlDistribution, 1, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add(self.m_pnlOutput, 1, wx.EXPAND |wx.ALL, 5 )
		self.szrMain.Add(self.m_pnlButtons, 0, wx.EXPAND |wx.ALL, 5 )
		self.Layout()
		self.Fit()


if __name__ == '__main__':
	frm = frmRandNumGen(None)
	frm.Show()