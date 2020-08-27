#include <iostream>
#include <WinSock2.h>
#include <string>
#include <locale>
#include <codecvt> 
#include <MSWSock.h>
#include <thread>
using std::string;

std::wstring GetProgramDir()
{
    WCHAR exeFullPath[MAX_PATH]; 
    std::wstring strPath;
    ::GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    strPath = exeFullPath;    
    int pos = strPath.find_last_of('\\', strPath.length());
    return std::move(strPath.substr(0, pos));
}
int main(int argc, char* argv[])
{
    std::string ip;
    std::string fileFullName;
    if (argc == 3)
    {
        ip = argv[1];
        fileFullName = argv[2];
    }
    else
    {
        std::cout << "Paramter number error!" << std::endl;
        return -1;
    }
    WORD ver = MAKEWORD(2, 2);
    WSADATA data;
    ::WSAStartup(ver, &data);
    SOCKET clientSock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSock == INVALID_SOCKET)
    {
        std::cout << "Create socket channel fail, error code: " << GetLastError() << std::endl;
        return -1;
    }
    SOCKADDR_IN		remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    remoteAddr.sin_port = ::htons(12001);
    int ret = ::connect(clientSock, (sockaddr*)(&remoteAddr), sizeof(remoteAddr));
    if (ret == SOCKET_ERROR)
    {
        std::cout << "Connect server fail, error code: !" << GetLastError() << std::endl;
        ::closesocket(clientSock);
        return -1;
    }
    size_t fileNamePos = fileFullName.rfind("\\");
    std::string fileName = fileFullName.substr(fileNamePos+1) + "#--";
    if (::send(clientSock, fileName.c_str(), fileName.length(), 0) != fileName.length())
    {
        std::cout << "Send fail, error code: " << GetLastError() << std::endl;
        ::closesocket(clientSock);
        return -1;
    }
    else
    {
        std::cout << "Sended file name: " << fileName.substr(0, fileName.length()-3) << std::endl;
    }
    std::wstring fileNameU = std::wstring_convert<std::codecvt_utf8<WCHAR>, WCHAR>().from_bytes(fileFullName);
    HANDLE fileHandle = CreateFile(fileNameU.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        std::cout << "Open file fail, error code: " << GetLastError() <<  std::endl;
        ::closesocket(clientSock);
        return -1;
    } 
    if (::TransmitFile(clientSock, fileHandle, 0, 0, nullptr, nullptr, TF_USE_DEFAULT_WORKER))
    {
        std::cout << "send success !";
    }
    else
    {
        std::cout << GetLastError() << std::endl;
        std::cout << "send fail !";
    }
    ::CloseHandle(fileHandle);
    ::closesocket(clientSock);
    
}