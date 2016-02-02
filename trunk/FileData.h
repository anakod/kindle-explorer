#include <vector>
#include <fstream>
#include <string>
#include <cerrno>
#include <algorithm>
#include <sys/stat.h>
#include "tinydir.h"

#ifndef FILE_ITEM_H
#define FILE_ITEM_H

using namespace std;

typedef struct
{
    string name;
    bool dir;
} FileItem;

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef WINDOWS
    #include <windows.h>
    #include <Shellapi.h>
    #define GetCurrentDir _getcwd
    #undef MessageBox
    #undef DeleteFile
    #undef CopyFile
    #define PATH_SEPARATOR '\\'
    string GetAppPath()
    {
        char buffer[MAX_PATH];
        GetModuleFileName( NULL, buffer, MAX_PATH );
        *strrchr(buffer, PATH_SEPARATOR) = 0;
        return string(buffer);
    }
    void Execute(string name, string workPath = "")
    {
        ShellExecute(NULL, (LPCTSTR)"open", name.c_str(), 0, 0, SW_SHOWNORMAL);
    }
    void CreateFolder(string name, int attr)
    {
        CreateDirectory(name.c_str(), NULL);
    }
    bool DirectoryExists(string path)
    {
      DWORD dwAttrib = GetFileAttributes(path.c_str());

      return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    }
    void OpenKeyboard() {};
    void CloseKeyboard() {}
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
    #define PATH_SEPARATOR '/'
    std::string GetAppPath()
    {
        char buffer[2048];
        ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
        if (len != -1)
        {
            buffer[len] = '\0';
            *strrchr(buffer, PATH_SEPARATOR) = 0;
            return std::string(buffer);
        }
    }

    void Execute(string name, string workPath = "")
    {
        string work;

        char curdir[2048];
        GetCurrentDir(curdir, sizeof(curdir));

        if (name[0] != '/')
            name = "./" + name;
        int space = name.find(' ');
        if (space != string::npos)
            work = name.substr(0, space);
        else
            work = name;

        int dirpos = work.find_last_of(PATH_SEPARATOR);
        if (dirpos != string::npos)
            work = name.substr(0, dirpos);

        if (workPath.length() > 0)
            work = workPath;

        if (work.length() > 0)
        {
            chdir(work.c_str());
            system(("(cd " + work + "; exec " + name + ")").c_str());
            chdir(curdir);
        }
        else
        {
            system((name + " &").c_str());
        }
    }

    void CreateFolder(string name, int attr)
    {
        mkdir(name.c_str(), attr);
    }

    bool DirectoryExists(string path)
    {
        DIR *pDir;
        bool bExists = false;

        pDir = opendir (path.c_str());

        if (pDir != NULL)
        {
            bExists = true;
            closedir(pDir);
        }

        return bExists;
    }
    void OpenKeyboard()
    {
      switch(fork())
      {
        case 0:
          {
            execlp("/bin/sh", "sh", "-c", "/usr/bin/lipc-set-prop -s com.lab126.keyboard open net.fabiszewski.leafpad:Abc:1", NULL);
          }
        case -1:
          perror("Failed to open keyboard\n");
          exit(1);
      }
    }

    void CloseKeyboard()
    {
      switch(fork())
      {
        case 0:
          {
            execlp("/bin/sh", "sh", "-c", "/usr/bin/lipc-set-prop -s com.lab126.keyboard close net.fabiszewski.leafpad", NULL);
          }
        case -1:
          perror("Failed to open keyboard\n");
          exit(1);
      }
    }
 #endif

bool FileExist(string fileName)
{
    FILE *file = fopen(fileName.c_str(), "r");
    if (file)
    {
        fclose(file);
        return 1;
    }
    return 0;
}

string GetFullLocalFilePath(string name)
{
    char curdir[2048];
    GetCurrentDir(curdir, sizeof(curdir));
    return string(curdir) + PATH_SEPARATOR + name;
}

string GetFileName(string file)
{
    guint dirpos = file.find_last_of(PATH_SEPARATOR);
    if (dirpos != string::npos)
        return file.substr(dirpos + 1);
    else
        return file;
}

string GetFilePath(string file)
{
    guint dirpos = file.find_last_of(PATH_SEPARATOR);
    if (dirpos != string::npos)
        return file.substr(0, dirpos);
    else
        return file;
}

std::string GetAppFile(string file)
{
    return GetAppPath() + PATH_SEPARATOR + string(file);
}

std::string GetResFile(string file)
{
    return GetAppFile("res" + (PATH_SEPARATOR + string(file)));
}

bool sorter(FileItem i, FileItem j) { if (i.dir != j.dir) return i.dir > j.dir; else return i.name < j.name; }

static vector<FileItem> GetFiles()
{
    vector<FileItem> result;

    tinydir_dir dir;
	if (tinydir_open(&dir, ".") == -1)
	{
		perror("Error opening file");
		tinydir_close(&dir);
		return result;
	}

	while (dir.has_next)
	{
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1)
		{
			perror("Error getting file");
			continue;
		}

        FileItem fit;
        fit.name = file.name;
        fit.dir = file.is_dir;

		if (fit.name != "." && fit.name != "..")
            result.push_back(fit);

		tinydir_next(&dir);
	}

    tinydir_close(&dir);
    std::sort (result.begin(), result.end(), sorter);
    return result;
}

static bool RemovePath(string path)
{
    tinydir_dir dir;
	if (tinydir_open(&dir, path.c_str()) == -1)
	{
	    if (remove(path.c_str()) == 0)
            return true; // file

		perror("Error opening dir");
		tinydir_close(&dir);
		return false;
	}

    ShakeWindow::DoEvents();

	while (dir.has_next)
	{
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1)
		{
			perror("Error getting file");
			continue;
		}

        FileItem fit;
        fit.name = file.name;
        fit.dir = file.is_dir;

		if (fit.name != "." && fit.name != "..")
        {
            string it = path + PATH_SEPARATOR + fit.name;
            if (fit.dir)
                RemovePath(it);
            else
                remove(it.c_str());
        }

		tinydir_next(&dir);
	}

    tinydir_close(&dir);
    return rmdir(path.c_str()) == 0;
}

bool CopyLocalFile(string from, string to)
{
    size_t BUFFER_SIZE = 4096;
    char buf[BUFFER_SIZE];
    size_t size;

    FILE* source = fopen(from.c_str(), "rb");
    if (source == NULL) return false;
    FILE* dest = fopen(to.c_str(), "wb");
    if (dest == NULL) return false;

    while (true)
    {
        size = fread(buf, 1, BUFFER_SIZE, source);
        if (size <= 0) break;
        fwrite(buf, 1, size, dest);
    }

    fclose(source);
    fclose(dest);
    return true;
}

static bool CopyPath(string from, string todir)
{
    if (!DirectoryExists(from))
    {
        if (FileExist(from))
            return CopyLocalFile(from, todir);
        else
        {
            (new ShakeWindow())->MessageBox(from + " not found");
            return false;
        }
    }

    if (!DirectoryExists(todir))
        CreateFolder(todir.c_str(), 766);

    tinydir_dir dir;
	if (tinydir_open(&dir, from.c_str()) == -1)
	{
		(new ShakeWindow())->MessageBox("Can't open directory " + from);
		tinydir_close(&dir);
		return false;
	}

    ShakeWindow::DoEvents();

	while (dir.has_next)
	{
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1)
		{
		    (new ShakeWindow())->MessageBox(string("Can't open file ") + file.name);
			continue;
		}

        FileItem fit;
        fit.name = file.name;
        fit.dir = file.is_dir;

		if (fit.name != "." && fit.name != "..")
        {
            string f1 = from + PATH_SEPARATOR + fit.name;
            string f2 = todir + PATH_SEPARATOR + fit.name;
            if (fit.dir)
                CopyPath(f1, f2);
            else if (!FileExist(f2))
                CopyLocalFile(f1, f2);
        }

		tinydir_next(&dir);
	}

    tinydir_close(&dir);
    return true;
}

std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

bool CheckExtension(string list, string fileName)
{
    list += ";";
    string ext = fileName.substr(fileName.find_last_of(".") + 1);
    return list.find(ext + ";") != std::string::npos;
}

#endif // END
