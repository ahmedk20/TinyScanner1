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

    // ── Token ────────────────────────────────────────────────────
    struct Token {
        std::string lexeme;
        std::string type;
        int         line;
    };

    // ── Reserved Words ───────────────────────────────────────────
    inline std::string checkKeyword(const std::string& word) {
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

    // ── Scanner (lexical analysis with line tracking) ────────────
    inline std::vector<Token> scanner(const std::string& input) {
        std::vector<Token> tokens;
        int i = 0;
        int line = 1;

        while (i < (int)input.length()) {
            char c = input[i];

            // newline → bump line counter
            if (c == '\n') { line++; i++; continue; }
            // other whitespace
            if (isspace((unsigned char)c)) { i++; continue; }

            std::string lexeme;

            // Identifier / Keyword
            if (isalpha((unsigned char)c)) {
                int startLine = line;
                while (i < (int)input.length() && isalnum((unsigned char)input[i]))
                    lexeme += input[i++];
                tokens.push_back({ lexeme, checkKeyword(lexeme), startLine });
            }
            // Number
            else if (isdigit((unsigned char)c)) {
                int startLine = line;
                while (i < (int)input.length() && isdigit((unsigned char)input[i]))
                    lexeme += input[i++];
                tokens.push_back({ lexeme, "NUMBER", startLine });
            }
            // String "..."
            else if (c == '"') {
                int startLine = line;
                i++;
                std::string str;
                while (i < (int)input.length() && input[i] != '"') {
                    if (input[i] == '\n') line++;
                    str += input[i++];
                }
                if (i < (int)input.length()) {
                    i++; // skip closing "
                    tokens.push_back({ "\"" + str + "\"", "STRING", startLine });
                } else {
                    tokens.push_back({ "\"" + str, "ERROR_UNCLOSED_STRING", startLine });
                }
            }
            // Comment { ... }
            else if (c == '{') {
                i++;
                while (i < (int)input.length() && input[i] != '}') {
                    if (input[i] == '\n') line++;
                    i++;
                }
                if (i < (int)input.length()) i++; // skip }
            }
            // Assignment :=
            else if (c == ':' && i + 1 < (int)input.length() && input[i + 1] == '=') {
                tokens.push_back({ ":=", "ASSIGNMENTOP", line });
                i += 2;
            }
            // Arithmetic operators
            else if (c == '+') { tokens.push_back({ "+", "ADDOP", line }); i++; }
            else if (c == '-') { tokens.push_back({ "-", "SUBOP", line }); i++; }
            else if (c == '*') { tokens.push_back({ "*", "MULOP", line }); i++; }
            else if (c == '/') { tokens.push_back({ "/", "DIVOP", line }); i++; }
            // Comparison / equality
            else if (c == '<') { tokens.push_back({ "<", "LESSTHAN",    line }); i++; }
            else if (c == '>') { tokens.push_back({ ">", "GREATERTHAN", line }); i++; }
            else if (c == '=') { tokens.push_back({ "=", "EQUAL",       line }); i++; }
            // Symbols
            else if (c == ';') { tokens.push_back({ ";", "SEMICOLON", line }); i++; }
            else if (c == ',') { tokens.push_back({ ",", "COMMA",     line }); i++; }
            else if (c == '(' || c == ')') {
                tokens.push_back({ std::string(1, c), "PUNCTUATION", line });
                i++;
            }
            // Anything else → invalid character (lexical error)
            else {
                tokens.push_back({ std::string(1, c), "INVALID", line });
                i++;
            }
        }

        return tokens;
    }

    // ── Parser (syntax analysis — recursive descent) ─────────────
    // Implements the LL(1) grammar after left-recursion elimination
    // and left-factoring (see Grammar.md).
    class Parser {
    public:
        Parser(const std::vector<Token>& t) : tokens_(t), pos_(0) {}

        std::vector<std::string> parse() {
            errors_.clear();
            if (tokens_.empty()) return errors_;     // nothing to parse
            parseP();
            if (pos_ < (int)tokens_.size()) {
                recordError("unexpected token '" + cur().lexeme +
                            "' after end of program");
            }
            return errors_;
        }

    private:
        const std::vector<Token>& tokens_;
        int                       pos_;
        std::vector<std::string>  errors_;

        // ── helpers ──
        const Token& cur() {
            static Token eof;
            if (pos_ < (int)tokens_.size()) return tokens_[pos_];
            int ln = tokens_.empty() ? 1 : tokens_.back().line;
            eof = { "<EOF>", "EOF", ln };
            return eof;
        }
        bool atEnd()                       { return pos_ >= (int)tokens_.size(); }
        bool check  (const std::string& t) { return cur().type   == t; }
        bool checkLx(const std::string& l) { return cur().lexeme == l; }
        void advance()                     { if (!atEnd()) pos_++; }

        void recordError(const std::string& msg) {
            errors_.push_back("[Syntax Error] Line " +
                              std::to_string(cur().line) + ": " + msg);
        }

        void expectType(const std::string& type, const std::string& display) {
            if (check(type)) { advance(); return; }
            recordError("expected " + display + " but found '" + cur().lexeme + "'");
        }
        void expectLex(const std::string& lex) {
            if (checkLx(lex)) { advance(); return; }
            recordError("expected '" + lex + "' but found '" + cur().lexeme + "'");
        }

        // FIRST(S) — used to decide whether P' continues
        bool firstS() {
            return checkLx("if") || checkLx("repeat") || check("ID")
                || checkLx("read") || checkLx("write");
        }

        // P  -> S ; P'         P' -> P | ε
        void parseP() {
            parseS();
            expectLex(";");
            if (firstS()) parseP();
        }

        // S  -> F | R | A | K | W
        void parseS() {
            if      (checkLx("if"))     parseF();
            else if (checkLx("repeat")) parseR();
            else if (check  ("ID"))     parseA();
            else if (checkLx("read"))   parseK();
            else if (checkLx("write"))  parseW();
            else {
                recordError("expected statement (if / repeat / id / read / write)"
                            " but found '" + cur().lexeme + "'");
                if (!atEnd()) advance();   // skip the offending token
            }
        }

        // F  -> if C then P O
        void parseF() {
            expectLex("if");
            parseC();
            expectLex("then");
            parseP();
            parseO();
        }

        // O  -> end | else P end
        void parseO() {
            if      (checkLx("end"))  { advance(); }
            else if (checkLx("else")) { advance(); parseP(); expectLex("end"); }
            else recordError("expected 'end' or 'else' but found '" +
                             cur().lexeme + "'");
        }

        // R  -> repeat P until C
        void parseR() {
            expectLex("repeat");
            parseP();
            expectLex("until");
            parseC();
        }

        // C  -> E C'           C' -> < E | = E
        void parseC() {
            parseE();
            if (checkLx("<") || checkLx("=")) {
                advance();
                parseE();
            } else {
                recordError("expected '<' or '=' in condition but found '" +
                            cur().lexeme + "'");
            }
        }

        // E  -> T E'           E' -> H E | ε
        void parseE() {
            parseT();
            if (checkLx("+") || checkLx("-") ||
                checkLx("*") || checkLx("/")) {
                advance();
                parseE();
            }
        }

        // T  -> D | N    (after scanning, D = ID, N = NUMBER)
        void parseT() {
            if (check("ID") || check("NUMBER")) advance();
            else recordError("expected identifier or number but found '" +
                             cur().lexeme + "'");
        }

        // A  -> D := E
        void parseA() {
            expectType("ID", "identifier");
            expectType("ASSIGNMENTOP", "':='");
            parseE();
        }

        // K  -> read D
        void parseK() {
            expectLex("read");
            expectType("ID", "identifier");
        }

        // W  -> write W'      W' -> D | "D"
        void parseW() {
            expectLex("write");
            if (check("ID") || check("STRING")) advance();
            else recordError("expected identifier or string after 'write'"
                             " but found '" + cur().lexeme + "'");
        }
    };

    // ── Form1 ─────────────────────────────────────────────────────
    public ref class Form1 : public Form {
    private:
        Label^        lblTitle;
        Label^        lblTokens;
        Label^        lblErrors;
        TextBox^      txtCode;
        Button^       btnAnalyze;
        DataGridView^ grid;
        ListBox^      lstErrors;

        void InitializeComponent() {
            this->Text            = "Tiny Language Scanner + Parser";
            this->Size            = Drawing::Size(780, 760);
            this->BackColor       = Color::FromArgb(240, 240, 240);
            this->Font            = gcnew Drawing::Font("Segoe UI", 9.5f);
            this->FormBorderStyle = Windows::Forms::FormBorderStyle::FixedSingle;
            this->MaximizeBox     = false;

            // ── Title label ──
            lblTitle           = gcnew Label();
            lblTitle->Text     = "Enter the Tiny code:";
            lblTitle->Location = Point(10, 10);
            lblTitle->AutoSize = true;

            // ── Code TextBox ──
            txtCode             = gcnew TextBox();
            txtCode->Multiline  = true;
            txtCode->ScrollBars = ScrollBars::Vertical;
            txtCode->Location   = Point(10, 32);
            txtCode->Size       = Drawing::Size(620, 150);
            txtCode->Font       = gcnew Drawing::Font("Courier New", 9.5f);
            txtCode->Text       =
                "x := 10;\r\n"
                "if x = 10 then\r\n"
                "  write x;\r\n"
                "end;";

            // ── Analyze Button ──
            btnAnalyze            = gcnew Button();
            btnAnalyze->Text      = "Analyze";
            btnAnalyze->Location  = Point(640, 32);
            btnAnalyze->Size      = Drawing::Size(120, 40);
            btnAnalyze->BackColor = Color::FromArgb(0, 120, 215);
            btnAnalyze->ForeColor = Color::White;
            btnAnalyze->FlatStyle = FlatStyle::Flat;
            btnAnalyze->FlatAppearance->BorderSize = 0;
            btnAnalyze->Cursor    = Cursors::Hand;
            btnAnalyze->Click    += gcnew EventHandler(this, &Form1::OnAnalyze);

            // ── Tokens label ──
            lblTokens           = gcnew Label();
            lblTokens->Text     = "Tokens:";
            lblTokens->Location = Point(10, 195);
            lblTokens->AutoSize = true;

            // ── Tokens DataGridView ──
            grid                        = gcnew DataGridView();
            grid->Location              = Point(10, 215);
            grid->Size                  = Drawing::Size(750, 260);
            grid->ReadOnly              = true;
            grid->AllowUserToAddRows    = false;
            grid->AllowUserToDeleteRows = false;
            grid->RowHeadersVisible     = false;
            grid->AutoSizeColumnsMode   = DataGridViewAutoSizeColumnsMode::Fill;
            grid->SelectionMode         = DataGridViewSelectionMode::FullRowSelect;
            grid->EnableHeadersVisualStyles = false;
            grid->ColumnHeadersDefaultCellStyle->BackColor = Color::FromArgb(74, 144, 217);
            grid->ColumnHeadersDefaultCellStyle->ForeColor = Color::White;
            grid->ColumnHeadersDefaultCellStyle->Font      =
                gcnew Drawing::Font("Segoe UI", 9.5f, FontStyle::Regular);
            grid->ColumnHeadersDefaultCellStyle->Alignment =
                DataGridViewContentAlignment::MiddleCenter;
            grid->ColumnHeadersHeight = 28;
            grid->GridColor           = Color::FromArgb(200, 210, 220);
            grid->DefaultCellStyle->BackColor          = Color::White;
            grid->DefaultCellStyle->ForeColor          = Color::Black;
            grid->DefaultCellStyle->SelectionBackColor = Color::FromArgb(232, 240, 254);
            grid->DefaultCellStyle->SelectionForeColor = Color::Black;
            grid->AlternatingRowsDefaultCellStyle->BackColor =
                Color::FromArgb(248, 250, 255);

            auto colLexeme        = gcnew DataGridViewTextBoxColumn();
            colLexeme->HeaderText = "Lexeme";
            colLexeme->Name       = "Lexeme";
            colLexeme->FillWeight = 40;

            auto colToken         = gcnew DataGridViewTextBoxColumn();
            colToken->HeaderText  = "Token";
            colToken->Name        = "Token";
            colToken->FillWeight  = 45;

            auto colLine          = gcnew DataGridViewTextBoxColumn();
            colLine->HeaderText   = "Line";
            colLine->Name         = "Line";
            colLine->FillWeight   = 15;
            colLine->DefaultCellStyle->Alignment =
                DataGridViewContentAlignment::MiddleCenter;

            grid->Columns->Add(colLexeme);
            grid->Columns->Add(colToken);
            grid->Columns->Add(colLine);

            // ── Errors label ──
            lblErrors           = gcnew Label();
            lblErrors->Text     = "Errors / Result:";
            lblErrors->Location = Point(10, 485);
            lblErrors->AutoSize = true;

            // ── Errors ListBox ──
            lstErrors           = gcnew ListBox();
            lstErrors->Location = Point(10, 508);
            lstErrors->Size     = Drawing::Size(750, 200);
            lstErrors->Font     = gcnew Drawing::Font("Consolas", 9.5f);
            lstErrors->BackColor = Color::FromArgb(252, 252, 252);
            lstErrors->HorizontalScrollbar = true;

            this->Controls->Add(lblTitle);
            this->Controls->Add(txtCode);
            this->Controls->Add(btnAnalyze);
            this->Controls->Add(lblTokens);
            this->Controls->Add(grid);
            this->Controls->Add(lblErrors);
            this->Controls->Add(lstErrors);
        }

        void OnAnalyze(Object^ sender, EventArgs^ e) {
            grid->Rows->Clear();
            lstErrors->Items->Clear();

            std::string input = msclr::interop::marshal_as<std::string>(txtCode->Text);

            // ── Phase 1: Lexical analysis ──
            auto tokens = scanner(input);

            std::vector<std::string> lexErrors;
            for (auto& tok : tokens) {
                grid->Rows->Add(
                    gcnew String(tok.lexeme.c_str()),
                    gcnew String(tok.type.c_str()),
                    gcnew String(std::to_string(tok.line).c_str())
                );
                if (tok.type == "INVALID") {
                    lexErrors.push_back("[Lexical Error] Line " +
                        std::to_string(tok.line) +
                        ": invalid character '" + tok.lexeme + "'");
                } else if (tok.type == "ERROR_UNCLOSED_STRING") {
                    lexErrors.push_back("[Lexical Error] Line " +
                        std::to_string(tok.line) +
                        ": unclosed string literal");
                }
            }
            for (auto& err : lexErrors)
                lstErrors->Items->Add(gcnew String(err.c_str()));

            // ── Phase 2: Syntax analysis ──
            // Strip the lex-error tokens so the parser only sees valid lexemes.
            std::vector<Token> clean;
            clean.reserve(tokens.size());
            for (auto& t : tokens) {
                if (t.type != "INVALID" && t.type != "ERROR_UNCLOSED_STRING")
                    clean.push_back(t);
            }

            Parser parser(clean);
            auto syntaxErrors = parser.parse();
            for (auto& err : syntaxErrors)
                lstErrors->Items->Add(gcnew String(err.c_str()));

            // ── Final verdict ──
            if (lexErrors.empty() && syntaxErrors.empty()) {
                lstErrors->Items->Add(
                    gcnew String("[OK] Program is lexically and syntactically valid."));
            } else {
                lstErrors->Items->Add(gcnew String((
                    "---- Summary: " + std::to_string(lexErrors.size()) +
                    " lexical, " + std::to_string(syntaxErrors.size()) +
                    " syntax error(s) ----").c_str()));
            }
        }

    public:
        Form1() { InitializeComponent(); }
    };
}
