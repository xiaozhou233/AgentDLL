/*
 * @Author: xiaozhou233
 * @Date: 2025-02-02 14:05:03
 * @Description: Implement JNI functions
 * 
 */

#include <Java_cn_xiaozhou233_OrangeX_JNI_NativeDll_inject.h>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "LoadLibraryR.h"
#include <Native.h>

#pragma comment(lib,"Advapi32.lib")

#define BREAK_WITH_ERROR( e ) { printf( "[-] %s. Error=%d", e, GetLastError() ); break; }

/**
 * @description: Inject DLL into target process
 * @note: This implementation references the solution by @stephenfewer 
 * (github.com/stephenfewer/ReflectiveDLLInjection). Many thanks to the original author and contributors
 * for their contribution to open-source software.
 * @param {JNIEnv} *env JNI environment
 * @param {jint} pid Target process ID
 * @param {jstring} dllPath Path to DLL file
 * @return {*}
 */
JNIEXPORT jboolean JNICALL Java_cn_xiaozhou233_OrangeX_JNI_NativeDll_inject(JNIEnv *env, jobject obj, jint pid, jstring dllPath) {
    DWORD dwProcessId = pid;
    
    // 修正 JNIEnv 指针使用
    const char *dllPathStr = (*env)->GetStringUTFChars(env, dllPath, 0);

    printf("[+] Get args:\n");
    printf(" pid: %d, dllPath: %s\n", dwProcessId, dllPathStr);

    injectDll(dwProcessId, dllPathStr);

    // 释放字符串资源
    (*env)->ReleaseStringUTFChars(env, dllPath, dllPathStr);

    return JNI_TRUE; // 修正返回值
}

int injectDll(int pid, const char *dllPath) {
	HANDLE hFile          = NULL;
	HANDLE hModule        = NULL;
	HANDLE hProcess       = NULL;
	HANDLE hToken         = NULL;
	LPVOID lpBuffer       = NULL;
	DWORD dwLength        = 0;
	DWORD dwBytesRead     = 0;
	TOKEN_PRIVILEGES priv = {0};

	do {
		hFile = CreateFileA( dllPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		if( hFile == INVALID_HANDLE_VALUE )
			BREAK_WITH_ERROR( "Failed to open the DLL file" );

		dwLength = GetFileSize( hFile, NULL );
		if( dwLength == INVALID_FILE_SIZE || dwLength == 0 )
			BREAK_WITH_ERROR( "Failed to get the DLL file size" );

		lpBuffer = HeapAlloc( GetProcessHeap(), 0, dwLength );
		if( !lpBuffer )
			BREAK_WITH_ERROR( "Failed to get the DLL file size" );

		if( ReadFile( hFile, lpBuffer, dwLength, &dwBytesRead, NULL ) == FALSE )
			BREAK_WITH_ERROR( "Failed to alloc a buffer!" );

		if( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
		{
			priv.PrivilegeCount           = 1;
			priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		
			if( LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &priv.Privileges[0].Luid ) )
				AdjustTokenPrivileges( hToken, FALSE, &priv, 0, NULL, NULL );

			CloseHandle( hToken );
		}

		hProcess = OpenProcess( PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid );
		if( !hProcess )
			BREAK_WITH_ERROR( "Failed to open the target process" );

		hModule = LoadRemoteLibraryR( hProcess, lpBuffer, dwLength, NULL );
		if( !hModule )
			BREAK_WITH_ERROR( "Failed to inject the DLL" );

		printf( "[+] Injected the '%s' DLL into process %d.", dllPath, pid );
		
		WaitForSingleObject( hModule, -1 );

	} while( 0 );

	if( lpBuffer )
		HeapFree( GetProcessHeap(), 0, lpBuffer );

	if( hProcess )
		CloseHandle( hProcess );

	return 1;
}