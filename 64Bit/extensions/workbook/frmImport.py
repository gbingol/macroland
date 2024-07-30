import wx
import pathlib
import numpy as np


from _sci import pnlOutputOptions, Frame, parent_path, messagebox



class frmImportLoc ( Frame ):
	def __init__( self, parent, FilePath:str ):
		super().__init__ (parent, 
				title = u"Select Output Loc", 
				style = wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP)

		self.m_Path = FilePath
		
		IconPath = parent_path(__file__) / "icons" / "import.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.SetSizeHints( wx.DefaultSize, wx.DefaultSize )

		self.m_pnlOutput = pnlOutputOptions( self)	

		m_sdbSizer = wx.StdDialogButtonSizer()
		self.m_sdbSizerOK = wx.Button( self, wx.ID_OK, label = "Import" )
		m_sdbSizer.AddButton( self.m_sdbSizerOK )
		self.m_sdbSizerCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		m_sdbSizer.AddButton( self.m_sdbSizerCancel )
		m_sdbSizer.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( m_sdbSizer, 0, wx.EXPAND, 5 )
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )

		self.m_sdbSizerCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )
		self.m_sdbSizerOK.Bind( wx.EVT_BUTTON, self.__OnOK )

	def __OnCancel( self, event ):
		self.Close()
		event.Skip()
	

	def __OnOK( self, event ):
		try:
			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Output Options: The selected range is not in correct format or valid."

			ext = pathlib.Path(self.m_Path).suffix.lower()
			arr = np.loadtxt(self.m_Path, dtype="U", delimiter = "," if ext == ".csv" else "\t")
			assert arr.ndim <=2, "Dimensions of data is suitable for output"
			
			if arr.ndim == 1:
				for e in arr:
					WS[row, col] = str(e)
					row += 1	
			else:
				nr, nc = arr.shape
				for i in range(nr):
					for j in range(nc):
						WS[row + i, col + j] = str(arr[i, j])
			
			self.Close()

		except Exception as e:
			messagebox(str(e), "Error")



def GetPath():
	wcard = "CSV files (*.csv)|*.csv"
	wcard += "|"
	wcard += "Tab sep text files (*.txt)|*.txt"

	#TODO: This will cause crash if user starts another selection
	dlg = wx.FileDialog(None, style= wx.STAY_ON_TOP, wildcard = wcard)
	dlg.ShowModal()
	path = str(dlg.GetPath())


	return path



if __name__ == "__main__":
	try:
		path = GetPath()
		if path != "":
			frm = frmImportLoc(None, path)
			frm.Show()

	except Exception as e:
		messagebox(str(e))