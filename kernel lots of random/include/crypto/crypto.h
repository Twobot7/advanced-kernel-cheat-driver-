#pragma once
#include <Windows.h>
#include <vector>
#include <string>

class Crypto {
public:
    static std::vector<BYTE> DecryptFile(const std::wstring& path);
    static bool VerifySignature(const std::vector<BYTE>& data);
}; 