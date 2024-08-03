import importlib
import wx

from _sci import Workbook, Frame, parent_path, CommandWindowDict



class pnlRange (wx.Panel ):
	def __init__( self, parent):
		super().__init__ (parent)
	
	def get(self, Rng):
		return Rng



class pnlDict ( wx.Panel ):
	def __init__( self, parent):
		super().__init__ (parent)

		self.m_chkHasHeader = wx.CheckBox( self, wx.ID_ANY, u"First Row is Header")
		
		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( self.m_chkHasHeader, 0, wx.ALL|wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
	
	def get(self, Rng):
		return Rng.todict(self.m_chkHasHeader.GetValue())
	



class pnlList ( wx.Panel ):
	def __init__( self, parent):
		super().__init__ (parent)

		stText = wx.StaticText( self, label = "Axis:")
		self.m_rdAxisNone = wx.RadioButton( self, label = "None")
		self.m_rdAxisNone.SetValue( True )
		self.m_rdAxis0 = wx.RadioButton( self, label = "0")
		self.m_rdAxis1 = wx.RadioButton( self, label = "1")
		
		szrOpts = wx.BoxSizer( wx.HORIZONTAL )
		szrOpts.Add( stText, 0, wx.ALL, 5 )
		szrOpts.Add( self.m_rdAxisNone, 0, wx.ALL, 5 )
		szrOpts.Add( self.m_rdAxis0, 0, wx.ALL, 5 )
		szrOpts.Add( self.m_rdAxis1, 0, wx.ALL, 5 )

		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( szrOpts, 0, wx.EXPAND, 5 )

		self.SetSizerAndFit( szrMain )
		self.Layout()
	

	def get(self, Rng):
		if self.m_rdAxisNone.GetValue():
			return Rng.tolist()

		Axis = 0 if self.m_rdAxis0.GetValue() else 1
		return Rng.tolist(Axis)




class frmCreateVar ( Frame ):

	def __init__( self, parent ):
		super().__init__ (parent,
			title="Create Variable", 
			style=wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP)

		IconPath = parent_path(__file__) / "icons" / "py_logo32.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_bookCtrl = wx.Simplebook(self)

		Panels = [
			["Range", pnlRange],
			["list", pnlList],
			["dict", pnlDict]]
		
		for pg in Panels:
			self.m_bookCtrl.AddPage(page=pg[1](self.m_bookCtrl), text=pg[0])
		
		self.m_bookCtrl.ChangeSelection(0)

		"""
		Note that this frame is non-blocking (modeless) to MacroLand framework. 
		Therefore user might change the selection any time after the frame is shown; 
		however we work with the initial one.
		"""
		WS = Workbook().activeworksheet()
		self.m_Range = WS.selection()

		stName = wx.StaticText( self, label= "Variable Name:")
		self._txtName = wx.TextCtrl( self)
		stType = wx.StaticText( self, label="Type:")

		chType = wx.Choice( self, choices = [e[0] for e in Panels])
		chType.SetSelection( 0 )

		fgSzr = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSzr.AddGrowableCol( 1 )
		fgSzr.SetFlexibleDirection( wx.BOTH )
		fgSzr.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		fgSzr.Add( stName, 0, wx.ALL, 5 )
		fgSzr.Add( self._txtName, 1, wx.ALL, 5 )
		fgSzr.Add( stType, 0, wx.ALL, 5 )
		fgSzr.Add( chType, 0, wx.ALL, 5 )
	
		BtnOK = wx.Button( self, label = "OK")	
		BtnCancel = wx.Button( self, label = "Cancel")
	
		szrBtns = wx.BoxSizer( wx.HORIZONTAL )
		szrBtns.Add( BtnOK, 0, wx.ALL, 5 )
		szrBtns.Add( BtnCancel, 0, wx.ALL, 5 )
		
		szrMain = wx.BoxSizer( wx.VERTICAL )
		szrMain.Add( fgSzr, 1, wx.EXPAND |wx.ALL, 5 )
		szrMain.Add( self.m_bookCtrl, 0, wx.EXPAND |wx.ALL, 5 )
		szrMain.Add( szrBtns, 0, wx.EXPAND|wx.ALL, 5 )
		self.Layout()
		self.SetSizerAndFit( szrMain )
		
		self.Centre( wx.BOTH )

		chType.Bind( wx.EVT_CHOICE, self.__OnChoiceType )
		BtnOK.Bind( wx.EVT_BUTTON, self.__OnOK )
		BtnCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )


	def __OnChoiceType( self, event ):
		self.m_bookCtrl.ChangeSelection(event.GetSelection())


	def __OnOK( self, event ):
		try:
			Name = str(self._txtName.GetValue())
			assert Name.isidentifier(), Name + " is not a valid identifier."

			Value = self.m_bookCtrl.GetCurrentPage().get(self.m_Range)
			assert type(Value) != type(None), "Could not create the variable"

			CommandWindowDict[Name] = Value
			self.Close()

		except Exception as e:
			wx.MessageBox(str(e), "Error")
		
		


	def __OnCancel( self, event ):
		self.Close()
		event.Skip()


if __name__=="__main__":
	try:
		frm = frmCreateVar(None)
		frm.Show()
	except Exception as e:
		wx.MessageBox(str(e), "Error")