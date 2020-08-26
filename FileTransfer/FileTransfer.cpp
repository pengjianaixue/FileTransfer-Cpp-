

#include <iostream>
#include <Windows.h>
#include <string>
#include <locale>
#include <codecvt>  
int main(int argc, char* argv[])
{
    std::string ip;
    std::string fileName;
    if (argc=3)
    {
        ip = argv[1];
        fileName = argv[2];
    }
    else
    {
        return -1;
    }
    SOCKET clientSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN		remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    remoteAddr.sin_port = htons(12001);
    int ret = ::connect(clientSock, (sockaddr*)(&remoteAddr), sizeof(remoteAddr));
    if (ret == SOCKET_ERROR)
    {
        closesocket(clientSock);
        return -1;
    }
    std::wstring fileNameU = std::wstring_convert<std::codecvt_utf8<WCHAR>, WCHAR>().from_bytes(fileName);
    HANDLE fileHandle =  CreateFile(fileNameU.c_str(), GENERIC_READ, 0, nullptr, 0, 0, nullptr);
    TRANSMIT_FILE_BUFFERS TransmitFileBuff;
    if (TransmitFile(clientSock, fileHandle, 0, 256, nullptr, &TransmitFileBuff, 0))
    {


    }
    CloseHandle(fileHandle);
    std::cout << "Hello World!\n";
}