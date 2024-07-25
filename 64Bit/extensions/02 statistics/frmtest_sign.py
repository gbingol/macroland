import numbers
import wx

import numpy as np

from scisuit.stats import test_sign, test_sign_Result
import _sci as _se


class frmtest_sign ( _se.Frame ):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Sign Test")

		ParentPath = _se.parent_path(__file__)
		IconPath = ParentPath / "icons" / "test_sign.png"
		self.SetIcon(wx.Icon(str(IconPath)))
	
		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 185, 185, 117 ) )

		self.m_stVar = wx.StaticText( self, label = u"Variable Range:")
		self.m_txtVar = _se.GridTextCtrl( self)

		WS = _se.Workbook().activeworksheet()
		rng:_se.Range = WS.selection()
		if rng != None and rng.ncols() == 1:
			self.m_txtVar.SetValue(str(rng))
		
		self.m_stSample2 = wx.StaticText( self, label = u"Second Sample Range:")
		self.m_stSample2.Enable( False )
		self.m_txtSample2 = _se.GridTextCtrl( self)
		self.m_txtSample2.Enable( False )

		self.m_stMedian = wx.StaticText( self, wx.ID_ANY, u"Test Median:")
		self.m_txtMedian = _se.NumTextCtrl( self, val = u"0.0")
		
		self.m_stConf = wx.StaticText( self, wx.ID_ANY, u"Confidence Level:")
		self.m_txtConf = _se.NumTextCtrl( self, val= u"95", minval=0.0, maxval=100.0)
		
		self.m_stAlt = wx.StaticText( self, wx.ID_ANY, u"Alternative:")
		self.m_chcAlt = wx.Choice( self, choices = [ u"less than", u"not equal", u"greater than" ])
		self.m_chcAlt.SetSelection( 1 )
		
		self.m_chkPaired = wx.CheckBox( self, wx.ID_ANY, u"Paired test")

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( self.m_stVar, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtVar, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stSample2, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtSample2, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stMedian, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtMedian, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stConf, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtConf, 0, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chcAlt, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_chkPaired, 0, wx.ALL, 5 )

		self.m_pnlOutput = _se.pnlOutputOptions( self)
		
		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.ALL|wx.EXPAND, 5 )
		mainSizer.Add( m_sdbSizer, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( mainSizer )
		self.Layout()
		self.Centre( wx.BOTH )
	
		self.m_chkPaired.Bind( wx.EVT_CHECKBOX, self.__chkPaired_OnCheckBox )
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelButtonClick )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKButtonClick )


	
	def __chkPaired_OnCheckBox( self, event ):
		if(event.IsChecked() == True):
			self.m_stVar.SetLabel("First Sample Range:")
		else:
			self.m_stVar.SetLabel("Variable Range:")
		
		self.m_stSample2.Enable(event.IsChecked())
		self.m_txtSample2.Enable(event.IsChecked())
			
		event.Skip()


	def __OnCancelButtonClick( self, event ):
		self.Close()


	def __PrintValues(self, Vals:list, WS:_se.Worksheet, Row:int, Col:int):
		pval = Vals[0]
		Results:test_sign_Result = Vals[1]
		N , NG, NE = Vals[2]
		CompMd = Vals[3] #computed median
		AlterSign = Vals[4]
		
		AsmMd = float(self.m_txtMedian.GetValue()) #assumed median
				
		Lower = Results.lower
		Interp = Results.interpolated
		Upper = Results.upper
		ListVals = [
			["N", N],
			["N>" + str(AsmMd), NG],
			["N=" + str(AsmMd), NE],
			[None],
			["Median", CompMd],
			[None],
			["Median ="+ str(AsmMd) + " vs Median" + str(AlterSign) + str(AsmMd)],
			["p-value", pval],
			[None],
			["CONFIDENCE INTERVALS"],
			["Lower Achieved", Lower.prob, Lower.CILow, Lower.CIHigh],
			["Interpolated", Interp.prob, Interp.CILow, Interp.CIHigh],
			["Interpolated", Upper.prob, Upper.CILow, Upper.CIHigh]]
			
		for List in ListVals:
			if(List[0] == None):
				Row += 1
				continue
				
			for i in range(len(List)):	
				WS[Row, Col+i] = str(List[i]) 
			
			Row += 1
		
		return


	def __OnOKButtonClick( self, event ):
		try:
			assert self.m_txtVar.GetValue() != "", "Selection expected for variable #1."
			
			if self.m_chkPaired.GetValue():
				assert self.m_txtSample2.GetValue != "", "Selection expected for second sample."
				
			Conf = float(self.m_txtConf.GetValue())/100
			AsmdMd = float(self.m_txtMedian.GetValue()) #assumed median

			Sel = self.m_chcAlt.GetSelection()
			Alt = (["less", "two.sided", "greater"])[Sel]
			AltSign = (["<", "!=", ">"])[Sel]
			
			XX, YY, Diff = None, None, None #Diff = XX-YY
			
			X:list = _se.Range(self.m_txtVar.GetValue()).tolist()
			XX = np.asfarray([i for i in X if isinstance(i, numbers.Real)])		
			
			if(self.m_chkPaired.GetValue()):
				Y:list = _se.Range(self.m_txtSample2.GetValue()).tolist()
				YY = np.asfarray([i for i in Y if isinstance(i, numbers.Real)])
				assert len(XX) == len(YY), "Paired test: Variables must be of same size."		
				Diff = XX - YY
			

			pval, Res = test_sign(x = XX, y = YY, md = AsmdMd, alternative = Alt, conflevel = Conf)
			
			N = len(XX) #in case of Arr2, len(Arr1)=len(Arr2)
			NG = len(np.argwhere(Diff > AsmdMd if Diff!=None else XX>AsmdMd))
			NE = len(np.argwhere(Diff == AsmdMd if Diff!=None else XX == AsmdMd))
			
			CompMd = np.median(XX if Diff==None else Diff) #computed median

			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: Selected range is not in correct format or valid."

			self.__PrintValues([pval, Res, (N, NG, NE), CompMd, AltSign], WS, row, col)

		except Exception as e:
			wx.MessageBox(str(e))
			return


if __name__ == "__main__":
	frm = frmtest_sign(None)
	frm.Show()


