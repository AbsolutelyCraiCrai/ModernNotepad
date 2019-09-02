#pragma once
#include <Windows.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include <winrt/Windows.UI.Xaml.Input.h>
#include <winrt/Windows.UI.Text.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include "file.h"

using namespace std;
using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::UI::Text;

class ModernNotepad
{
public:
	ModernNotepad(HINSTANCE hInstance, int showWindow);
	int MakeWindow();
	void SetupXAML();
	void DockXAML();
	void Run();
	void ProcessArgs(int argc, WCHAR** argv);

private:
	HWND hwnd;
	HWND hwndXaml;
	HINSTANCE hInstance;
	int showWindow;
	DesktopWindowXamlSource xamlSource;
	BOOL changesMade = FALSE;
	BOOL allowClose = FALSE;
	std::string originalText = "";
	std::string fileName = "";

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK ActualWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void NewDocument();
	void OpenDocument();
	void LoadFile(ModernNotepadFile* file);
	void SaveDocumentAs();
	void SaveDocument();
	void WriteSettings();
	void ReadSettings();
	std::string GetEditBoxContent();
	ContentDialog CreateCloseConfirmDialog();
	void OnAboutMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnExitMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnClosingCloseDlg(Windows::Foundation::IInspectable const&, ContentDialogClosingEventArgs const &args);
	void OnNewDocumentConfirm(Windows::Foundation::IInspectable const&, ContentDialogClosingEventArgs const &args);
	void OnTextChanged(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnNewMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnOpenMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnSaveAsMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnSaveMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
	void OnFontMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args);
};

ModernNotepad* mainClass;

class Globals
{
public:
	Grid mainGrid;
	MenuBar menuBar;
	RichEditBox editBox;
};

Globals* globals;

typedef struct _MODERN_NOTEPAD_SETTINGS
{
	char Header[3];
	char FontFamily[33];
	int FontSize;
} MODERN_NOTEPAD_SETTINGS;