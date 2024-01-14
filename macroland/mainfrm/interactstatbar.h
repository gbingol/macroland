#pragma once

#include <wx/wx.h>

class InteractiveStatusBar :public wxStatusBar
{
public:

	InteractiveStatusBar() = default;

	InteractiveStatusBar(
		wxWindow* parent,
		wxWindowID id = wxID_ANY,
		long style = wxSTB_DEFAULT_STYLE,
		const wxString& name = wxASCII_STR(wxStatusBarNameStr));

	~InteractiveStatusBar();

	/*
		Catches statbarmouseevent
		Therefore, reports the rectangle field number and the rectangle
	*/
	void CatchStatBarMouseEvents(bool LeftDown, bool LeftUp, bool RightDown, bool RightUp)
	{
		m_BindLeftDown = LeftDown;
		m_BindLeftUp = LeftUp;
		m_BindRightDown = RightDown;
		m_BindRightUp = RightUp;
	}


protected:

	void OnLeftDown(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);

	void OnRightDown(wxMouseEvent& event);
	void OnRightUp(wxMouseEvent& event);

private:

	bool m_BindLeftDown{ true };
	bool m_BindLeftUp{ true };
	bool m_BindRightDown{ true };
	bool m_BindRightUp{ true };
};



/************************************************************************/

class StatBarMouseEvent : public wxMouseEvent
{
public:

	StatBarMouseEvent(wxEventType eventType = wxEVT_NULL) :
		wxMouseEvent(eventType) {}

	StatBarMouseEvent(const StatBarMouseEvent& other) : wxMouseEvent(other)
	{
		m_RectFieldNumber = other.m_RectFieldNumber;
	}

	wxEvent* Clone() const
	{
		return new StatBarMouseEvent(*this);
	}


	auto GetRectFieldNumber() const {
		return m_RectFieldNumber;
	}

	void SetRectFieldNumber(int number) {
		m_RectFieldNumber = number;
	}

	auto GetRectField() const {
		return m_RectField;
	}

	void SetRectField(const wxRect& fieldRect) {
		m_RectField = fieldRect;
	}


private:
	//the number of rectangle field at the status bar
	int m_RectFieldNumber{ 0 };

	wxRect m_RectField{};
};


wxDECLARE_EVENT(ssEVT_STATBAR_LEFT_DOWN, StatBarMouseEvent);
wxDECLARE_EVENT(ssEVT_STATBAR_LEFT_UP, StatBarMouseEvent);
wxDECLARE_EVENT(ssEVT_STATBAR_RIGHT_DOWN, StatBarMouseEvent);
wxDECLARE_EVENT(ssEVT_STATBAR_RIGHT_UP, StatBarMouseEvent);