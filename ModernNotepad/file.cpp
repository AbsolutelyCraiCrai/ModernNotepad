#include "file.h"

ModernNotepadFile::ModernNotepadFile(string fileName, bool createNew)
{
	this->fileName = fileName;

	hFile = CreateFileA(
		fileName.c_str(),
		FILE_READ_ACCESS | FILE_WRITE_ACCESS,
		FILE_SHARE_READ,
		NULL,
		(createNew) ? CREATE_ALWAYS : OPEN_EXISTING,
		NULL,
		NULL
	);
}

ModernNotepadFile::~ModernNotepadFile()
{
	CloseHandle(hFile);
}

void ModernNotepadFile::WriteText(string text)
{
	if (!FileExists())
		return;

	const char* textA = text.c_str();

	DWORD written;
	WriteFile(
		hFile,
		textA,
		strlen(textA),
		&written,
		NULL
	);
}

void ModernNotepadFile::Flush()
{
	if (!FileExists())
		return;

	FlushFileBuffers(hFile);
}

string ModernNotepadFile::ReadText()
{
	if (!FileExists())
		return string("");

	LARGE_INTEGER fileSize;
	GetFileSizeEx(hFile, &fileSize);

	char* data = new char[fileSize.QuadPart + 1];

	DWORD read;
	ReadFile(
		hFile,
		data,
		fileSize.QuadPart,
		&read,
		NULL
	);

	data[read] = '\0';

	string data_s(data);
	delete[] data;

	return data_s;
}

void ModernNotepadFile::Delete()
{
	if (!FileExists())
		return;

	DeleteFileA(
		fileName.c_str()
	);

	CloseHandle(hFile);
	hFile = NULL;
}

bool ModernNotepadFile::FileExists()
{
	return (hFile != NULL && hFile != INVALID_HANDLE_VALUE);
}

HANDLE ModernNotepadFile::GetHandle()
{
	return hFile;
}