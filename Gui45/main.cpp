#include <BalanceMatrix.h>
#include <wx/wx.h>
#include <wx/bookctrl.h>
#include <wx/grid.h>
#include <wx/spinctrl.h>
#include <wx/clipbrd.h>
#include <wx/colour.h>
#include <sstream>

class MyFrame : public wxFrame {
public:
    MyFrame(char const* name);
    
    void size_cols();
    void size_rows();
    void on_cell_change(wxGridEvent const& evt);
    void on_cell_dclick(wxGridEvent& evt);
    void on_size_button(wxCommandEvent const& evt);
    void on_solve_button(wxCommandEvent const& evt);
    
    void clear_notebooks();
    void fill_notebook(std::vector<BalanceMatrix::Step> const& steps, wxNotebook* book);

private:
    wxBoxSizer* _mainSizer;
    
    wxSpinCtrl* _colsSpin;
    wxSpinCtrl* _rowsSpin;
    
    wxGrid* _inputGrid;
    
    wxNotebook* _NWBook;
    wxNotebook* _MinBook;
    std::vector<std::pair<wxGrid*, int>> _stepGrids;
};

using std::vector;

inline namespace helpers {
    void
    set_default_input_grid(wxGrid* const grid) {
        grid->SetCellValue(0, 0, wxT("5"));
        grid->SetCellValue(0, 1, wxT("8"));
        grid->SetCellValue(0, 2, wxT("4"));
        grid->SetCellValue(0, 3, wxT("4"));
        grid->SetCellValue(0, 4, wxT("80"));
        
        grid->SetCellValue(1, 0, wxT("1"));
        grid->SetCellValue(1, 1, wxT("2"));
        grid->SetCellValue(1, 2, wxT("3"));
        grid->SetCellValue(1, 3, wxT("8"));
        grid->SetCellValue(1, 4, wxT("45"));
        
        grid->SetCellValue(2, 0, wxT("4"));
        grid->SetCellValue(2, 1, wxT("7"));
        grid->SetCellValue(2, 2, wxT("6"));
        grid->SetCellValue(2, 3, wxT("1"));
        grid->SetCellValue(2, 4, wxT("60"));
        
        grid->SetCellValue(3, 0, wxT("45"));
        grid->SetCellValue(3, 1, wxT("60"));
        grid->SetCellValue(3, 2, wxT("70"));
        grid->SetCellValue(3, 3, wxT("40"));
    }
}

MyFrame::MyFrame(char const* name)
: wxFrame(nullptr, wxID_ANY, name, wxDefaultPosition)
{
    _mainSizer = new wxBoxSizer(wxVERTICAL);
    
    auto spinSizer = new wxBoxSizer(wxHORIZONTAL);
    auto rowLabel = new wxStaticText(this, wxID_ANY, wxT("Рядов:"));
    spinSizer->Add(rowLabel, wxSizerFlags().Border(wxALL, 5));
    _rowsSpin = new wxSpinCtrl(
        this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxSize(50, -1),
        wxSP_ARROW_KEYS|wxALIGN_RIGHT,
        4, 7, 4
    );
    spinSizer->Add(_rowsSpin, wxSizerFlags().Border(wxALL, 5));
    
    auto colLabel = new wxStaticText(this, wxID_ANY, wxT("Столбцов:"));
    spinSizer->Add(colLabel, wxSizerFlags().Border(wxALL, 5));
    _colsSpin = new wxSpinCtrl(
        this, wxID_ANY, wxEmptyString,
        wxDefaultPosition, wxSize(50, -1),
        wxSP_ARROW_KEYS|wxALIGN_RIGHT,
        4, 7, 5
    );
    spinSizer->Add(_colsSpin, wxSizerFlags().Border(wxALL, 5));
    
    auto sizeButton = new wxButton(this, wxID_ANY, wxT("Размер"));
    spinSizer->Add(sizeButton, wxSizerFlags().Border(wxALL, 5));
    _mainSizer->Add(spinSizer, wxSizerFlags().Center());
    
    _inputGrid = new wxGrid(this, wxID_ANY);
    _inputGrid->CreateGrid(_rowsSpin->GetValue(), _colsSpin->GetValue());
    _inputGrid->EnableDragColSize(false);
    _inputGrid->EnableDragRowSize(false);
    _inputGrid->HideColLabels();
    _inputGrid->HideRowLabels();
    set_default_input_grid(_inputGrid);
    _mainSizer->Add(_inputGrid, wxSizerFlags().Border(wxALL, 5).Center());
    
    auto solveButton = new wxButton(this, wxID_ANY, wxT("Решить"));
    _mainSizer->Add(solveButton, wxSizerFlags().Expand().Border(wxALL, 5));
    
    auto NWMethodLabel = new wxStaticText(this, wxID_ANY, wxT("Северо-западный угол:"));
    _mainSizer->Add(NWMethodLabel, wxSizerFlags().Expand());
    
    _NWBook = new wxNotebook(this, wxID_ANY);
    _mainSizer->Add(_NWBook, wxSizerFlags().Expand());
    
    auto MinMethodLabel = new wxStaticText(this, wxID_ANY, wxT("Минимальная стоимость:"));
    _mainSizer->Add(MinMethodLabel, wxSizerFlags().Expand());
    
    _MinBook = new wxNotebook(this, wxID_ANY);
    _mainSizer->Add(_MinBook, wxSizerFlags().Expand());
    
#ifdef __MINGW32__
    SetBackgroundColour(*wxWHITE);
#endif
    SetSizerAndFit(_mainSizer);
    
    _inputGrid->Bind(wxEVT_GRID_CELL_CHANGING, &MyFrame::on_cell_change, this);
    solveButton->Bind(wxEVT_BUTTON, &MyFrame::on_solve_button, this);
    sizeButton->Bind(wxEVT_BUTTON, &MyFrame::on_size_button, this);
}

void MyFrame::on_cell_change(wxGridEvent const& evt) {
    clear_notebooks();
    Fit();
}

void MyFrame::on_size_button(wxCommandEvent const& evt) {
    size_cols();
    size_rows();
    clear_notebooks();
    Fit();
}

void MyFrame::size_cols() {
    int newSize = _colsSpin->GetValue();
    int oldSize = _inputGrid->GetNumberCols();
    if(newSize > oldSize) {
        _inputGrid->AppendCols(newSize - oldSize);
    }
    else if(newSize < oldSize) {
        _inputGrid->DeleteCols(newSize, oldSize - newSize);
    }
}

void MyFrame::size_rows() {
    int newSize = _rowsSpin->GetValue();
    int oldSize = _inputGrid->GetNumberRows();
    if(newSize > oldSize) {
        _inputGrid->AppendRows(newSize - oldSize);
    }
    else if(newSize < oldSize) {
        _inputGrid->DeleteRows(newSize, oldSize - newSize);
    }
}

void MyFrame::clear_notebooks() {
    _NWBook->DeleteAllPages();
    _MinBook->DeleteAllPages();
    
    _stepGrids.clear();
}

void MyFrame::on_cell_dclick(wxGridEvent& evt) {
    int col = evt.GetCol();
    int row = evt.GetRow();
    int id = evt.GetId();
    
    for(auto const& p : _stepGrids) {
        if(p.second != id) continue;
        if(wxTheClipboard->Open()) {
            wxTheClipboard->SetData(new wxTextDataObject(p.first->GetCellValue(row, col)));
            wxTheClipboard->Close();
        }
    }
}

void MyFrame::fill_notebook(vector<BalanceMatrix::Step> const& steps, wxNotebook* book) {
    if(steps.empty() || !steps.back().valid()) return;
    
    int stepN = 1;
    for(auto const& step : steps) {
        int pageId = wxNewId();
        auto page = new wxGrid(book, pageId);
        _stepGrids.emplace_back(page, pageId);
        
        int rowsNum = step.X.size();
        int colsXNum = step.X[0].size();
        int colsDNum = step.D[0].size();
        int colsNum = colsXNum + colsDNum + 1;
        
        page->CreateGrid(rowsNum + 2, colsNum);
        page->HideColLabels();
        page->HideRowLabels();
        page->EnableDragColSize(false);
        page->EnableDragRowSize(false);
        page->EnableDragGridSize(false);
        page->EnableEditing(false);
        page->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &MyFrame::on_cell_dclick, this);
        
        page->SetCellValue(0, 0, "X:");
        page->SetCellValue(0, colsXNum + 1, "D:");
        page->SetCellValue(rowsNum + 1, 0, "W:");
        page->SetCellValue(rowsNum + 1, 1, wxString::Format("%d", step.W));
        
        for(int r = 0; r < rowsNum; ++r) {
            for(int c = 0; c < colsXNum; ++c) {
                wxString formatStr = "%d";
                if(step.X[r][c] < 0) formatStr += "E";
                
                page->SetCellValue(
                    r + 1, c, 
                    wxString::Format(formatStr, std::abs(step.X[r][c]))
                );
            }
            
            for(int c = 0; c < colsDNum; ++c) {
                page->SetCellValue(
                    r + 1, c + colsXNum + 1,
                    wxString::Format("%d", step.D[r][c])
                );
            }
        }
        
        page->AutoSize();
        book->AddPage(page, wxString::Format(wxT("Шаг %d"), stepN));
        ++stepN;
    }
    
    Fit();
}

void MyFrame::on_solve_button(wxCommandEvent const& evt) {
    int rowsNum = _rowsSpin->GetValue();
    int colsNum = _colsSpin->GetValue();
    BalanceMatrix m;
    vector<vector<int>> rows;
    for(int i = 0; i < rowsNum - 1; ++i) {
        vector<int> row;
        for(int j = 0; j < colsNum; ++j) {
            long value;
            bool parseStatus = _inputGrid->GetCellValue(i, j).ToLong(&value);
            if(!parseStatus) {
                wxMessageBox(
                    wxString::Format(wxT("Некорректное значение [%d-%d]"), i + 1, j + 1)
                );
                return;
            }
            row.push_back(value);
        }
        rows.push_back(row);
    }
    {
        vector<int> row;
        int const i = rowsNum - 1;
        for(int j = 0; j < colsNum - 1; ++j) {
            long value;
            bool parseStatus = _inputGrid->GetCellValue(i, j).ToLong(&value);
            if(!parseStatus) {
                wxMessageBox(
                    wxString::Format(wxT("Некорректное значение [%d-%d]"), i + 1, j + 1)
                );
                return;
            }
            row.push_back(value);
        }
        rows.push_back(row);
    }
    
    if(_inputGrid->GetCellValue(rowsNum - 1, colsNum - 1) != wxEmptyString) {
        wxMessageBox(wxString::Format(wxT("Значение в [%d-%d] проигнорировано"), rowsNum, colsNum));
    }
    
    m.set(rows);
    
    clear_notebooks();
    fill_notebook(m.solve(BalanceMatrix::Meth::NW), _NWBook);
    fill_notebook(m.solve(BalanceMatrix::Meth::Min), _MinBook);
}

class MyApp : public wxApp {
public:
    virtual bool OnInit() override;
};

bool MyApp::OnInit() {
    MyFrame* frame = new MyFrame("CO 45");
    frame->Show();
    return true;
}

IMPLEMENT_APP(MyApp)
