#include <iostream>
#include <WinSock2.h>
#include <string>
#include <locale>
#include <codecvt> 
#include <MSWSock.h>
#include <thread>
#include <fstream>
#include <sstream>
#include <ostream>
#include <cryptlib.h>
#include <md5.h>
#include <filters.h>
#include <hex.h>
#include <files.h>
using std::string;
using namespace CryptoPP;
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
std::wstring getProgramDir()
{
    WCHAR exeFullPath[MAX_PATH]; 
    std::wstring strPath;
    ::GetModuleFileName(NULL, exeFullPath, MAX_PATH);
    strPath = exeFullPath;    
    int pos = strPath.find_last_of('\\', strPath.length());
    return std::move(strPath.substr(0, pos));
}
void recviServerReponse(void* params)
{
    SOCKET *clientSock = static_cast<SOCKET *>(params);
    char buff[1024]{};
    while (true)
    {
        if (::recv(*clientSock, buff, 1024, 0)>0)
        {
            std::cout << std::string(buff) << std::endl;
            break;
        }
    }


}
std::string getFileMD5Value(const std::string& fileName)
{
    
    std::string md5value;
    std::ifstream checkFileStream;
    std::wstring fileNameU = std::wstring_convert<std::codecvt_utf8<WCHAR>, WCHAR>().from_bytes(fileName);
    HANDLE fileHandle =  ::CreateFile(fileNameU.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return std::move(std::string());
    }
    size_t fileLength =  ::GetFileSize(fileHandle, nullptr);
    char  *readBuff = new char[fileLength] {};
    if (!::ReadFile(fileHandle, readBuff, fileLength, nullptr, nullptr))
    {
        return std::move(std::string());
    }
    std::ostringstream fileBuff;
    Weak1::MD5 hash;
    hash.Update((const byte*)readBuff, fileLength);
    md5value.resize(hash.DigestSize());
    hash.Final((byte*)&md5value[0]);
    HexEncoder encoder(new FileSink(fileBuff));
    StringSource(md5value, true, new Redirector(encoder));
    delete[] readBuff;
    return std::move(fileBuff.str());

}
std::string  getFileName(std::string fullFileName)
{
    size_t fileNamePos = fullFileName.rfind("\\");
    if (fileNamePos!=std::string::npos)
    {
        return std::move(fullFileName.substr(fileNamePos + 1));
    }
    else
    {
        fileNamePos = fullFileName.rfind("/");
        if (fileNamePos != std::string::npos)
        {
            return std::move(fullFileName.substr(fileNamePos + 1));
        }
    }
    return std::move(std::string(""));

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
    std::string  md5Vaule = getFileMD5Value(fileFullName);
    std::string fileMD5Vaule = md5Vaule + "-#-";

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
    //std::thread recviThread(recviServerReponse, &clientSock);
    if (::send(clientSock, fileMD5Vaule.c_str(), fileMD5Vaule.length(), 0) != fileMD5Vaule.length())
    {
        std::cout << "Send file MD5 fail, error code: " << GetLastError() << std::endl;
        ::closesocket(clientSock);
        return -1;
    }
    std::string fileName = getFileName(fileFullName) + "#--";
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
    
    std::wstring unicodeFileName = std::wstring_convert<std::codecvt_utf8<WCHAR>, WCHAR>().from_bytes(fileFullName);
    HANDLE fileHandle = CreateFile(unicodeFileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
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
    //if (recviThread.joinable())
    //{
    //    recviThread.join();
    //}
    ::CloseHandle(fileHandle);
    ::closesocket(clientSock);
    
}