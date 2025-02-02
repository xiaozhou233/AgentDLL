CC = gcc
BUILD_DIR = build
TMP_DIR = $(BUILD_DIR)/tmp
INCLUDES = -Iinclude/jvm \
	-Iinclude/jvm/win32 \
	-Iinclude/OrangeX \
	-Iinclude/ReflectiveDLLInjection

Args = -lpsapi -lkernel32 -luser32 -ladvapi32 -lshell32

$(shell mkdir -p $(TMP_DIR))

all: deps exe link clean run_inject


# Build Deps
deps:
	@echo "====== Building Dependencies ======"
	$(CC) -c include/ReflectiveDLLInjection/GetProcAddressR.c -o $(TMP_DIR)/GetProcAddressR.o $(INCLUDES)
	$(CC) -c include/ReflectiveDLLInjection/LoadLibraryR.c -o $(TMP_DIR)/LoadLibraryR.o $(INCLUDES)
	$(CC) -c include/ReflectiveDLLInjection/ReflectiveLoader.c -o $(TMP_DIR)/ReflectiveLoader.o $(INCLUDES)
	@echo "====== Done ======"
	@echo ""

exe:
	@echo "====== Building Executable ======"
	$(CC) -c DllMain.c -o $(TMP_DIR)/DllMain.o $(INCLUDES)
	$(CC) -c Native.c -o $(TMP_DIR)/Native.o $(INCLUDES)
	$(CC) -c inject.c -o $(TMP_DIR)/inject.o $(INCLUDES)
	@echo "====== Done ======"

Dependencies = $(TMP_DIR)/GetProcAddressR.o $(TMP_DIR)/LoadLibraryR.o $(TMP_DIR)/ReflectiveLoader.o

link:
	@echo "====== Linking ======"
	@echo "====== Linking DllMain ======"
	$(CC) -shared -o $(BUILD_DIR)/DllMain.dll $(TMP_DIR)/DllMain.o $(Dependencies) $(Args)
	@echo "====== Linking Native ======"
	$(CC) -shared -o $(BUILD_DIR)/Native.dll $(TMP_DIR)/Native.o $(Dependencies) $(Args)
	@echo "====== Linking inject ======"
	$(CC) -o inject.exe $(TMP_DIR)/inject.o $(TMP_DIR)/Native.o $(Dependencies) $(Args)
	@echo "====== Done ======"

run_inject:
	./inject.exe

clean:
	rm -rf $(TMP_DIR)
