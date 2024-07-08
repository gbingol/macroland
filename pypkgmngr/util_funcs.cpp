#include "util_funcs.h"

#include <wx/clipbrd.h>
#include <wx/tokenzr.h>
#include <wx/fontenum.h>


namespace util
{

	void ShowInExplorer(const std::filesystem::path& FullPath)
	{
		wxString cmd = L"explorer /select, " + FullPath.wstring();
		wxExecute(cmd, wxEXEC_ASYNC, NULL);
	}


	bool ClipbrdSupportsText()
	{
		if (!wxTheClipboard->Open())
			return false;

		bool Supports = wxTheClipboard->IsSupported(wxDF_TEXT);
		wxTheClipboard->Close();

		return Supports;
	}


	wxBitmap CreateBitmap(
		const wxString& Text,
		const wxSize& szBMP,
		const wxFont& Font,
		const wxColour& BG,
		const wxColour& TxtFG,
		wxWindow* wnd)
	{
		int BMPWidth = szBMP.GetWidth();
		int BMPHeight = szBMP.GetHeight();

		wxBitmap bmp(BMPWidth, BMPHeight);

		wxMemoryDC memDC;
		memDC.SelectObject(bmp);
		if (BG.IsOk())
			memDC.SetBackground(BG);
		else
			memDC.SetBackground(*wxWHITE);
		memDC.Clear();

		if (TxtFG.IsOk())
			memDC.SetTextForeground(TxtFG);

		if (Font.IsOk())
			memDC.SetFont(Font);

		wxSize szTxt;
		if (wnd)
			szTxt = wnd->FromDIP(memDC.GetTextExtent(Text));
		else
			szTxt = memDC.GetTextExtent(Text);

		int y = BMPHeight / 2 - szTxt.GetHeight() / 2;
		int x = BMPWidth / 2 - szTxt.GetWidth() / 2;

		if (!Text.empty())
			memDC.DrawText(Text, wxPoint(x, y));

		return bmp;
	}


	wxBitmap GetAsBitmap(wxWindow* window, const wxPoint& TopLeft, const wxSize& Size, wxColor BGColor)
	{
		wxClientDC cdc(window);

		int Width = Size.GetWidth(), Height = Size.GetHeight();
		wxBitmap bmp(Width, Height);

		wxMemoryDC memDC;
		memDC.SelectObject(bmp);
		memDC.SetBackground(BGColor);
		memDC.Clear();

		memDC.Blit(wxPoint(0, 0), wxSize(Width, Height), &cdc, TopLeft);

		memDC.SelectObject(wxNullBitmap);

		return bmp;
	}



	wxBitmap GetAsBitmap(wxWindow* window, wxColor BGColor)
	{
		int Width, Height;
		wxPoint TopLeft;

		if (window->IsTopLevel())
		{
			Width = window->GetClientRect().width;
			Height = window->GetClientRect().height;

			TopLeft = window->GetClientRect().GetTopLeft();
		}
		else
		{
			Width = window->GetRect().width;
			Height = window->GetRect().height;

			TopLeft = window->GetRect().GetTopLeft();
		}

		return GetAsBitmap(window, TopLeft, wxSize(Width, Height), BGColor);
	}


	wxArrayString GetFontFaces(bool Sorted)
	{
		wxArrayString fontFaceList;
		wxFontEnumerator fe;
		fe.EnumerateFacenames();
		fontFaceList = fe.GetFacenames();

		for (size_t i = 0; i < fontFaceList.size(); i++)
		{
			wxString str = fontFaceList[i];
			if (str.Contains("@")) {
				fontFaceList.Remove(str);
				continue;
			}
		}

		if (Sorted)
			fontFaceList.Sort();

		return fontFaceList;
	}


	/*
	
	static bool Check_SciSuitPkg()
	{
		std::string PythonCmd = "import pkgutil \n"
			"x = pkgutil.iter_modules() \n"
			"SCISUIT = False \n"
			"for i in x: \n"
			"    if (i.ispkg==True and i.name == \"scisuit\"): \n"
			"        SCISUIT=True \n"
			"        break";

		auto Module = PyModule_New("Check_SciSuitPkg");
		auto Dict = PyModule_GetDict(Module);

		bool IsInstalled = false;

		auto ResultObj = PyRun_String(PythonCmd.c_str(), Py_file_input, Dict, Dict);
		if (ResultObj)
		{
			PyObject* wxObj = PyDict_GetItemString(Dict, "SCISUIT");
			IsInstalled = Py_IsTrue(wxObj);
		}

		Py_XDECREF(Module);

		return IsInstalled;
	}

	
	*/
}