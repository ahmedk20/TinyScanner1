#pragma once
#using <System.dll>
#using <System.Windows.Forms.dll>
#using <System.Drawing.dll>
/*
 * ================================================================
 *  Tiny Language Scanner — Windows Forms GUI
 *  Visual Studio C++/CLR project
 *
 *  How to create the project:
 *    1. File > New > Project > CLR > Windows Forms App (.NET)
 *    2. Replace the generated Form1.h with this file
 *    3. Build and Run (F5)
 * ================================================================
 */





namespace TinyScanner {




    using namespace System;
    using namespace System::Windows::Forms;
    using namespace System::Drawing;
    using namespace System::Collections::Generic;

    // ── Token types ──────────────────────────────────────────────
    public enum class TokenType {
        ReservedWord,
        Identifier,
        Number,
        AssignOp,
        CompareOp,
        ArithOp,
        SpecialSymbol,
        EndOfFile,
        Error
    };

    public ref struct Token {
        String^ lexeme;
        TokenType type;
        Token(String^ l, TokenType t) : lexeme(l), type(t) {}
    };

    // ── Scanner (DFA-based) ───────────────────────────────────────
    public ref class Scanner {
    private:
        String^ src;
        int pos;

        static Dictionary<String^, TokenType>^ keywords;

        static Scanner() {
            keywords = gcnew Dictionary<String^, TokenType>();
            keywords->Add("if", TokenType::ReservedWord);
            keywords->Add("then", TokenType::ReservedWord);
            keywords->Add("else", TokenType::ReservedWord);
            keywords->Add("end", TokenType::ReservedWord);
            keywords->Add("repeat", TokenType::ReservedWord);
            keywords->Add("until", TokenType::ReservedWord);
            keywords->Add("read", TokenType::ReservedWord);
            keywords->Add("write", TokenType::ReservedWord);
        }

        wchar_t Advance() {
            if (pos >= src->Length) return L'\0';
            return src[pos++];
        }
        void Retreat() { if (pos > 0) pos--; }

    public:
        Scanner(String^ source) : src(source), pos(0) {}

        static String^ TokenTypeName(TokenType t) {
            switch (t) {
            case TokenType::ReservedWord:  return "Reserved Word";
            case TokenType::Identifier:    return "Identifier";
            case TokenType::Number:        return "Number";
            case TokenType::AssignOp:      return "Assignment Operator";
            case TokenType::CompareOp:     return "Comparison Operator";
            case TokenType::ArithOp:       return "Arithmetic Operator";
            case TokenType::SpecialSymbol: return "Special Symbol";
            case TokenType::EndOfFile:     return "EOF";
            default:                       return "Error";
            }
        }

        List<Token^>^ Tokenize() {
            auto result = gcnew List<Token^>();
            Token^ tok = nullptr;

            while ((tok = NextToken())->type != TokenType::EndOfFile)
                result->Add(tok);

            return result;
        }

    private:
        Token^ NextToken() {
            enum State { START, INNUM, INID, INASSIGN, INCOMMENT, DONE };
            State state = START;
            String^ lexeme = "";

            while (state != DONE) {
                wchar_t c = Advance();
                bool save = true;

                switch (state) {
                case START:
                    if (Char::IsWhiteSpace(c)) {
                        save = false;
                        if (c == L'\0') { state = DONE; }
                    }
                    else if (Char::IsDigit(c))  state = INNUM;
                    else if (Char::IsLetter(c)) state = INID;
                    else if (c == L':')          state = INASSIGN;
                    else if (c == L'{') { save = false; state = INCOMMENT; }
                    else if (c == L'\0') { save = false; state = DONE; return gcnew Token("", TokenType::EndOfFile); }
                    else {
                        state = DONE;
                        if (c == L'=' || c == L'<')
                            return gcnew Token(Char::ToString(c), TokenType::CompareOp);
                        if (c == L'+' || c == L'-' || c == L'*' || c == L'/')
                            return gcnew Token(Char::ToString(c), TokenType::ArithOp);
                        if (c == L'(' || c == L')' || c == L';')
                            return gcnew Token(Char::ToString(c), TokenType::SpecialSymbol);
                        return gcnew Token(Char::ToString(c), TokenType::Error);
                    }
                    break;

                case INNUM:
                    if (!Char::IsDigit(c)) {
                        Retreat(); save = false; state = DONE;
                        return gcnew Token(lexeme, TokenType::Number);
                    }
                    break;

                case INID:
                    if (!Char::IsLetterOrDigit(c)) {
                        Retreat(); save = false; state = DONE;
                        TokenType t;
                        if (keywords->TryGetValue(lexeme->ToLower(), t))
                            return gcnew Token(lexeme, t);
                        return gcnew Token(lexeme, TokenType::Identifier);
                    }
                    break;

                case INASSIGN:
                    state = DONE;
                    if (c == L'=')
                        return gcnew Token(":=", TokenType::AssignOp);
                    Retreat(); save = false;
                    return gcnew Token(":", TokenType::Error);

                case INCOMMENT:
                    save = false;
                    if (c == L'\0') return gcnew Token("{unterminated}", TokenType::Error);
                    if (c == L'}')  state = START;
                    break;

                default: break;
                }

                if (save) lexeme += c;
            }
            return gcnew Token(lexeme, TokenType::EndOfFile);
        }
    };

    // ── Form1 ─────────────────────────────────────────────────────
    public ref class Form1 : public Form {
    private:
        Label^      lblTitle;
        TextBox^    txtCode;
        Button^     btnUpload;
        DataGridView^ grid;

        void InitializeComponent() {
            this->Text    = "Tiny Language Scanner";
            this->Size    = Drawing::Size(520, 500);
            this->BackColor = Color::FromArgb(240, 240, 240);
            this->Font    = gcnew Drawing::Font("Segoe UI", 9.5f);
            this->FormBorderStyle = Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox = false;

            // ── Label ──
            lblTitle = gcnew Label();
            lblTitle->Text   = "Enter The Code:";
            lblTitle->Location = Point(10, 10);
            lblTitle->AutoSize = true;

            // ── TextBox ──
            txtCode = gcnew TextBox();
            txtCode->Multiline  = true;
            txtCode->ScrollBars = ScrollBars::Vertical;
            txtCode->Location   = Point(10, 32);
            txtCode->Size       = Drawing::Size(390, 90);
            txtCode->Font       = gcnew Drawing::Font("Courier New", 9.5f);
            txtCode->Text       = "if x=10 then\r\nx:=x+1;\r\nend";

            // ── Upload Button ──
            btnUpload = gcnew Button();
            btnUpload->Text      = "Upload";
            btnUpload->Location  = Point(410, 32);
            btnUpload->Size      = Drawing::Size(88, 30);
            btnUpload->BackColor = Color::FromArgb(0, 120, 215);
            btnUpload->ForeColor = Color::White;
            btnUpload->FlatStyle = FlatStyle::Flat;
            btnUpload->FlatAppearance->BorderSize = 0;
            btnUpload->Cursor    = Cursors::Hand;
            btnUpload->Click    += gcnew EventHandler(this, &Form1::OnUpload);

            // ── DataGridView ──
            grid = gcnew DataGridView();
            grid->Location      = Point(10, 135);
            grid->Size          = Drawing::Size(488, 320);
            grid->ReadOnly      = true;
            grid->AllowUserToAddRows    = false;
            grid->AllowUserToDeleteRows = false;
            grid->RowHeadersVisible     = false;
            grid->AutoSizeColumnsMode   = DataGridViewAutoSizeColumnsMode::Fill;
            grid->SelectionMode         = DataGridViewSelectionMode::FullRowSelect;

            // Header style
            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(74, 144, 217);
            grid->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
            grid->ColumnHeadersDefaultCellStyle->Font      = gcnew Drawing::Font("Segoe UI", 9.5f, FontStyle::Regular);
            grid->ColumnHeadersDefaultCellStyle->Alignment = DataGridViewContentAlignment::MiddleCenter;
            grid->ColumnHeadersHeight    = 28;
            grid->GridColor              = Color::FromArgb(200, 210, 220);

            // Row style
            grid->DefaultCellStyle->BackColor = Color::White;
            grid->DefaultCellStyle->ForeColor = Color::Black;
            grid->DefaultCellStyle->SelectionBackColor = Color::FromArgb(232, 240, 254);
            grid->DefaultCellStyle->SelectionForeColor = Color::Black;
            grid->AlternatingRowsDefaultCellStyle->BackColor = Color::FromArgb(248, 250, 255);

            // Columns
            auto colLexeme = gcnew DataGridViewTextBoxColumn();
            colLexeme->HeaderText = "Lexeme";
            colLexeme->Name       = "Lexeme";
            colLexeme->FillWeight = 45;

            auto colToken = gcnew DataGridViewTextBoxColumn();
            colToken->HeaderText = "Token";
            colToken->Name       = "Token";
            colToken->FillWeight = 55;

            grid->Columns->Add(colLexeme);
            grid->Columns->Add(colToken);

            // Add controls
            this->Controls->Add(lblTitle);
            this->Controls->Add(txtCode);
            this->Controls->Add(btnUpload);
            this->Controls->Add(grid);
        }

        void OnUpload(Object^ sender, EventArgs^ e) {
            grid->Rows->Clear();
            auto sc     = gcnew Scanner(txtCode->Text);
            auto tokens = sc->Tokenize();
            for each (Token^ tok in tokens) {
                grid->Rows->Add(tok->lexeme, Scanner::TokenTypeName(tok->type));
            }
        }

    public:
        Form1() { InitializeComponent(); }
    };
}

