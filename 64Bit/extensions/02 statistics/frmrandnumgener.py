import wx

import scisuit.stats as stat
from _sci import NumTextCtrl, pnlOutputOptions, Frame, parent_path, messagebox



class pnlDist(wx.Panel):
	def __init__(self, parent):
		super().__init__ (parent)
	

	def GenerateRandNumbers(self, NVars, NRandNums):
		raise NotImplementedError("GenerateRandNumbers must be overridden before it can be called.")



class pnlBeta ( pnlDist ):
	def __init__( self, parent):
		super().__init__ ( parent)

		self.m_shape1 = wx.StaticText( self, wx.ID_ANY, u"shape1 (alpha) =")
		self.m_txtShape1 = NumTextCtrl(self)	

		self.m_shape2 = wx.StaticText( self, wx.ID_ANY, u"shape2 (beta) =")
		self.m_txtShape2 = NumTextCtrl(self)

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.AddGrowableCol(1)
		fgSzr.Add( self.m_shape1, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtShape1, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_shape2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtShape2, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizer( fgSzr )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtShape1.GetValue() != "" , "shape1 cannot be blank"
		assert self.m_txtShape2.GetValue() != "" , "shape2 cannot be blank"
		

		shape1 = float(self.m_txtShape1.GetValue())
		shape2 = float(self.m_txtShape1.GetValue())
		
		assert shape1>0 and shape2>0, "shape1 and shape2 >0 expected"

		retList =[]
		for i in range(NVars):
			retList.append(stat.rbeta(n=NRandNums, shape1=shape1, shape2=shape2))

		return retList	



class pnlBinom ( pnlDist ):
	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stPVal = wx.StaticText( self, wx.ID_ANY, u"p-value =")
		self.m_txtPVal = NumTextCtrl( self, minval=0.0, maxval=1.0)	

		self.m_stNTrials = wx.StaticText( self, wx.ID_ANY, u"Number of trials =")
		self.m_txtNTrials = NumTextCtrl( self)

		fgSzrBinom = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrBinom.SetFlexibleDirection( wx.BOTH )
		fgSzrBinom.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrBinom.AddGrowableCol(1)
		fgSzrBinom.Add( self.m_stPVal, 0, wx.ALL, 5 )
		fgSzrBinom.Add( self.m_txtPVal, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzrBinom.Add( self.m_stNTrials, 0, wx.ALL, 5 )
		fgSzrBinom.Add( self.m_txtNTrials, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizer( fgSzrBinom )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		
		assert self.m_txtPVal.GetValue() != "" , "p-value cannot be blank"
		assert self.m_txtNTrials.GetValue()!="", "Number of trials cannot be blank"

		pval = float(self.m_txtPVal.GetValue())
		NTrials = int(self.m_txtNTrials.GetValue())
		assert NTrials>0, "Number of trials must be positive"

		retList =[]
		for i in range(NVars):
			retList.append(stat.rbinom(n=NRandNums, size=NTrials, prob=pval))

		return retList	
		
		


class pnlChisq ( pnlDist ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stDF = wx.StaticText( self, wx.ID_ANY, u"Degrees of freedom =")
		self.m_txtDF = NumTextCtrl( self)

		szrChisq = wx.BoxSizer( wx.HORIZONTAL )
		szrChisq.Add( self.m_stDF, 0, wx.ALL, 5 )
		szrChisq.Add( self.m_txtDF, 1, wx.ALL, 5 )

		self.SetSizer( szrChisq )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):	
		assert self.m_txtDF.GetValue() != "" , "Degrees of freedom cannot be blank"
		
		DF = int(self.m_txtDF.GetValue())
		assert DF>0, "Degrees of freedom must be positive"

		retList =[]
		for i in range(NVars):
			retList.append(stat.rchisq(n=NRandNums, df=DF))

		return retList	
		
		



class pnlFdist ( pnlDist ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stDF1 = wx.StaticText( self, wx.ID_ANY, u"DF1 =")
		self.m_txtDF1 = NumTextCtrl( self)

		self.m_stDF2 = wx.StaticText( self, wx.ID_ANY, u"DF2 =")
		self.m_txtDF2 = NumTextCtrl( self)

		fgSzrFDist = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrFDist.AddGrowableCol( 1 )
		fgSzrFDist.SetFlexibleDirection( wx.BOTH )
		fgSzrFDist.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrFDist.Add( self.m_stDF1, 0, wx.ALL, 5 )
		fgSzrFDist.Add( self.m_txtDF1, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzrFDist.Add( self.m_stDF2, 0, wx.ALL, 5 )
		fgSzrFDist.Add( self.m_txtDF2, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizer( fgSzrFDist )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
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




class pnlGamma( pnlDist ):
	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stShape = wx.StaticText( self, wx.ID_ANY, u"Shape =")
		self.m_txtShape = wx.TextCtrl( self)

		self.m_stScale = wx.StaticText( self, wx.ID_ANY, u"Scale =")
		self.m_txtScale = wx.TextCtrl( self, value="1.0")

		szrFG = wx.FlexGridSizer( 0, 2, 0, 0 )
		szrFG.AddGrowableCol( 1 )
		szrFG.SetFlexibleDirection( wx.BOTH )
		szrFG.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		szrFG.Add( self.m_stShape, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtShape, 0, wx.ALL|wx.EXPAND, 5 )
		szrFG.Add( self.m_stScale, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtScale, 0, wx.ALL|wx.EXPAND, 5 )
		

		self.SetSizer(szrFG)
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtScale.GetValue() != "" , "Scale cannot be blank."
		assert self.m_txtShape.GetValue() != "" , "Shape cannot be blank."
		
		shape = float(self.m_txtShape.GetValue())
		assert shape>0, "shape>0 expected."

		scale = float(self.m_txtScale.GetValue())
		assert scale>0, "shape>0 expected."

		retList =[]
		for i in range(NVars):
			retList.append(stat.rgamma(n=NRandNums, shape=shape, scale=scale))

		return retList
	




class pnlHyperGeom( pnlDist ):
	def __init__( self, parent):
		super().__init__ (parent)

		"""
		m, the number of good samples in the urn.
		n, the number of bad samples in the urn.
		k, the number of samples drawn from the urn
		"""

		self.m_stM = wx.StaticText( self, wx.ID_ANY, u"m (good) =")
		self.m_txtM = wx.TextCtrl( self)

		self.m_stN = wx.StaticText( self, wx.ID_ANY, u"n (bad) =")
		self.m_txtN = wx.TextCtrl( self)

		self.m_stK = wx.StaticText( self, wx.ID_ANY, u"k (sampled) =")
		self.m_txtK = wx.TextCtrl( self)

		szrFG = wx.FlexGridSizer( 0, 2, 0, 0 )
		szrFG.AddGrowableCol( 1 )
		szrFG.SetFlexibleDirection( wx.BOTH )
		szrFG.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		szrFG.Add( self.m_stM, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtM, 0, wx.ALL|wx.EXPAND, 5 )
		szrFG.Add( self.m_stN, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtN, 0, wx.ALL|wx.EXPAND, 5 )
		szrFG.Add( self.m_stK, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtK, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizer(szrFG)
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtM.GetValue() != "" , "m cannot be blank."
		assert self.m_txtN.GetValue() != "" , "n cannot be blank."
		assert self.m_txtK.GetValue() != "" , "k cannot be blank."
		
		m = int(self.m_txtM.GetValue())
		assert m>0, "m>0 expected."

		n = float(self.m_txtN.GetValue())
		assert n>0, "n>0 expected."

		k = float(self.m_txtK.GetValue())
		assert k>0, "k>0 expected."
		assert (m+n)>=k, "k<=(m+n) expected."

		retList =[]
		for i in range(NVars):
			retList.append(stat.rhyper(nn=NRandNums, m=m, n=n, k=k))

		return retList





class pnlNorm ( pnlDist ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stMean = wx.StaticText( self, wx.ID_ANY, u"Mean =")
		self.m_txtMean = NumTextCtrl( self, val="0.0")
		
		self.m_stSD = wx.StaticText( self, wx.ID_ANY, u"Standard Deviation =")
		self.m_txtSD = NumTextCtrl( self, val="1.0")

		fgSzrNorm = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrNorm.AddGrowableCol( 1 )
		fgSzrNorm.SetFlexibleDirection( wx.BOTH )
		fgSzrNorm.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrNorm.Add( self.m_stMean, 0, wx.ALL, 5 )
		fgSzrNorm.Add( self.m_txtMean, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzrNorm.Add( self.m_stSD, 0, wx.ALL, 5 )
		fgSzrNorm.Add( self.m_txtSD, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizer( fgSzrNorm )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtMean.GetValue() != "" , "Mean value cannot be blank"
		assert self.m_txtSD.GetValue() != "" , "Standard deviation cannot be blank"
		
		Mean = float(self.m_txtMean.GetValue())

		SD = float(self.m_txtSD.GetValue())
		assert SD>0, "SD must be positive"

		retList =[]
		for i in range(NVars):
			retList.append(stat.rnorm(n=NRandNums, mean=Mean, sd=SD))

		return retList




class pnlPois ( pnlDist ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stLambda = wx.StaticText( self, wx.ID_ANY, u"Lambda =")
		self.m_txtLambda = NumTextCtrl( self)
		
		szrPois = wx.BoxSizer( wx.HORIZONTAL )
		szrPois.Add( self.m_stLambda, 0, wx.ALL, 5 )
		szrPois.Add( self.m_txtLambda, 1, wx.ALL, 5 )

		self.SetSizer( szrPois )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtLambda.GetValue() != "" , "Lambda cannot be blank"
		
		Lambda = float(self.m_txtLambda.GetValue())
		assert Lambda>0, "Lambda must be positive"

		retList =[]
		for i in range(NVars):
			retList.append(stat.rpois(n=NRandNums, mu=Lambda))

		return retList




class pnlTDist( pnlDist ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stDF = wx.StaticText( self, wx.ID_ANY, u"Degrees of freedom =")
		self.m_txtDF = wx.TextCtrl( self)

		szrTDist = wx.BoxSizer( wx.HORIZONTAL )
		szrTDist.Add( self.m_stDF, 0, wx.ALL, 5 )
		szrTDist.Add( self.m_txtDF, 1, wx.ALL, 5 )

		self.SetSizer( szrTDist )
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtDF.GetValue() != "" , "Degrees of freedom cannot be blank"
		
		DF = int(self.m_txtDF.GetValue())
		assert DF>0, "Degrees of freedom must be positive"

		retList =[]
		for i in range(NVars):
			retList.append(stat.rt(n=NRandNums, df=DF))

		return retList



class pnlWeibull( pnlDist ):

	def __init__( self, parent):
		super().__init__ (parent)

		self.m_stShape = wx.StaticText( self, wx.ID_ANY, u"Shape =")
		self.m_txtShape = wx.TextCtrl( self)

		self.m_stScale = wx.StaticText( self, wx.ID_ANY, u"Scale =")
		self.m_txtScale = wx.TextCtrl( self, value="1.0")

		szrFG = wx.FlexGridSizer( 0, 2, 0, 0 )
		szrFG.AddGrowableCol( 1 )
		szrFG.SetFlexibleDirection( wx.BOTH )
		szrFG.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		szrFG.Add( self.m_stShape, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtShape, 0, wx.ALL|wx.EXPAND, 5 )
		szrFG.Add( self.m_stScale, 0, wx.ALL, 5 )
		szrFG.Add( self.m_txtScale, 0, wx.ALL|wx.EXPAND, 5 )
		

		self.SetSizer(szrFG)
		self.Layout()

	
	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtScale.GetValue() != "" , "Scale cannot be blank."
		assert self.m_txtShape.GetValue() != "" , "Shape cannot be blank."
		
		shape = float(self.m_txtShape.GetValue())
		assert shape>0, "shape>0 expected."

		scale = float(self.m_txtScale.GetValue())
		assert scale>0, "shape>0 expected."

		retList =[]
		for i in range(NVars):
			retList.append(stat.rweibull(n=NRandNums, shape=shape, scale=scale))

		return retList




class pnlUnif ( pnlDist ):

	def __init__( self, parent ):
		super().__init__ (parent )

		self.m_stMin = wx.StaticText( self, wx.ID_ANY, u"Min =")
		self.m_txtMin = NumTextCtrl( self)
		
		self.m_stMax = wx.StaticText( self, wx.ID_ANY, u"Max =")
		self.m_txtMax = NumTextCtrl( self)

		fgSzrUnif = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzrUnif.AddGrowableCol( 1 )
		fgSzrUnif.SetFlexibleDirection( wx.BOTH )
		fgSzrUnif.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzrUnif.Add( self.m_stMin, 0, wx.ALL, 5 )
		fgSzrUnif.Add( self.m_txtMin, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzrUnif.Add( self.m_stMax, 0, wx.ALL, 5 )
		fgSzrUnif.Add( self.m_txtMax, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizer( fgSzrUnif )
		self.Layout()


	def GenerateRandNumbers(self, NVars, NRandNums):
		assert self.m_txtMin.GetValue() != "" , "Min value cannot be blank"
		assert self.m_txtMax.GetValue() != "" , "Max cannot be blank"
		
		varMin = float(self.m_txtMin.GetValue())
		varMax = float(self.m_txtMax.GetValue())
		assert varMax> varMin, "Min must be smaller than Max."

		retList =[]
		for i in range(NVars):
			retList.append(stat.runif(n=NRandNums, min=varMin, max=varMax))

		return retList



class frmRandNumGen (Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Random Number Generation" )
		
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )
		
		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" /  "randomnumgener.jpg"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_Panels = [
			["Beta", pnlBeta],
			["Binomial", pnlBinom], 
			["Chisq", pnlChisq], 
			["F-dist", pnlFdist], 
			["Gamma", pnlGamma],
			["Hypergeometric", pnlHyperGeom],
			["Normal", pnlNorm], 
			["Poisson", pnlPois], 
			["T-dist", pnlTDist], 
			["Weibull", pnlWeibull],
			["Uniform", pnlUnif]]

		self.m_pnlInput = wx.Panel( self, wx.ID_ANY)
		
		self.m_stNVars = wx.StaticText( self.m_pnlInput, label = u"Number of variables =")
		self.m_txtNVars = NumTextCtrl( self.m_pnlInput, val="1" )
		
		self.m_stNRandNums = wx.StaticText( self.m_pnlInput, label = u"Number of random numbers =" )
		self.m_txtNRandNums = NumTextCtrl( self.m_pnlInput, val="10")
		
		self.m_stDist = wx.StaticText( self.m_pnlInput, label = u"Distribution")
		self.m_choiceDist = wx.Choice( self.m_pnlInput, choices = [s[0] for s in self.m_Panels])
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
		
		self.m_pnlDistribution = self.m_Panels[0][1]( self )
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
			assert self.m_txtNRandNums!="", "Number of random variables cannot be blank."
			assert self.m_txtNVars!="", "Number of random numbers cannot be blank."

			NVars = int(self.m_txtNVars.GetValue())
			assert NVars>0, "Number of variables must be greater than 0."

			NRandNums = int(self.m_txtNRandNums.GetValue())
			assert NRandNums>1, "Number of random numbers must be greater than 1."

			tbl = self.m_pnlDistribution.GenerateRandNumbers(NVars, NRandNums)

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."

			self.__PrintValues(tbl, WS, row, col)
		
		except Exception as e:
			messagebox(str(e), "Error")

	

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

		SelPanel = self.m_Panels[event.GetSelection()][1]
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