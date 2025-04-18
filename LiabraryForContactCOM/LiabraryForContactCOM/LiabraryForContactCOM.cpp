#include <iostream>
#include "Windows.h"
#include "LiabraryForContactCOM.h"
using namespace std;

string SendToCom(const string& Port, string Message) {
    wstring wPort = wstring(Port.begin(), Port.end());
    HANDLE hSerial = CreateFile(
        wPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL
    );
    if (hSerial == INVALID_HANDLE_VALUE) {
        return "false: CreateFile failed";
    }

    // Настройка скорости
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(dcb);
    if (!GetCommState(hSerial, &dcb)) {
        CloseHandle(hSerial);
        return "false: GetCommState failed";
    }
    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;
    if (!SetCommState(hSerial, &dcb)) {
        CloseHandle(hSerial);
        return "false: SetCommState failed";
    }

    PurgeComm(hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    // Настройка таймаутов
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 500;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);
    if (!SetCommTimeouts(hSerial, &timeouts)) {
        CloseHandle(hSerial);
        return "false: SetCommTimeouts failed";
    }

    // Отправка сообщения
    DWORD bytesWritten;
    if (!WriteFile(hSerial, Message.c_str(), Message.size(), &bytesWritten, NULL)) {
        CloseHandle(hSerial);
        return "false: WriteFile failed";
    }

    // Чтение ответа
    char buffer[256] = { 0 };
    DWORD bytesRead;
    std::string response;

    do {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL)) {
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                response += buffer;
            }
        }
    } while (bytesRead > 0 && response.find('\n') == std::string::npos);

    // Удаляем \r\n из ответа
    response.erase(std::remove(response.begin(), response.end(), '\r'), response.end());
    response.erase(std::remove(response.begin(), response.end(), '\n'), response.end());

    return response.empty() ? "No response" : response;
}