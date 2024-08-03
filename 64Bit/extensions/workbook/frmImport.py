import wx
import pathlib
import numpy as np


from _sci import pnlOutputOptions, Frame, parent_path, Framework



class frmImportLoc ( Frame ):
	def __init__( self, parent, FilePath:str ):
		super().__init__ (parent, 
				title="Select Output Location", 
				style = wx.CAPTION | wx.CLOSE_BOX | wx.RESIZE_BORDER | wx.STAY_ON_TOP)

		self.m_Path = FilePath
		
		IconPath = parent_path(__file__) / "icons" / "import.png"
		self.SetIcon(wx.Icon(str(IconPath)))

		self.m_pnlOutput = pnlOutputOptions( self, ShowPrettify=False)	

		btnOK = wx.Button( self, wx.ID_OK, label="Import" )
		btnCancel = wx.Button( self, wx.ID_CANCEL, label="Close" )
		sdbSzr = wx.StdDialogButtonSizer()
		sdbSzr.AddButton( btnOK )
		sdbSzr.AddButton( btnCancel )
		sdbSzr.Realize()

		mainSizer = wx.BoxSizer( wx.VERTICAL )
		mainSizer.Add( self.m_pnlOutput, 0, wx.EXPAND |wx.ALL, 5 )
		mainSizer.Add( sdbSzr, 0, wx.EXPAND, 5 )
		self.SetSizerAndFit( mainSizer )
		self.Layout()

		self.Centre( wx.BOTH )

		btnCancel.Bind( wx.EVT_BUTTON, self.__OnCancel )
		btnOK.Bind( wx.EVT_BUTTON, self.__OnOK )



	def __OnCancel( self, event ):
		self.Close()
		event.Skip()
	


	def __OnOK( self, event:wx.CommandEvent ):
		try:
			WS, row, col = self.m_pnlOutput.Get()
			assert WS != None, "Ouput Options: Selected range is invalid."

			ext = pathlib.Path(self.m_Path).suffix.lower()
			arr = np.loadtxt(self.m_Path, dtype="U", delimiter = "," if ext == ".csv" else "\t")
			
			if arr.ndim == 1:
				WS.writelist(arr.tolist(), row, col)	
			elif arr.ndim ==2:
				WS.writelist2d(arr.tolist(), row, col)
			else:
				raise RuntimeError("Data has more than 2 dimensions.")
			
			self.Close()

		except Exception as e:
			Framework().messagebox(str(e), "Error")



def GetPath()->str:
	wcard = "CSV files (*.csv)|*.csv"
	wcard += "|"
	wcard += "Tab sep text files (*.txt)|*.txt"

	Framework.Enable(False)
	
	dlg = wx.FileDialog(None, style= wx.STAY_ON_TOP, wildcard = wcard)
	dlg.ShowModal()

	Framework.Enable(True)

	return str(dlg.GetPath())



if __name__ == "__main__":
	try:
		path = GetPath()
		if path != "":
			frm = frmImportLoc(None, path)
			frm.Show()

	except Exception as e:
		Framework().messagebox(str(e))