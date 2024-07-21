#include "cmdline.h"

#include <wx/sstream.h>
#include <wx/artprov.h>
#include <wx/gdicmn.h> 

#include <lua.hpp>
#include "../lua/luautil.h"

#include <boost/json/value.hpp>
#include <boost/json/array.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>

#include <script/scriptviewer.h>

#include "../python/PythonWrapper.h"

#include <script/scripting_funcs.h>

#include "../consts.h"




extern std::filesystem::path glbExeDir;
extern lua_State* glbLuaState;


static wxString CmdListtoCmd(const std::list<wxString>& lst)
{
	wxString retStr;
	for (const auto& cmd : lst)
		retStr += cmd + "\n";

	retStr.RemoveLast(1);

	return retStr;
}


namespace scripting::cmdedit
{
	
	wxDEFINE_EVENT(ssEVT_SCRIPTCTRL_RETURN, wxCommandEvent);


	
	/************************************************************************/

	CInputWnd::CInputWnd(CCmdLine* parent, PyObject* Module) : 
		script::CInputWndBase(parent, Module)
	{
		m_ParentWnd = parent;

		m_PyModule = Module;

		OpenHistoryFile();
		m_HistPos = m_CmdHist.size();

		m_Txt->Bind(ssEVT_SCRIPTCTRL_RETURN, &CInputWnd::OnReturn, this);
	}


	CInputWnd::~CInputWnd()
	{
		CloseHistoryFile();
	}


	void CInputWnd::OnKeyDown(wxKeyEvent& evt)
	{
		int evtCode = evt.GetKeyCode();
		int KeyCode = evt.GetKeyCode();

		if (m_Mode == MODE::SINGLE && 
			(evtCode == WXK_UP || evtCode == WXK_DOWN) && 
			!m_Txt->AutoCompActive())
		{
			if (m_CmdHist.size() == 0)
				return;

			evtCode == WXK_UP ? m_HistPos--: m_HistPos++;

			if (m_HistPos < 0 || m_HistPos >= m_CmdHist.size())
			{
				m_HistPos = std::clamp(m_HistPos, 0, (int)(m_CmdHist.size() - 1));
				wxBell();
				return;
			}

			const auto& CurCmd = m_CmdHist.at(m_HistPos);
			if (std::holds_alternative<wxString>(CurCmd))
				m_Txt->SetText(std::get<wxString>(CurCmd));
			else
			{
				m_Txt->SetText(CmdListtoCmd(std::get<std::list<wxString>>(CurCmd)));
				SwitchToSingleMode();
			}

			m_Txt->GotoPos(m_Txt->GetLastPosition());

			return;
		}

		else if (
			(evtCode == WXK_SPACE && evt.ControlDown()) || 
			(m_Mode == MODE::SINGLE && evtCode == WXK_TAB))
		{
			ShowAutoComp();
			return;
		}

		else if ((KeyCode == WXK_NUMPAD_ENTER || KeyCode == WXK_RETURN))
		{
			if (m_AutoComp->IsActive())
			{
				m_AutoComp->Hide();

				//In single mode do not add a new line
				if(m_Mode == MODE::SINGLE)
					return;
			}

			auto PostReturnEvent = [&]()
			{
				wxCommandEvent retEvt(ssEVT_SCRIPTCTRL_RETURN);
				retEvt.SetId(evt.GetId());
				retEvt.SetEventObject(evt.GetEventObject());

				wxPostEvent(m_Txt, retEvt);
			};

			/*
				If multiple line mode, we need the Shift key to execute command
				If Shiftkey is not pressed, then we need to add lines (skip the event and return)
			*/
			if ((m_Mode == MODE::MULTI && evt.ShiftDown() == false) || 
				(m_Mode == MODE::SINGLE && evt.ShiftDown() == true))
			{
				evt.Skip();
				return;
			}

			PostReturnEvent();

			return;
		}

		int x, y;
		m_Txt->GetTextExtent(m_Txt->GetText(), &x, &y);
		m_Txt->SetScrollWidth(x);
		m_Txt->SetUseHorizontalScrollBar(x > m_Txt->GetClientSize().x);

		evt.Skip();
	}


	void CInputWnd::OnReturn(wxCommandEvent& evt)
	{
		auto OutWnd = m_ParentWnd->GetOutputWnd();

		if (!m_Txt->GetText().empty())
			m_Txt->GotoPos(m_Txt->GetLastPosition());

		wxString CmdStr;

		if (m_Mode == MODE::SINGLE)
		{
			CmdStr = m_Txt->GetLineText(0);
			CmdStr.Trim().Trim(false);
			
			if(!CmdStr.empty())
				m_CmdHist.push_back(CmdStr);
		}
		else
		{
			CmdStr = m_Txt->GetText();
			CmdStr.Trim().Trim(false);

			if (!CmdStr.empty())
			{

				std::list<wxString> Cmds;
				for (size_t i = 0; i < m_Txt->GetLineCount(); ++i)
				{
					wxString curCmd = m_Txt->GetLineText(i);
					curCmd.Trim();
					Cmds.push_back(curCmd);
				}

				m_CmdHist.push_back(Cmds);
			}
		}
		
		m_Txt->SetText("");

		if (CmdStr.empty())
		{
			OutWnd->AppendOutput(">>");
			evt.Skip();
			return;
		}

		OutWnd->AppendOutput(m_Mode == MODE::SINGLE ? ">>" + CmdStr : ">>{" + CmdStr + "}");

		wxString CmdOutput = ProcessCommand(CmdStr);
		if (!CmdOutput.empty())
			OutWnd->AppendOutput(CmdOutput);

		//reset history position to show the last (this) command
		m_HistPos = m_CmdHist.size();

		SwitchToSingleMode();

		evt.Skip();
	}



	bool CInputWnd::OpenHistoryFile(std::string* Msg)
	{
		std::string HISTFILE = "home/cmdline_history.json";

		wxFile file;

		auto histPath = glbExeDir / HISTFILE;
		if (!std::filesystem::exists(histPath))
			file.Create(histPath.wstring(), true);
	
		bool IsOpened = file.Open(histPath.wstring(), wxFile::read);
		if (!IsOpened)
			return false;

		wxString JSON;
		file.ReadAll(&JSON, wxConvUTF8);
		file.Close();

		if (JSON.empty())
		{
			if(Msg)
				*Msg = "JSON file is empty";
			
			//no point to return false
			return true;
		}

		JSON = JSON.Trim().Trim(false);
		const char* EncodedContent = JSON.mb_str(wxConvUTF8);

		boost::json::error_code ec;
		boost::json::value val = boost::json::parse(EncodedContent, ec);

		if (ec.failed() || val.is_array() == false)
		{
			if(Msg)
				*Msg = "Error parsing the JSON file, must start with [ and end with ]";
			return false;
		}

		auto MainArr = val.as_array();
		for (size_t i = 0; i < MainArr.size(); ++i)
		{
			if (!MainArr[i].is_array())
				continue;

			auto Arr = MainArr[i].as_array();
			if (Arr.size() == 0)
				continue;

			if(Arr.size() == 1)
				m_CmdHist.push_back(wxString::FromUTF8(Arr[0].as_string().c_str()));
			else
			{
				std::list<wxString> lst;
				for (size_t j = 0; j < Arr.size(); ++j)
				{
					if (!Arr[j].is_string())
						continue;
					wxString str = wxString::FromUTF8(Arr[j].as_string().c_str());
					lst.push_back(str);
				}

				m_CmdHist.push_back(lst);
			}
		}

		return true;
	}


	bool CInputWnd::CloseHistoryFile()
	{
		//Nothing to write
		if (m_CmdHist.size() == 0)
			return 0;

		std::string HISTFILE = "home/cmdline_history.json";
		auto histPath = glbExeDir / HISTFILE;
		wxFile file(histPath.wstring(), wxFile::write);

		wxString JSON;

		boost::json::array MainArr;
		for (size_t i = 0; i < m_CmdHist.size(); ++i)
		{
			boost::json::array BoostArr;
			if (std::holds_alternative<std::list<wxString>>(m_CmdHist[i]))
			{
				const auto& Lst = std::get<std::list<wxString>>(m_CmdHist[i]);
				for (const auto& cmd : Lst)
				{
					boost::json::string BoostStr = (const char*)cmd.mb_str(wxConvUTF8);
					BoostArr.push_back(BoostStr);
				}	
			}
			else
			{
				wxString str = std::get<wxString>(m_CmdHist[i]);
				boost::json::string BoostStr = (const char*)str.mb_str(wxConvUTF8);
				BoostArr.push_back(BoostStr);
			}

			MainArr.push_back(BoostArr);
		}

		JSON << wxString::FromUTF8(boost::json::serialize(MainArr));

		return file.Write(JSON) && file.Close();
	}



	/**************************  CCmdLine  **********************************/

	CCmdLine::CCmdLine(wxWindow* parent, PyObject* Module) : 
		wxControl(parent, wxID_ANY)
	{
		m_PrntWnd = parent;

		SetBackgroundColour(wxColour(255, 255, 255));

		m_TxtInput = new CInputWnd(this, Module);
		m_TxtOutput = new script::COutputWnd(this, wxID_ANY, m_TxtInput->GetScriptCtrl());

		Bind(wxEVT_PAINT, &CCmdLine::OnPaint, this);
		Bind(wxEVT_SIZE, [&](wxSizeEvent& event)
		{
			m_TxtOutput->Refresh();
			m_TxtInput->Refresh();
			event.Skip();
		});
	}


	wxSize CCmdLine::DoGetBestSize() const
	{
		wxSize sz = m_TxtInput->GetBestSize();
		//wxSize sz2 = m_TxtOutput->GetBestSize();

		return sz;
	}


	void CCmdLine::OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);
		wxSize szClnt = GetClientSize();

		wxRect OutputRect;
		wxPoint BL = wxPoint(0, 0);
		if (m_TxtOutput->GetTextLength() == 0)
			m_TxtOutput->Hide();
		else
		{
			m_TxtOutput->Show();
			OutputRect = m_TxtOutput->GetRect();
			BL = OutputRect.GetBottomLeft();
		}

		m_TxtInput->SetSize(wxSize(szClnt.x, szClnt.y - OutputRect.GetHeight()));
		m_TxtInput->SetPosition(BL);

		m_TxtOutput->Refresh();
		m_TxtInput->Refresh();
	}

}