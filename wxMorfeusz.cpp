#include "morfeusz.h"


#include <wx/wx.h>
#include <wx/srchctrl.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/richtext/richtextstyles.h>


#define MAIN_WINDOW_NAME _("wxMorfeusz")
#define MAIN_WINDOW_INITIAL_SIZE wxSize(800, 600)
#define DEFAULT_STATUS wxEmptyString
#define WXMORFEUSZ_ABOUT _("\nwxMorfeusz SGJP wersja 0.33 <2013/05/27>\n Copyright (c) 2013 by Marcin Woliński, Łukasz Kożuchowski.\n\n")
#define WX_BORDER_WIDTH 5


enum
{
  ID_Panel = 1,
  ID_Open,
  ID_Quit,
  ID_View,
  ID_Options,
  ID_Contents,
  ID_Notation,
  ID_License,
  ID_About,
  ID_Search,
  ID_Results,
};


class wxMorfeusz : public wxApp
{
  virtual bool OnInit();
};


IMPLEMENT_APP(wxMorfeusz)


class MainFrame : public wxFrame
{
public:
  MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

protected:
  wxSearchCtrl *srchCtrl;
  wxRichTextCtrl *morfeuszResultsBox;

  void OnOpen(wxCommandEvent& event);
  void OnQuit(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);
  void OnSearch(wxCommandEvent& event);
  void OnClear(wxCommandEvent& event);
  void NotImplemented(wxCommandEvent& event);

private:
  DECLARE_EVENT_TABLE();
};


BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU(ID_Open, MainFrame::OnOpen)
    EVT_MENU(ID_Quit, MainFrame::OnQuit)
    EVT_MENU(ID_View, MainFrame::NotImplemented)
    EVT_MENU(ID_Options, MainFrame::NotImplemented)
    EVT_MENU(ID_Contents, MainFrame::NotImplemented)
    EVT_MENU(ID_Notation, MainFrame::NotImplemented)
    EVT_MENU(ID_License, MainFrame::NotImplemented)
    EVT_MENU(ID_About, MainFrame::OnAbout)
    EVT_SEARCHCTRL_SEARCH_BTN(ID_Search, MainFrame::OnSearch)
    EVT_SEARCHCTRL_CANCEL_BTN(ID_Search, MainFrame::OnClear)
    EVT_TEXT_ENTER(ID_Search, MainFrame::OnSearch)
END_EVENT_TABLE()


/*
  TODO:
  Replace wxString->wxString function with a function writing directly
  to morfeuszResultsBox (changing fonts, creating tables etc.)
*/
wxString wxMorfeusz_analyse(wxString inputString)
{
  InterpMorf *results;
  wxString resultsString;
  int i, prevp = -1;

  results = morfeusz_analyse((char*) inputString.utf8_str().data());

  if (results[0].p == -1) {
     //fprintf(stderr, "wxMorfeusz_analyse: results[0].p == -1");
     return wxEmptyString;
  }

  resultsString += _("[");

  for (i = 0; results[i].p != -1; i++) {
    if (prevp != -1) {
      if (prevp != results[i].p)
        resultsString += _("]\n[");
      else
        resultsString += _("; ");
    }

    resultsString += wxString::FromUTF8(results[i].forma) + _(",") +
      wxString::FromUTF8(results[i].haslo==NULL?"?":results[i].haslo) + _(",") +
      wxString::FromUTF8(results[i].interp==NULL?"?":results[i].interp);

    prevp = results[i].p;
  }
  resultsString += _("]");

  return resultsString;
}


bool wxMorfeusz::OnInit()
{
  MainFrame *frame = new MainFrame(MAIN_WINDOW_NAME, wxDefaultPosition,
                                   MAIN_WINDOW_INITIAL_SIZE);
  frame->Centre();
  frame->Show(true);
  SetTopWindow(frame);
  return true;
}


MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
  : wxFrame(NULL, -1, title, pos, size)
{
  wxMenuBar *menuBar = new wxMenuBar;
  wxMenu *menuFile = new wxMenu;
  wxMenu *menuSettings = new wxMenu;
  wxMenu *menuHelp = new wxMenu;
  wxPanel *mainPanel = new wxPanel(this, ID_Panel, wxDefaultPosition, wxDefaultSize,
                                   wxNO_BORDER|wxTAB_TRAVERSAL);
  srchCtrl = new wxSearchCtrl(mainPanel, ID_Search, wxEmptyString,
                              wxDefaultPosition, wxDefaultSize,
                              wxTE_PROCESS_ENTER|wxTE_NOHIDESEL);
  morfeuszResultsBox = new wxRichTextCtrl(mainPanel, ID_Results, wxEmptyString,
                                          wxDefaultPosition, wxDefaultSize,
                                          wxTE_READONLY|wxTE_MULTILINE);
  wxSizer *sizer = new wxBoxSizer(wxVERTICAL);

  morfeusz_set_option(MORFOPT_ENCODING, MORFEUSZ_UTF_8);
  morfeusz_set_option(MORFOPT_WHITESPACE, MORFEUSZ_KEEP_WHITESPACE);

  srchCtrl -> SetDescriptiveText(wxEmptyString);
  srchCtrl -> ShowSearchButton(true);
  srchCtrl -> ShowCancelButton(true);
  srchCtrl -> SetFocus();

  sizer->Add(srchCtrl, 0, wxGROW|wxALL, WX_BORDER_WIDTH);
  sizer->Add(morfeuszResultsBox, 1, wxEXPAND|wxGROW|wxALL, WX_BORDER_WIDTH);

  mainPanel->SetSizer(sizer);

  // FIXME: letter choices: &Plik vs &Pomoc etc.
  menuFile->Append(ID_Open, _("&Analizuj plik..."));
  menuFile->AppendSeparator();
  menuFile->Append(ID_Quit, _("&Wyjście"));

  menuSettings->Append(ID_View, _("&Widok"));
  menuSettings->Append(ID_Options, _("&Opcje"));

  menuHelp->Append(ID_Contents, _("&Spis treści"));
  menuHelp->Append(ID_Notation, _("Stosowane &oznaczenia"));
  menuHelp->Append(ID_License, _("&Licencja"));
  menuHelp->Append(ID_About, _("O &programie"));

  menuBar->Append(menuFile, _("&Plik"));
  menuBar->Append(menuSettings, _("&Ustawienia"));
  menuBar->Append(menuHelp, _("P&omoc"));

  SetMenuBar(menuBar);

  CreateStatusBar();
  SetStatusText(DEFAULT_STATUS);
}


void MainFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
  wxString filePath, fileName, str;
  wxTextFile file;
  wxFileDialog *OpenDialog =
    new wxFileDialog(this, _("Wybierz plik do zanalizowania"),
                     wxEmptyString, wxEmptyString,
                     _("Pliki tekstowe (*.txt)|*.txt|Dowolne pliki (*.*)|*.*"),
                     wxFD_OPEN, wxDefaultPosition);

  if (OpenDialog->ShowModal() == wxID_OK) // the user clicked "Open" (not "Cancel")
    {
      filePath = OpenDialog->GetPath();
      file.Open(filePath);
      srchCtrl->Clear();
      morfeuszResultsBox->Clear();

      // Line by line analysis
      for (str = file.GetFirstLine(); !file.Eof(); str = file.GetNextLine()) {
        morfeuszResultsBox->BeginBold();
        morfeuszResultsBox->WriteText(str + _("\n"));
        morfeuszResultsBox->EndBold();
        morfeuszResultsBox->WriteText(wxMorfeusz_analyse(str));
      }

      fileName = OpenDialog->GetFilename();
      SetTitle(fileName);
      SetStatusText(_("Wczytano ") + fileName);
    }
}


void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
  Close(true);
}


void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxMessageBox(wxString::FromUTF8(morfeusz_about())
               + WXMORFEUSZ_ABOUT,
               _("O programie"),
               wxOK|wxICON_INFORMATION, this);
}


void MainFrame::NotImplemented(wxCommandEvent& WXUNUSED(event))
{
  wxMessageBox(_("Jeszcze niezaimplementowane"),
               _("W budowie"),
               wxOK|wxICON_INFORMATION, this);
}


void MainFrame::OnSearch(wxCommandEvent& WXUNUSED(event))
{
  wxString inputString = srchCtrl->GetValue(),
           resultsString = wxEmptyString;

  srchCtrl->Clear();
  morfeuszResultsBox->Clear();

  morfeuszResultsBox->BeginBold();
  morfeuszResultsBox->WriteText(inputString + _("\n"));
  morfeuszResultsBox->EndBold();
  morfeuszResultsBox->WriteText(wxMorfeusz_analyse(inputString));
  SetTitle(MAIN_WINDOW_NAME);
  SetStatusText(DEFAULT_STATUS);
}


void MainFrame::OnClear(wxCommandEvent& WXUNUSED(event))
{
  srchCtrl->Clear();
  morfeuszResultsBox->Clear();
  SetTitle(MAIN_WINDOW_NAME);
  SetStatusText(DEFAULT_STATUS);
}
