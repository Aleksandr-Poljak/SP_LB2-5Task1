#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <sstream>
#include <algorithm>
#include "framework.h"
#include "FileRev2.2.h"
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define MAX_LOADSTRING 100

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING] = TEXT("FileReverce");
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR fileName[MAX_LOADSTRING] = TEXT("FILEREV.DAT");
HANDLE hFile = INVALID_HANDLE_VALUE;
HANDLE hFileMap = NULL;
LPVOID lpvFile = NULL;
LPSTR lpchANSI = NULL;
DWORD dwFileSize = 0;

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDC_FILEREV22, szWindowClass, MAX_LOADSTRING);
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) { return FALSE; }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FILEREV22));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FILEREV22));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FILEREV22);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) { return FALSE; }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        TCHAR szFileName[MAX_PATH] = { 0 };
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_FILE_SELECT:
        {
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = NULL;
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = sizeof(szFileName) / sizeof(*szFileName);
            ofn.lpstrFilter = TEXT("All Files\0*.*\0Text Files\0*.TXT\0");
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (!GetOpenFileName(&ofn))
            {
                MessageBox(NULL, TEXT("ERROR!\nFile for conversion is not selected."), TEXT("FileRev"), MB_OK);
                return(0);
            }
            if (CopyFile(szFileName, fileName, FALSE))
            {
                MessageBox(NULL, TEXT("New file successfully created!"), TEXT("FileRev"), MB_OK);
            }
            else
            {
                MessageBox(NULL, TEXT("ERROR!\nNew file cannot be created."), TEXT("FileRev"), MB_OK);
                return (0);
            }
            hFile = CreateFile(fileName,
                GENERIC_WRITE | GENERIC_READ,
                0,
                nullptr,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                nullptr);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                MessageBox(NULL, TEXT("ERROR!\nFile failed to open."), TEXT("FileRev"), MB_OK);
                return(0);
            }
        }
        break;

        case ID_MAPFILE_CREATE:
        {
            dwFileSize = GetFileSize(hFile, NULL);
            if (dwFileSize == INVALID_FILE_SIZE)
            {
                MessageBox(NULL, TEXT("ERROR!\nFailed to get file size."), TEXT("FileRev"), MB_OK);
                CloseHandle(hFile);
                return (0);
            }

            hFileMap = CreateFileMapping(
                hFile,
                NULL,
                PAGE_READWRITE,
                0,
                dwFileSize + sizeof(WCHAR),
                NULL);
            if (hFileMap == NULL)
            {
                MessageBox(NULL, TEXT("ERROR!\nFailed to create file mapping."), TEXT("FileRev"), MB_OK);
                CloseHandle(hFile);
                return (0);
            }
            else
            {
                MessageBox(NULL, TEXT("MAP file successfully created!"), TEXT("FileRev"), MB_OK);
            }
        }
        break;

        case ID_ADRESSSPACE_SHOW:
        {
            lpvFile = MapViewOfFile(
                hFileMap,
                FILE_MAP_READ | FILE_MAP_WRITE, // Убедитесь, что доступ для чтения и записи установлен
                0,
                0,
                0);
            if (lpvFile == NULL)
            {
                MessageBox(NULL, TEXT("ERROR!\nFailed to map view of file."), TEXT("FileRev"), MB_OK);
                CloseHandle(hFileMap);
                CloseHandle(hFile);
                return(0);
            }
            else
            {
                lpchANSI = (LPSTR)lpvFile; // Привязываем к отображению файла
                MessageBox(NULL, TEXT("The window view of the file has been successfully projected!"), TEXT("FileRev"), MB_OK);
            }
        }
        break;

        case ID_ADRESSSPACE_CLOSE:
        {
            if (lpvFile)
            {
                UnmapViewOfFile(lpvFile);
                lpvFile = NULL;
            }
            if (hFileMap)
            {
                CloseHandle(hFileMap);
                hFileMap = NULL;
            }
            if (hFile != INVALID_HANDLE_VALUE)
            {
                // Перемещаем указатель файла в конец и устанавливаем размер
                SetFilePointer(hFile, dwFileSize, NULL, FILE_BEGIN);
                SetEndOfFile(hFile);
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }
        }
        break;

        case ID_CONVERSATION_FLIP:
        {
            if (lpchANSI != nullptr && dwFileSize > 0)
            {
                std::string fileContent(lpchANSI, dwFileSize);

                std::stringstream ss(fileContent);
                std::string word;
                std::string reversedContent;

                while (ss >> word)
                {
                    std::reverse(word.begin(), word.end());
                    reversedContent += word + " ";
                }

                if (!reversedContent.empty())
                {
                    reversedContent.pop_back();
                }

                // Убедитесь, что файл открыт и перемещен в конец перед записью
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    MessageBox(NULL, TEXT("ERROR!\nFile handle is invalid."), TEXT("FileRev"), MB_OK);
                    break;
                }

                DWORD written;
                if (!WriteFile(hFile, reversedContent.c_str(), reversedContent.size(), &written, NULL))
                {
                    MessageBox(NULL, TEXT("ERROR!\nFailed to write reversed content to file."), TEXT("FileRev"), MB_OK);
                }
                else
                {
                    MessageBox(NULL, TEXT("Words successfully reversed and written to the end of the file!"), TEXT("FileRev"), MB_OK);
                }
            }
            else
            {
                MessageBox(NULL, TEXT("ERROR!\nFile content is empty or not mapped."), TEXT("FileRev"), MB_OK);
            }
        }
        break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
