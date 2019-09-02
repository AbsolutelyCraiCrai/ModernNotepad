#pragma once
#include <Windows.h>
#include <string>

using namespace std;

class ModernNotepadFile
{
public:
	ModernNotepadFile(string fileName, bool createNew);
	~ModernNotepadFile();
	void WriteText(string text);
	void Flush();
	string ReadText();
	void Delete();
	bool FileExists();
	HANDLE GetHandle();

private:
	HANDLE hFile;
	string fileName;
};