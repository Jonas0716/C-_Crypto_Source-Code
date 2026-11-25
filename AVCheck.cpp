#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib") 

std::wstring ToLower(const std::wstring& str) {
    std::wstring lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
    return lowerStr;
}

int CheckAntivirusStatus() {
    HRESULT hres;

    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres) && hres != RPC_E_CHANGED_MODE) {
        return -1;
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,                          
        NULL,                        
        NULL,                        
        RPC_C_AUTHN_LEVEL_DEFAULT,   
        RPC_C_IMP_LEVEL_IMPERSONATE, 
        NULL,                        
        EOAC_NONE,                   
        NULL                         
    );

    if (FAILED(hres) && hres != RPC_E_TOO_LATE) {
        CoUninitialize();
        return -1;
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );

    if (FAILED(hres)) {
        CoUninitialize();
        return -1;
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\SecurityCenter2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT displayName, productState FROM AntiVirusProduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return -1;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    bool hasDefender = false;
    bool hasThirdPartyAV = false;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        VariantInit(&vtProp);

        hr = pclsObj->Get(L"displayName", 0, &vtProp, 0, 0);
        if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR && vtProp.bstrVal != NULL) {
            std::wstring avName = ToLower(vtProp.bstrVal);

            if (avName.find(L"alyac") != std::wstring::npos ||
                avName.find(L"ahnlab") != std::wstring::npos ||
                avName.find(L"v3") != std::wstring::npos ||
                avName.find(L"mcafee") != std::wstring::npos) {
                hasThirdPartyAV = true;
            }
            else if (avName.find(L"windows defender") != std::wstring::npos ||
                avName.find(L"microsoft defender") != std::wstring::npos) {
                hasDefender = true;
            }
        }
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pEnumerator->Release();
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();

    if (hasThirdPartyAV) {
        return 0;
    }
    if (hasDefender) {
        return 1;
    }
    return -1;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    __try {
        int avStatus = CheckAntivirusStatus();
        if (avStatus == 1) {
            //Windows Defender
        }
        else if (avStatus == 0) {
            //Third Party AV
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
    return 0;
}