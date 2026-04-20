#pragma once
#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>
#include <string>
#include <vector>
#include <cctype>
#include <msclr/marshal_cppstd.h>

namespace TinyScanner {

    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace System::Collections::Generic;

    // ── Check Reserved Words ─────────────────────────────────────
    std::string checkKeyword(std::string word) {
        if (word == "if")     return "IF_KW";
        if (word == "then")   return "THEN_KW";
        if (word == "else")   return "ELSE_KW";
        if (word == "end")    return "END_KW";
        if (word == "repeat") return "REPEAT_KW";
        if (word == "until")  return "UNTIL_KW";
        if (word == "read")   return "READ_KW";
        if (word == "write")  return "WRITE_KW";
        return "ID";
    }

    // ── Scanner ──────────────────────────────────────────────────
    // Returns a list of {lexeme, token} pairs instead of printing
    std::vector<std::pair<std::string, std::string>> scanner(std::string input) {
        std::vector<std::pair<std::string, std::string>> tokens;
        int i = 0;

        while (i < (int)input.length()) {
            char c = input[i];

            // Ignore whitespace
            if (isspace(c)) { i++; continue; }

            std::string lexeme = "";

            // Identifier / Keyword
            if (isalpha(c)) {
                while (i < (int)input.length() && isalnum(input[i]))
                    lexeme += input[i++];
                tokens.push_back({ lexeme, checkKeyword(lexeme) });
            }

            // Number
            else if (isdigit(c)) {
                while (i < (int)input.length() && isdigit(input[i]))
                    lexeme += input[i++];
                tokens.push_back({ lexeme, "NUMBER" });
            }

            // String "..."
            else if (c == '"') {
                i++;
                std::string str = "";
                while (i < (int)input.length() && input[i] != '"')
                    str += input[i++];
                if (i < (int)input.length()) {
                    i++; // skip closing "
                    tokens.push_back({ "\"" + str + "\"", "STRING" });
                } else {
                    tokens.push_back({ "\"" + str, "ERROR: Unclosed string" });
                }
            }

            // Comment { ... }
            else if (c == '{') {
                i++;
                while (i < (int)input.length() && input[i] != '}') i++;
                if (i < (int)input.length()) i++; // skip }
                // comments are ignored — no token added
            }

            // Assignment :=
            else if (c == ':' && i + 1 < (int)input.length() && input[i + 1] == '=') {
                tokens.push_back({ ":=", "ASSIGNMENTOP" });
                i += 2;
            }

            // Arithmetic Operators
            else if (c == '+') { tokens.push_back({ "+", "ADDOP"  }); i++; }
            else if (c == '-') { tokens.push_back({ "-", "SUBOP"  }); i++; }
            else if (c == '*') { tokens.push_back({ "*", "MULOP"  }); i++; }
            else if (c == '/') { tokens.push_back({ "/", "DIVOP"  }); i++; }

            // Comparison Operators
            else if (c == '=' || c == '<') {
                tokens.push_back({ std::string(1, c), "COMPARISONOP" });
                i++;
            }

            // Symbols
            else if (c == ';') { tokens.push_back({ ";", "SEMICOLON"   }); i++; }
            else if (c == ',') { tokens.push_back({ ",", "COMMA"       }); i++; }
            else if (c == '(' || c == ')') {
                tokens.push_back({ std::string(1, c), "PUNCTUATION" });
                i++;
            }

            // Invalid
            else {
                tokens.push_back({ std::string(1, c), "INVALID" });
                i++;
            }
        }

        return tokens;
    }

    // ── Form1 ─────────────────────────────────────────────────────
    public ref class Form1 : public Form {
    private:
        Label^        lblTitle;
        TextBox^      txtCode;
        Button^       btnScan;
        DataGridView^ grid;

        void InitializeComponent() {
            this->Text            = "Tiny Language Scanner";
            this->Size            = Drawing::Size(520, 500);
            this->BackColor       = Color::FromArgb(240, 240, 240);
            this->Font            = gcnew Drawing::Font("Segoe UI", 9.5f);
            this->FormBorderStyle = Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox     = false;

            // Label
            lblTitle           = gcnew Label();
            lblTitle->Text     = "Enter The Code:";
            lblTitle->Location = Point(10, 10);
            lblTitle->AutoSize = true;

            // TextBox
            txtCode             = gcnew TextBox();
            txtCode->Multiline  = true;
            txtCode->ScrollBars = ScrollBars::Vertical;
            txtCode->Location   = Point(10, 32);
            txtCode->Size       = Drawing::Size(390, 90);
            txtCode->Font       = gcnew Drawing::Font("Courier New", 9.5f);
            txtCode->Text       = "if x=10 then\r\nx:=x+1;\r\nend";

            // Scan Button
            btnScan             = gcnew Button();
            btnScan->Text       = "Scan";
            btnScan->Location   = Point(410, 32);
            btnScan->Size       = Drawing::Size(88, 30);
            btnScan->BackColor  = Color::FromArgb(0, 120, 215);
            btnScan->ForeColor  = Color::White;
            btnScan->FlatStyle  = FlatStyle::Flat;
            btnScan->FlatAppearance->BorderSize = 0;
            btnScan->Cursor     = Cursors::Hand;
            btnScan->Click     += gcnew EventHandler(this, &Form1::OnScan);

            // DataGridView
            grid                        = gcnew DataGridView();
            grid->Location              = Point(10, 135);
            grid->Size                  = Drawing::Size(488, 320);
            grid->ReadOnly              = true;
            grid->AllowUserToAddRows    = false;
            grid->AllowUserToDeleteRows = false;
            grid->RowHeadersVisible     = false;
            grid->AutoSizeColumnsMode   = DataGridViewAutoSizeColumnsMode::Fill;
            grid->SelectionMode         = DataGridViewSelectionMode::FullRowSelect;

            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersDefaultCellStyle->BackColor  = Color::FromArgb(74, 144, 217);
            grid->ColumnHeadersDefaultCellStyle->ForeColor  = Color::White;
            grid->ColumnHeadersDefaultCellStyle->Font       = gcnew Drawing::Font("Segoe UI", 9.5f, FontStyle::Regular);
            grid->ColumnHeadersDefaultCellStyle->Alignment  = DataGridViewContentAlignment::MiddleCenter;
            grid->ColumnHeadersHeight   = 28;
            grid->GridColor             = Color::FromArgb(200, 210, 220);

            grid->DefaultCellStyle->BackColor              = Color::White;
            grid->DefaultCellStyle->ForeColor              = Color::Black;
            grid->DefaultCellStyle->SelectionBackColor     = Color::FromArgb(232, 240, 254);
            grid->DefaultCellStyle->SelectionForeColor     = Color::Black;
            grid->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(248, 250, 255);

            auto colLexeme          = gcnew DataGridViewTextBoxColumn();
            colLexeme->HeaderText   = "Lexeme";
            colLexeme->Name         = "Lexeme";
            colLexeme->FillWeight   = 45;

            auto colToken           = gcnew DataGridViewTextBoxColumn();
            colToken->HeaderText    = "Token";
            colToken->Name          = "Token";
            colToken->FillWeight    = 55;

            grid->Columns->Add(colLexeme);
            grid->Columns->Add(colToken);

            this->Controls->Add(lblTitle);
            this->Controls->Add(txtCode);
            this->Controls->Add(btnScan);
            this->Controls->Add(grid);
        }

        void OnScan(Object^ sender, EventArgs^ e) {
            grid->Rows->Clear();

            // Convert managed String^ to native std::string
            std::string input = msclr::interop::marshal_as<std::string>(txtCode->Text);

            auto tokens = scanner(input);

            for (auto& tok : tokens) {
                grid->Rows->Add(
                    gcnew String(tok.first.c_str()),
                    gcnew String(tok.second.c_str())
                );
            }
        }

    public:
        Form1() { InitializeComponent(); }
    };
}
