#include <windows.h>
#include <stdio.h>
#include <LoadLibraryR.h>
#include <Native.h>

void ExecuteJps(){
    // Execute "jps -l" and print the output
    char command[MAX_PATH] = "jps -l";
    FILE* pipe = _popen(command, "r");
    if (!pipe) {
        printf("Error opening pipe!\n");
        return;
    }
    char buffer[128];
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            printf("%s", buffer);
    }
    _pclose(pipe);
}

int main(){
    ExecuteJps();

    printf("Enter the process ID: ");
    const char dllPath[MAX_PATH] = "build/DllMain.dll";
    DWORD processID;
    scanf("%d", &processID);
    injectDll(processID, dllPath);
}