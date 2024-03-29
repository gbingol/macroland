import wx
from . import GridTextCtrl, Range, Worksheet


__all__ = ['pnlOutputOptions']


class pnlOutputOptions ( wx.Panel ):

	def __init__( self, parent, bgcolor = None):
		wx.Panel.__init__ ( self, parent)

		#imitate parent's background color to blend well
		if(bgcolor == None):
			self.SetBackgroundColour(parent.GetBackgroundColour())
		else:
			if(isinstance(bgcolor, wx.Colour) == False):
				raise TypeError("bgcolor must be of type wx.Colour")
			self.SetBackgroundColour(bgcolor)

		mainSizer = wx.BoxSizer( wx.VERTICAL )

		#header section
		sizerHeader = wx.BoxSizer( wx.HORIZONTAL )
		self.m_staticText = wx.StaticText( self, wx.ID_ANY, u"Ouput Options" )
		self.m_staticText.Wrap( -1 )

		sizerHeader.Add( self.m_staticText, 0, wx.ALL, 5 )
		self.m_staticlineHeader = wx.StaticLine( self )
		sizerHeader.Add( self.m_staticlineHeader, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )

		mainSizer.Add( sizerHeader, 0, wx.EXPAND, 5 )


		#selection section
		fgSizer = wx.FlexGridSizer( 0, 2, 10, 0 )
		fgSizer.AddGrowableCol( 1 )
		fgSizer.SetFlexibleDirection( wx.BOTH )
		fgSizer.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )

		self.m_radioSelection = wx.RadioButton( self, wx.ID_ANY, u"Selection")
		fgSizer.Add( self.m_radioSelection, 0, wx.ALL, 5 )

		self.m_txtSelRange = GridTextCtrl( self)
		self.m_txtSelRange.Enable(False) #radiobox is selected, so disable it
		fgSizer.Add( self.m_txtSelRange, 0, wx.ALL|wx.EXPAND, 5 )

		self.m_radioNewWS = wx.RadioButton( self, wx.ID_ANY, u"New Sheet")
		self.m_radioNewWS.SetValue( True ) #by default new worksheet
		fgSizer.Add( self.m_radioNewWS, 0, wx.ALL, 5 )

		mainSizer.Add( fgSizer, 0, wx.EXPAND, 5 )


		#footer section
		sizerFooter = wx.BoxSizer( wx.HORIZONTAL )
		self.m_staticlineFooter = wx.StaticLine( self)
		sizerFooter.Add( self.m_staticlineFooter, 1, wx.ALL|wx.ALIGN_CENTER_VERTICAL, 5 )
		mainSizer.Add( sizerFooter, 0, wx.EXPAND, 5 )


		self.SetSizerAndFit( mainSizer )
		self.Layout()


		self.m_radioSelection.Bind( wx.EVT_RADIOBUTTON, self.radioSelection_OnRadioBtn )
		self.m_radioNewWS.Bind( wx.EVT_RADIOBUTTON, self.radioNewWS_OnRadioBtn )


	def __del__( self ):
		pass


	def radioSelection_OnRadioBtn( self, event ):
		self.m_txtSelRange.Enable()
		event.Skip()


	def radioNewWS_OnRadioBtn( self, event ):
		self.m_txtSelRange.Enable(False)
		event.Skip()
	


	def __GetSelRange(self):
		if(self.m_radioSelection.GetValue() == False or self.m_txtSelRange.GetValue() == ""):
			return None

		try:
			rng = Range(self.m_txtSelRange.GetValue())
			return rng
		except Exception as e:
			return None

	

	def Get(self)->tuple:
		"""
		Returns worksheet (new or where range is), row and col indexes
		If there is a selection and selected range is not valid returns: None, -1, -1 
		If new worksheet returns: WS, 0, 0
		"""
		WS = None
		row, col = -1, -1
		
		if(self.m_radioNewWS.GetValue() == True): #new worksheet
			WS = Worksheet()
			row, col = 0, 0
		else:
			SelRange = self.__GetSelRange()

			if(SelRange != None):
				WS = SelRange.parent()
				row, col = SelRange.coords()[0] #[0]:top-left
		
		return WS, row, col

