#include <Solver.h>
#include <wx/wx.h>
#include <wx/bookctrl.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>
#include <wx/colour.h>
#include <sstream>

class MyFrame : public wxFrame {
public:
    MyFrame(char const* name);
    
    void OnNewRestrButton(wxCommandEvent&);
    void OnDelRestrButton(wxCommandEvent&);
    void OnSolveButton(wxCommandEvent&);
    void on_text_change(wxCommandEvent&);
    void on_cell_dclick(wxGridEvent&);
    
    void ClearNotebooks();
    void FillNotebook(std::vector<Solver::Step> const& steps, wxNotebook* book);

private:
    wxBoxSizer* restrSizer;
    wxBoxSizer* mainSizer;

    wxTextCtrl* goalEntry;
    std::vector<wxTextCtrl*> restrEntries;
    
    std::vector<std::pair<wxGrid*, int>> stepsGrids;
    wxNotebook* directStepsBook;
    wxNotebook* invertStepsBook;
};

MyFrame::MyFrame(char const* name)
: wxFrame(nullptr, wxID_ANY, name, wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, "MyFrame")
{
    mainSizer = new wxBoxSizer(wxVERTICAL);
    
    goalEntry = new wxTextCtrl(
        this, wxID_ANY, wxT("x1 + x2 => min"),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER
    );
    goalEntry->Bind(wxEVT_TEXT, &MyFrame::on_text_change, this);
    mainSizer->Add(goalEntry, wxSizerFlags().Expand().Border(wxALL, 5));
    
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton* newRestrButton = new wxButton( this, wxID_ANY, wxT("+ ограничение") );
    buttonSizer->Add(newRestrButton, wxSizerFlags(1).Border(wxALL, 5));
    wxButton* delRestrButton = new wxButton( this, wxID_ANY, wxT("- ограничение") );
    buttonSizer->Add(delRestrButton, wxSizerFlags(1).Border(wxALL, 5));
    wxButton* solveButton = new wxButton( this, wxID_ANY, wxT("Решить") );
    buttonSizer->Add(solveButton, wxSizerFlags(1).Border(wxALL, 5));
    
    mainSizer->Add(buttonSizer, wxSizerFlags().Expand());
    
    
    restrSizer = new wxBoxSizer(wxVERTICAL);
    
    restrEntries.emplace_back(new wxTextCtrl(
        this, wxID_ANY, wxT("2x1 + 4x2 <= 16"),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER
    ));
    restrEntries.emplace_back(new wxTextCtrl(
        this, wxID_ANY, wxT("-4x1 + 2x2 <= 8"),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER
    ));
    restrEntries.emplace_back(new wxTextCtrl(
        this, wxID_ANY, wxT("1x1 + 3x2 >= 9"),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER
    ));
    for(auto& entry : restrEntries) {
        restrSizer->Add(entry, wxSizerFlags().Expand().Border(wxALL, 5));
        entry->Bind(wxEVT_TEXT, &MyFrame::on_text_change, this);
    }
    
    mainSizer->Add(restrSizer, wxSizerFlags().Expand());
    
    
    auto directStepsLabel = new wxStaticText(this, wxID_ANY, wxT("Прямая задача:"));
    mainSizer->Add(directStepsLabel, wxSizerFlags().Expand().Border(wxALL, 5));
    directStepsBook = new wxNotebook(this, wxID_ANY);
    mainSizer->Add(directStepsBook, wxSizerFlags().Expand().Border(wxALL, 5));
    
    auto invertStepsLabel = new wxStaticText(this, wxID_ANY, wxT("Двойственная задача:"));
    mainSizer->Add(invertStepsLabel, wxSizerFlags().Expand().Border(wxALL, 5));
    invertStepsBook = new wxNotebook(this, wxID_ANY);
    mainSizer->Add(invertStepsBook, wxSizerFlags().Expand().Border(wxALL, 5));
    
    
    newRestrButton->Bind(wxEVT_BUTTON, &MyFrame::OnNewRestrButton, this);
    delRestrButton->Bind(wxEVT_BUTTON, &MyFrame::OnDelRestrButton, this);
    solveButton   ->Bind(wxEVT_BUTTON, &MyFrame::OnSolveButton,    this);
    
#ifdef __MINGW32__
    SetBackgroundColour(*wxWHITE);
#endif
    SetSizerAndFit(mainSizer);
    Layout();
    Center(wxBOTH);
}

void MyFrame::on_text_change(wxCommandEvent& evt) {
    if(!stepsGrids.empty()) {
        ClearNotebooks();
        mainSizer->SetSizeHints(this);
        Fit();
    }
}

void MyFrame::OnNewRestrButton(wxCommandEvent& evt) {
    if(restrEntries.size() >= 6) return;
    
    auto textCtrl = new wxTextCtrl(
        this, wxID_ANY, wxT("Новое ограничение"),
        wxDefaultPosition, wxDefaultSize,
        wxTE_PROCESS_ENTER
    );
    restrEntries.emplace_back(textCtrl);
    textCtrl->Bind(wxEVT_TEXT, &MyFrame::on_text_change, this);
    restrSizer->Add(textCtrl, wxSizerFlags().Expand().Border(wxALL, 5));
    
    restrSizer->SetSizeHints(this);
    Fit();
}

void MyFrame::OnDelRestrButton(wxCommandEvent& evt) {
    if(restrEntries.size() <= 1) return;
    
    restrSizer->Detach(restrEntries.back());
    delete restrEntries.back();
    restrEntries.pop_back();
    
    restrSizer->SetSizeHints(this);
    Fit();
}

void MyFrame::ClearNotebooks() {
    directStepsBook->DeleteAllPages();
    invertStepsBook->DeleteAllPages();
    
    stepsGrids.clear();
}

inline namespace helpers {
    template <typename T>
    std::string to_string(T const& obj) {
        std::ostringstream os;
        os << obj;
        return os.str();
    }
}

void MyFrame::on_cell_dclick(wxGridEvent& evt) {
    int col = evt.GetCol();
    int row = evt.GetRow();
    int id = evt.GetId();
    
    for(auto const p : stepsGrids) {
        if(p.second != id) continue;
        if(wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(p.first->GetCellValue(row, col)));
            wxTheClipboard->Close();
        }
    }
}

void MyFrame::FillNotebook(std::vector<Solver::Step> const& steps, wxNotebook* book) {
    bool solutionValid = steps.back().valid();
    if(!solutionValid) return;
    
    using fracType = Fraction;
    
    int stepN = 1;
    for(auto const& step : steps) {
        int pageId = wxNewId();
        wxGrid* page = new wxGrid(book, pageId);
        stepsGrids.emplace_back(page, pageId);
        
        int needMPrice = 0;
        for(auto col : step.mprice.terms()) {
            if(col.coeff() != 0) {
                needMPrice = 1;
                break;
            }
        }
        
        page->CreateGrid(
            step.restrs.size() + 2 + static_cast<int>(needMPrice),
            step.goal.size() + 2
        );
        page->HideColLabels();
        page->HideRowLabels();
        page->EnableDragColSize(false);
        page->EnableDragRowSize(false);
        page->EnableDragGridSize(false);
        page->EnableEditing(false);
        page->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &MyFrame::on_cell_dclick, this);
        
        // goal row
        int row = 0;
        int col = 0;
        page->SetCellValue(row, col++, step.goal.right());
        page->SetCellValue(row, col++, "B");
        for(int j : step.goal.indices()) {
            page->SetCellValue(row, col++, to_string(step.goal.term(j)));
        }
        
        // restrictions rows
        for(auto const& restr : step.restrs) {
            ++row;
            col = 0;
            page->SetCellValue(row, col++, to_string(step.sel[row - 1]));
            page->SetCellValue(
                row, col++, 
                to_string(static_cast<fracType>(restr.right()))
            );
            for(int j : restr.indices()) {
                page->SetCellValue(
                    row, col++, 
                    to_string(static_cast<fracType>(restr.coeff(j)))
                );
            }
        }
        
        // plain price row
        ++row;
        col = 0;
        page->SetCellValue(row, col++, "W");
        page->SetCellValue(
            row, col++, 
            to_string(static_cast<fracType>(step.w))
        );
        for(int fi : step.pprice.indices()) {
            page->SetCellValue(
                row, col++, 
                to_string(static_cast<fracType>(step.pprice.coeff(fi)))
            );
        }
        
        // mega price row
        if(needMPrice) {
            ++row;
            col = 0;
            page->SetCellValue(row, col++, "M");
            page->SetCellValue(
                row, col++, 
                to_string(static_cast<fracType>(step.m))
            );
            for(int fi : step.mprice.indices()) {
                page->SetCellValue(
                    row, col++, 
                    to_string(static_cast<fracType>(step.mprice.coeff(fi)))
                );
            }
        }
        
        page->AutoSize();
        book->AddPage(page, wxString::Format(wxT("Шаг %d"), stepN++));
    }
    
    mainSizer->SetSizeHints(this);
    Fit();
}

void MyFrame::OnSolveButton(wxCommandEvent& evt) {
    Solver solver;
    
    if(!solver.set_goal(goalEntry->GetValue().ToStdString())) {
        wxMessageBox(wxT("Неверный формат целевой функции"), 
                     wxT("Ошибка"), wxOK|wxCENTER|wxICON_ERROR);
        return;
    }
    
    for(auto const& entry : restrEntries) {
        bool restrSet = solver.add_restriction(entry->GetValue().ToStdString());
        
        if(!restrSet) {
            wxMessageBox(wxT("Неверный формат ограничения"), 
                         wxT("Ошибка"), wxOK|wxCENTER|wxICON_ERROR);
            return;
        }
    }
    
    Solver invSolver = solver;
    invSolver.invert_to_dual();
    
    auto steps = solver.solve();
    auto invSteps = invSolver.solve();
    
    if(!steps.back().valid()) {
        wxMessageBox(wxT("Неразрешимая система"),
                     wxT("Ошибка"), wxOK|wxCENTER|wxICON_ERROR);
        return;
    }
    
    ClearNotebooks();
    FillNotebook(steps, directStepsBook);
    FillNotebook(invSteps, invertStepsBook);
}

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
};

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("CO 23");
    frame->Show();
    return true;
}

IMPLEMENT_APP(MyApp)
