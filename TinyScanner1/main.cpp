/*
 * ================================================================
 *  TinyScanner — main.cpp
 *  Entry point for the Windows Forms GUI application
 * ================================================================
 *  Visual Studio setup steps:
 *
 *  1. Create new project:
 *       File > New > Project
 *       Search "CLR" > select "CLR Empty Project"
 *       Name: TinyScanner
 *
 *  2. Add files:
 *       - Add this file (main.cpp)   to Source Files
 *       - Add Form1.h                to Header Files
 *
 *  3. Project Properties:
 *       Configuration Properties > General
 *         > Common Language Runtime Support = /clr
 *       Linker > System
 *         > SubSystem = Windows (/SUBSYSTEM:WINDOWS)
 *       Linker > Advanced
 *         > Entry Point = main
 *
 *  4. Press F5 to build and run.
 * ================================================================
 */

 #include "Form1.h"

 using namespace TinyScanner;
 
 [STAThread]
 int main(array<String^>^ args) {
     Application::EnableVisualStyles();
     Application::SetCompatibleTextRenderingDefault(false);
     Application::Run(gcnew Form1());
     return 0;
 }
 