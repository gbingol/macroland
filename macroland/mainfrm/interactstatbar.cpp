#include "interactstatbar.h"

wxDEFINE_EVENT(ssEVT_STATBAR_LEFT_DOWN, StatBarMouseEvent);
wxDEFINE_EVENT(ssEVT_STATBAR_LEFT_UP, StatBarMouseEvent);
wxDEFINE_EVENT(ssEVT_STATBAR_RIGHT_DOWN, StatBarMouseEvent);
wxDEFINE_EVENT(ssEVT_STATBAR_RIGHT_UP, StatBarMouseEvent);


InteractiveStatusBar::InteractiveStatusBar(
	wxWindow* parent,
	wxWindowID id,
	long style,
	const wxString& name) :wxStatusBar(parent, id, style, name)
{
	Bind(wxEVT_LEFT_DOWN, &InteractiveStatusBar::OnLeftDown, this);
	Bind(wxEVT_LEFT_UP, &InteractiveStatusBar::OnLeftUp, this);

	Bind(wxEVT_RIGHT_DOWN, &InteractiveStatusBar::OnRightDown, this);
	Bind(wxEVT_RIGHT_UP, &InteractiveStatusBar::OnRightUp, this);
}


InteractiveStatusBar::~InteractiveStatusBar()
{
	Unbind(wxEVT_LEFT_DOWN, &InteractiveStatusBar::OnLeftDown, this);
	Unbind(wxEVT_LEFT_UP, &InteractiveStatusBar::OnLeftUp, this);

	Unbind(wxEVT_RIGHT_DOWN, &InteractiveStatusBar::OnRightDown, this);
	Unbind(wxEVT_RIGHT_UP, &InteractiveStatusBar::OnRightUp, this);
}


void InteractiveStatusBar::OnLeftDown(wxMouseEvent& event)
{
	if (m_BindLeftDown)
	{
		wxPoint pos = event.GetPosition();

		int Field = 0;
		for (Field = 0; Field < GetFieldsCount(); ++Field) {
			wxRect R;
			GetFieldRect(Field, R);

			if (R.Contains(pos)) {
				StatBarMouseEvent MouseEvent(ssEVT_STATBAR_LEFT_DOWN);
				MouseEvent.SetRectFieldNumber(Field);
				MouseEvent.SetRectField(R);

				wxPostEvent(this, MouseEvent);
				return;
			}
		}
	}

	event.Skip();
}


void InteractiveStatusBar::OnLeftUp(wxMouseEvent& event)
{
	if (m_BindLeftUp)
	{
		wxPoint pos = event.GetPosition();

		int Field = 0;
		for (Field = 0; Field < GetFieldsCount(); ++Field) {
			wxRect R;
			GetFieldRect(Field, R);

			if (R.Contains(pos)) {
				StatBarMouseEvent MouseEvent(ssEVT_STATBAR_LEFT_UP);
				MouseEvent.SetRectFieldNumber(Field);
				MouseEvent.SetRectField(R);

				wxPostEvent(this, MouseEvent);
				return;
			}
		}
	}

	event.Skip();
}


void InteractiveStatusBar::OnRightDown(wxMouseEvent& event)
{
	if (m_BindRightDown)
	{
		wxPoint pos = event.GetPosition();

		int Field = 0;
		for (Field = 0; Field < GetFieldsCount(); ++Field) {
			wxRect R;
			GetFieldRect(Field, R);

			if (R.Contains(pos)) {
				StatBarMouseEvent MouseEvent(ssEVT_STATBAR_RIGHT_DOWN);
				MouseEvent.SetRectFieldNumber(Field);
				MouseEvent.SetRectField(R);

				wxPostEvent(this, MouseEvent);
				return;
			}
		}
	}

	event.Skip();
}


void InteractiveStatusBar::OnRightUp(wxMouseEvent& event)
{
	if (m_BindRightUp)
	{
		wxPoint pos = event.GetPosition();

		int Field = 0;
		for (Field = 0; Field < GetFieldsCount(); ++Field) {
			wxRect R;
			GetFieldRect(Field, R);

			if (R.Contains(pos)) {
				StatBarMouseEvent MouseEvent(ssEVT_STATBAR_RIGHT_UP);
				MouseEvent.SetRectFieldNumber(Field);
				MouseEvent.SetRectField(R);

				wxPostEvent(this, MouseEvent);
				return;
			}
		}
	}

	event.Skip();
}