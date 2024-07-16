import wx
import numpy as _np

from _sci import Range, NumTextCtrl, GridTextCtrl, \
				activeworksheet, Frame, pnlOutputOptions, parent_path


class frmSampling (Frame):

	def __init__( self, parent ):
		super().__init__ (parent, title = u"Sampling")
		
		ParentPath = parent_path(__file__)
		IconPath = ParentPath / "icons" /  "sampling.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )
		self.SetBackgroundColour( wx.Colour( 255, 192, 130 ) )

		self.m_stSampleSpace = wx.StaticText( self, wx.ID_ANY, u"Sample Space:")
		self.m_stSampleSpace.Wrap( -1 )
		self.m_txtSampleSpace = GridTextCtrl( self)

		WS = activeworksheet()
		Rng = WS.selection()
		if Rng != None:
			self.m_txtSampleSpace.SetValue(str(Rng))

		#input sample space section
		sizerSS = wx.BoxSizer( wx.HORIZONTAL )
		sizerSS.Add( self.m_stSampleSpace, 0, wx.ALL, 5 )
		sizerSS.Add( self.m_txtSampleSpace, 1, wx.ALL, 5 )

		#sample size
		self.m_stSize = wx.StaticText( self, wx.ID_ANY, u"Sample size =")
		self.m_stSize.Wrap( -1 )
		self.m_txtSize = NumTextCtrl(self)	

		self.m_stNSamples = wx.StaticText( self, wx.ID_ANY, u"Number of samples =")
		self.m_stNSamples.Wrap( -1 )
		self.m_txtNSamples = NumTextCtrl(self, val="1")

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.AddGrowableCol(1)
		fgSzr.Add( self.m_stSize, 0, wx.ALL, 5 )
		fgSzr.Add( self.m_txtSize, 1, wx.ALL|wx.EXPAND, 5 )
		fgSzr.Add( self.m_stNSamples, 1, wx.ALL, 5 )
		fgSzr.Add( self.m_txtNSamples, 1, wx.ALL|wx.EXPAND, 5 )

		self.m_chkReplace = wx.CheckBox( self, wx.ID_ANY, u"Sampling with Replacement")
		self.m_chkReplace.SetValue(True)
	
		self.m_pnlOutput = pnlOutputOptions( self )		

		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Compute" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label = "Close" ) 
		m_sdbSizer.AddButton( self.m_sdbSizerCancel ) 
		m_sdbSizer.Realize()

		self.m_stline1 = wx.StaticLine( self)
		self.m_stline2 = wx.StaticLine( self)

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( sizerSS, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_chkReplace, 0, wx.ALL, 5 )
		mainSizer.Add( self.m_stline1, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( fgSzr, 0, wx.EXPAND, 5 )
		mainSizer.Add( self.m_stline2, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( m_sdbSizer, 0, 0, 5 )   
		
		self.SetSizerAndFit( mainSizer )
		self.Layout() 
		self.Centre( wx.BOTH )  

		
		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancelButton )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOKButton )



	def __OnCancelButton( self, event ):
		frm.Close()
	
		

	def __OnOKButton( self, event ):
		try:			
			assert self.m_txtSampleSpace.GetValue() != "", "A data range must be selected"
			SS:list = Range(self.m_txtSampleSpace.GetValue()).tolist()

			assert len(SS)>=3, "Selection must contain at least 3 cells."

			SS = [i for i in SS if isinstance(i, str|float|int)]
			assert len(SS)>=3, "Selected range must contain at least 3 values (str|float|int)."

			assert self.m_txtSize.GetValue()!="", "Sample size cannot be blank."
			assert self.m_txtNSamples.GetValue()!="", "Number of samples cannot be blank."

			SampleSize = int(self.m_txtSize.GetValue())
			NSamples = int(self.m_txtNSamples.GetValue())
			CanReplace:bool = self.m_chkReplace.GetValue()

			if not CanReplace:
				assert (SampleSize*NSamples)<=len(SS), \
					"if replacement=False then total number of samples should be smaller than sample space"
			
			RetVals = []
			for _ in range(NSamples):
				samples = _np.random.choice(SS, SampleSize, replace=CanReplace)
				RetVals.append(samples)

			WS, Row, Col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."

			r, c = Row, Col
			for arr in RetVals:
				for e in arr:
					WS[r, c] = str(e)
					r += 1
				r = Row
				c += 1

		except Exception as e:
			wx.MessageBox(str(e), "Error")
			return



if __name__=="__main__":
	try:
		frm = frmSampling(None)
		sz = frm.GetSize()
		sz.x=400
		frm.SetSize(sz)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error!")