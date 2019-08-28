#include "notepad.h"

//
// Entry point
//
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR cmdLine, int show)
{
	mainClass = new ModernNotepad(hInstance, show);
	int result = mainClass->MakeWindow();

	if (result != 0)
		return result;

	mainClass->SetupXAML();
	mainClass->DockXAML();
	mainClass->Run();
	return 0;
}

ModernNotepad::ModernNotepad(HINSTANCE hInstance, int showWindow)
{
	this->hInstance = hInstance;
	this->showWindow = showWindow;
}

int ModernNotepad::MakeWindow()
{
	//
	// Setup window class and handle
	//
	WNDCLASSEX WndClass = {};

	WndClass.cbSize = sizeof(WNDCLASSEX);
	WndClass.hCursor = LoadCursor(this->hInstance, IDC_ARROW);
	WndClass.hIcon = NULL;
	WndClass.hInstance = this->hInstance;
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.lpfnWndProc = ModernNotepad::WindowProc;
	WndClass.lpszClassName = L"ModernNotepadWindow";

	if (RegisterClassEx(&WndClass) == NULL)
	{
		MessageBox(NULL, L"Windows registration failed!", L"Error", NULL);
		return 1;
	}

	this->hwnd = CreateWindow(
		L"ModernNotepadWindow",
		L"Modern Notepad",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		1000,
		800,
		NULL,
		NULL,
		this->hInstance,
		NULL
	);

	if (this->hwnd == NULL)
	{
		MessageBox(NULL, L"Failed to create window.", L"Error", NULL);
		return 1;
	}

	return 0;
}

void ModernNotepad::SetupXAML()
{
	//
	// Initialise WinRT
	//
	init_apartment(apartment_type::single_threaded);
	WindowsXamlManager xamlManager = WindowsXamlManager::InitializeForCurrentThread();

	//
	// Setup XAML island
	//
	auto interop = this->xamlSource.as<IDesktopWindowXamlSourceNative>();
	interop->AttachToWindow(this->hwnd);
	interop->get_WindowHandle(&this->hwndXaml);

	//
	// Setup globals class
	//
	globals = new Globals();

	//
	// Setup grid
	//
	RowDefinition row_top;
	row_top.Height(GridLength{ 40, GridUnitType::Pixel });
	globals->mainGrid.RowDefinitions().Append(row_top);

	RowDefinition row_main;
	row_main.Height(GridLength{ 1, GridUnitType::Star });
	globals->mainGrid.RowDefinitions().Append(row_main);

	//
	// Setup menu
	//
	MenuBarItem fileMenuItem;
	fileMenuItem.Title(L"File");
	MenuBarItem editMenuItem;
	editMenuItem.Title(L"Edit");
	MenuBarItem helpMenuItem;
	helpMenuItem.Title(L"Help");

	globals->menuBar.Items().Append(fileMenuItem);
	globals->menuBar.Items().Append(editMenuItem);
	globals->menuBar.Items().Append(helpMenuItem);

	MenuFlyoutItem newFileMenuItem;
	newFileMenuItem.Text(L"New");
	KeyboardAccelerator accelNew;
	accelNew.Key(Windows::System::VirtualKey::N);
	accelNew.Modifiers(Windows::System::VirtualKeyModifiers::Control);
	newFileMenuItem.KeyboardAccelerators().Append(accelNew);
	newFileMenuItem.Click({ this, &ModernNotepad::OnNewMenuItemClick });
	fileMenuItem.Items().Append(newFileMenuItem);

	MenuFlyoutItem openFileMenuItem;
	openFileMenuItem.Text(L"Open...");
	KeyboardAccelerator accelOpen;
	accelOpen.Key(Windows::System::VirtualKey::O);
	accelOpen.Modifiers(Windows::System::VirtualKeyModifiers::Control);
	openFileMenuItem.KeyboardAccelerators().Append(accelOpen);
	openFileMenuItem.Click({ this, &ModernNotepad::OnOpenMenuItemClick });
	fileMenuItem.Items().Append(openFileMenuItem);

	MenuFlyoutItem saveFileMenuItem;
	saveFileMenuItem.Text(L"Save");
	KeyboardAccelerator accelSave;
	accelSave.Key(Windows::System::VirtualKey::S);
	accelSave.Modifiers(Windows::System::VirtualKeyModifiers::Control);
	saveFileMenuItem.KeyboardAccelerators().Append(accelSave);
	saveFileMenuItem.Click({ this, &ModernNotepad::OnSaveMenuItemClick });
	fileMenuItem.Items().Append(saveFileMenuItem);

	MenuFlyoutItem saveAsFileMenuItem;
	saveAsFileMenuItem.Text(L"Save as...");
	saveAsFileMenuItem.Click({ this, &ModernNotepad::OnSaveAsMenuItemClick });
	fileMenuItem.Items().Append(saveAsFileMenuItem);

	MenuFlyoutSeparator fileSep;
	fileMenuItem.Items().Append(fileSep);

	MenuFlyoutItem exitFileMenuItem;
	exitFileMenuItem.Text(L"Exit");
	KeyboardAccelerator accelExit;
	accelExit.Key(Windows::System::VirtualKey::F4);
	accelExit.Modifiers(Windows::System::VirtualKeyModifiers::Menu);
	exitFileMenuItem.KeyboardAccelerators().Append(accelExit);
	exitFileMenuItem.Click({ this, &ModernNotepad::OnExitMenuItemClick });
	fileMenuItem.Items().Append(exitFileMenuItem);

	MenuFlyoutItem fontEditMenuItem;
	fontEditMenuItem.Text(L"Font...");
	KeyboardAccelerator accelFont;
	accelFont.Key(Windows::System::VirtualKey::F);
	accelFont.Modifiers(static_cast<Windows::System::VirtualKeyModifiers>(static_cast<int>(Windows::System::VirtualKeyModifiers::Control) | static_cast<int>(Windows::System::VirtualKeyModifiers::Menu)));
	fontEditMenuItem.KeyboardAccelerators().Append(accelFont);
	fontEditMenuItem.Click({ this, &ModernNotepad::OnFontMenuItemClick });
	editMenuItem.Items().Append(fontEditMenuItem);

	MenuFlyoutItem aboutHelpMenuItem;
	aboutHelpMenuItem.Text(L"About Modern Notepad...");
	aboutHelpMenuItem.Click({ this, &ModernNotepad::OnAboutMenuItemClick });
	helpMenuItem.Items().Append(aboutHelpMenuItem);

	//
	// Add menu bar
	//
	Grid::SetRow(globals->menuBar, 0);
	globals->mainGrid.Children().Append(globals->menuBar);

	//
	// Add edit box
	//
	globals->editBox.IsColorFontEnabled(false);
	globals->editBox.SelectionFlyout(nullptr);
	globals->editBox.ClipboardCopyFormat(RichEditClipboardFormat::PlainText);
	globals->editBox.DisabledFormattingAccelerators(DisabledFormattingAccelerators::All);
	globals->editBox.TextChanged({ this, &ModernNotepad::OnTextChanged });
	
	//
	// Set our originalText variable to the current contents of the edit box
	//
	this->originalText = this->GetEditBoxContent();

	Grid::SetRow(globals->editBox, 1);
	globals->mainGrid.Children().Append(globals->editBox);

	//
	// Add XAML grid to the XAML island
	//
	globals->mainGrid.UpdateLayout();
	this->xamlSource.Content(globals->mainGrid);
}

void ModernNotepad::DockXAML()
{
	RECT rcClient;

	//
	// Dock the XAML island to fill the entire window space
	//
	GetClientRect(this->hwnd, &rcClient);
	SetWindowPos(
		this->hwndXaml,
		0,
		0,
		0,
		rcClient.right - rcClient.left,
		rcClient.bottom - rcClient.top,
		SWP_SHOWWINDOW
	);
}

void ModernNotepad::Run()
{
	//
	// Show the window and run its message loop
	//
	UpdateWindow(this->hwnd);
	ShowWindow(this->hwnd, this->showWindow);

	this->ReadSettings();

	//
	// Set focus to XAML window
	//
	SetFocus(this->hwndXaml);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK ModernNotepad::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//
	// Redirect this callback to our non-static WindowProc
	//
	return mainClass->ActualWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ModernNotepad::ActualWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		if (hwnd == this->hwnd)
		{
			//
			// Dock XAML island to the main window
			//
			this->DockXAML();
		}
		return 0;
		break;
	case WM_CLOSE:
	{
		if (this->changesMade && !this->allowClose)
		{
			ContentDialog closeDlg = this->CreateCloseConfirmDialog();
			closeDlg.Closing({ this, &ModernNotepad::OnClosingCloseDlg });
			closeDlg.ShowAsync();
			return 0;
		}
		
		break;
	}
	case WM_SETFOCUS:
		if (hwnd == this->hwnd)
		{
			SetFocus(this->hwndXaml);
		}
		break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void ModernNotepad::NewDocument()
{
	if (this->changesMade)
	{
		ContentDialog closeDlg = this->CreateCloseConfirmDialog();
		closeDlg.Closing({ this, &ModernNotepad::OnNewDocumentConfirm });
		closeDlg.ShowAsync();
		return;
	}

	this->fileName = "";
	globals->editBox.Document().SetText(TextSetOptions::None, to_hstring(""));

	//
	// Set the original text to the edit box's version of it
	//
	this->originalText = this->GetEditBoxContent();
}

void ModernNotepad::OpenDocument()
{
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->hwnd;
	ofn.hInstance = this->hInstance;
	ofn.lpstrTitle = L"Open...";
	ofn.lpstrFilter = L"Text Document (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = (LPWSTR)malloc(1024 * sizeof(WCHAR));
	ofn.nMaxFile = 1024 * sizeof(WCHAR);
	ofn.Flags = OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = L"txt";

	wcscpy_s(ofn.lpstrFile, ofn.nMaxFile, L"");

	if (GetOpenFileName(&ofn))
	{
		char* mbFileName = new char[ofn.nMaxFile + 1];
		size_t dummy;
		wcstombs_s(&dummy, mbFileName, ofn.nMaxFile + 1, ofn.lpstrFile, ofn.nMaxFile);

		std::string fileName;
		fileName.append(mbFileName);
		delete[] mbFileName;
		free(ofn.lpstrFile);

		this->fileName = fileName;

		HANDLE hFile = CreateFileA(
			this->fileName.c_str(),
			FILE_READ_ACCESS,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			NULL,
			NULL
		);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			MessageBox(this->hwnd, L"There was an error trying to open the file.", L"Error", MB_ICONERROR);
			return;
		}

		LARGE_INTEGER fileSize;
		GetFileSizeEx(hFile, &fileSize);

		char* buffer = new char[fileSize.QuadPart + 1];

		DWORD read;
		ReadFile(
			hFile,
			buffer,
			fileSize.QuadPart,
			&read,
			NULL
		);

		buffer[read] = '\0';

		std::string buffer_s;
		buffer_s.append(buffer);

		CloseHandle(hFile);
		delete[] buffer;

		globals->editBox.Document().SetText(TextSetOptions::None, to_hstring(buffer_s));

		//
		// Set the original text to the edit box's version of it
		//
		this->originalText = this->GetEditBoxContent();

		//
		// Reset the changes flag
		//
		this->changesMade = FALSE;
		return;
	}

	free(ofn.lpstrFile);
}

void ModernNotepad::SaveDocumentAs()
{
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->hwnd;
	ofn.hInstance = this->hInstance;
	ofn.lpstrTitle = L"Save As...";
	ofn.lpstrFilter = L"Text Document (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile = (LPWSTR)malloc(1024 * sizeof(WCHAR));
	ofn.nMaxFile = 1024 * sizeof(WCHAR);
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = L"txt";

	wcscpy_s(ofn.lpstrFile, ofn.nMaxFile, L"New Text Document.txt");

	if (GetSaveFileName(&ofn))
	{
		char* mbFileName = new char[ofn.nMaxFile + 1];
		size_t dummy;
		wcstombs_s(&dummy, mbFileName, ofn.nMaxFile + 1, ofn.lpstrFile, ofn.nMaxFile);

		std::string fileName;
		fileName.append(mbFileName);
		delete[] mbFileName;
		free(ofn.lpstrFile);

		this->fileName = fileName;
		this->SaveDocument();
		return;
	}

	free(ofn.lpstrFile);
}

void ModernNotepad::SaveDocument()
{
	if (this->fileName.length() == 0)
	{
		this->SaveDocumentAs();
		return;
	}

	HANDLE hFile = CreateFileA(
		this->fileName.c_str(),
		FILE_READ_ACCESS | FILE_WRITE_ACCESS,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		NULL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(this->hwnd, L"There was an error trying to create the file.", L"Error", MB_ICONERROR);
		return;
	}

	this->originalText = this->GetEditBoxContent();
	const char* buffer = this->originalText.c_str();

	DWORD written;
	WriteFile(
		hFile,
		buffer,
		strlen(buffer) - 1, // Ignore the last character in a RichEditBox
		&written,
		NULL
	);

	if (written != strlen(buffer) - 1)
	{
		//
		// We should never have this issue, unless an access problem has occured.
		//
		MessageBox(this->hwnd, L"There was an error trying to write to the file.", L"Error", MB_ICONERROR);
	}

	CloseHandle(hFile);
	this->changesMade = FALSE;
}

void ModernNotepad::WriteSettings()
{
	MODERN_NOTEPAD_SETTINGS settings;
	strcpy_s(settings.Header, 3, "MN");
	strcpy_s(settings.FontFamily, 33, to_string(globals->editBox.FontFamily().Source()).c_str());
	settings.FontSize = static_cast<int>(globals->editBox.FontSize());

	HANDLE hFile = CreateFileA(
		"settings.dat",
		FILE_READ_ACCESS | FILE_WRITE_ACCESS,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		NULL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
		return;

	DWORD dummy;
	WriteFile(
		hFile,
		(LPVOID)&settings,
		sizeof(settings),
		&dummy,
		NULL
	);

	CloseHandle(hFile);
}

void ModernNotepad::ReadSettings()
{
	MODERN_NOTEPAD_SETTINGS settings;

	HANDLE hFile = CreateFileA(
		"settings.dat",
		FILE_READ_ACCESS,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		NULL,
		NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
		return;

	DWORD dummy;
	ReadFile(
		hFile,
		(LPVOID)&settings,
		sizeof(settings),
		&dummy,
		NULL
	);

	CloseHandle(hFile);

	if (strcmp(settings.Header, "MN") != 0)
		return;

	globals->editBox.FontFamily(Media::FontFamily(to_hstring(settings.FontFamily)));
	globals->editBox.FontSize(settings.FontSize);
}

std::string ModernNotepad::GetEditBoxContent()
{
	hstring text;
	globals->editBox.Document().GetText(TextGetOptions::None, text);
	return to_string(text);
}

ContentDialog ModernNotepad::CreateCloseConfirmDialog()
{
	ContentDialog closeDlg;
	closeDlg.Title(box_value(L"Do you want to save your changes?"));

	TextBlock closeMsg;
	closeMsg.Text(L"You have unsaved changes.\n\nChoose how to proceed.");
	closeDlg.Content(closeMsg);

	closeDlg.CloseButtonText(L"Cancel");
	closeDlg.PrimaryButtonText(L"Save changes");
	closeDlg.SecondaryButtonText(L"Don't save");
	closeDlg.XamlRoot(globals->mainGrid.XamlRoot());
	return closeDlg;
}

void ModernNotepad::OnAboutMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	//
	// Create a ContentDialog containing about info
	//
	ContentDialog aboutDlg;
	aboutDlg.Title(box_value(L"About Modern Notepad"));
	aboutDlg.CloseButtonText(L"Close");

	TextBlock aboutDesc;
	aboutDesc.Text(L"Modern Notepad\nCreated by John Parker");
	aboutDesc.TextWrapping(TextWrapping::WrapWholeWords);

	aboutDlg.Content(aboutDesc);
	aboutDlg.XamlRoot(globals->mainGrid.XamlRoot());
	aboutDlg.ShowAsync();
}

void ModernNotepad::OnExitMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	SendMessage(this->hwnd, WM_CLOSE, NULL, NULL);
}

void ModernNotepad::OnClosingCloseDlg(Windows::Foundation::IInspectable const&, ContentDialogClosingEventArgs const &args)
{
	if (args.Result() == ContentDialogResult::Secondary)
	{
		this->allowClose = TRUE;
		SendMessage(this->hwnd, WM_CLOSE, NULL, NULL);
	}
	else if (args.Result() == ContentDialogResult::Primary)
	{
		this->allowClose = TRUE;
		this->SaveDocument();
		SendMessage(this->hwnd, WM_CLOSE, NULL, NULL);
	}
}

void ModernNotepad::OnNewDocumentConfirm(Windows::Foundation::IInspectable const&, ContentDialogClosingEventArgs const &args)
{
	if (args.Result() == ContentDialogResult::Secondary)
	{
		this->changesMade = FALSE;
		this->NewDocument();
	}
	else if (args.Result() == ContentDialogResult::Primary)
	{
		this->SaveDocument();
		this->NewDocument();
	}
}

void ModernNotepad::OnTextChanged(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	std::string ourText = this->GetEditBoxContent();

	if (ourText != this->originalText)
	{
		this->changesMade = TRUE;
	}
	else
	{
		this->changesMade = FALSE;
	}
}

void ModernNotepad::OnNewMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	this->NewDocument();
}

void ModernNotepad::OnOpenMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	this->OpenDocument();
}

void ModernNotepad::OnSaveAsMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	this->SaveDocumentAs();
}

void ModernNotepad::OnSaveMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	this->SaveDocument();
}

void ModernNotepad::OnFontMenuItemClick(Windows::Foundation::IInspectable const&, RoutedEventArgs const &args)
{
	LOGFONT logFont;
	CHOOSEFONT chooseFont = { 0 };
	chooseFont.lStructSize = sizeof(CHOOSEFONT);
	chooseFont.hwndOwner = this->hwnd;
	chooseFont.hInstance = this->hInstance;
	chooseFont.lpLogFont = &logFont;
	chooseFont.Flags = CF_FORCEFONTEXIST | CF_NOSCRIPTSEL;
	
	if (ChooseFont(&chooseFont))
	{
		char* mbFontFamily = new char[lstrlenW(logFont.lfFaceName) + 1];
		size_t dummy;
		wcstombs_s(&dummy, mbFontFamily, lstrlenW(logFont.lfFaceName) + 1, logFont.lfFaceName, lstrlenW(logFont.lfFaceName));

		std::string fontFamily;
		fontFamily.append(mbFontFamily);
		int fontSize = chooseFont.iPointSize / 10;

		globals->editBox.FontFamily(Media::FontFamily(to_hstring(fontFamily)));
		globals->editBox.FontSize(fontSize);

		this->WriteSettings();
	}
}