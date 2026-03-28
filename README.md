# ArmMem
适用于安卓的 Inline Hook 和内存读写库  
Inline Hook and Memory I/O Library for Android

## 特性及 TODO
- [x] Hook 支持 arm64-v8a (aarch64)
- [x] Hook 支持 armeabi-v7a (arm)
- [ ] ~~Hook 支持 x86~~
- [ ] ~~Hook 支持 x86_64~~
- [x] 解除 Hook
- [ ] ~~GOT/PLT Hook 支持~~
- [x] Java 层内存读写封装
- [ ] ~~Java 层 Hook 封装~~
- [ ] aarch64/arm 内存写入断点与监控
- [x] Dword 内存操作
- [x] Float 内存操作
- [x] Double 内存操作
- [x] Byte 内存操作
- [x] Word 内存操作
- [x] Qword 内存操作
- [x] 内存搜索
- [x] 内存二次搜索
- [ ] 可执行段特征码搜索
- [ ] 联合搜索
- [ ] 模糊搜索
- [x] Prefab 支持

## 构建
使用 Android Studio 构建 ArmMem 模块或使用 gradlew 命令行构建

## 使用
### 导入库
在应用模块的 build.gradle 中启用 Prefab 支持，并引入 aar
```groovy build.gradle
android {
    buildFeatures {
        prefab true
    }
}
dependencies {
    implementation file('ArmMem.aar')
}
```

## Hook 函数

**跨进程 Hook 需要注入 ELF 到目标进程**  

```cpp
#include "armmem/hook.h"
// 这是一个被 Hook 的函数
int plus(int a, int b) {
    return a + b;
}

// 这是原函数指针，用于存储被 Hook 的函数的原始地址
static int (*original_plus)(int, int) = nullptr;

// 这是一个 Hook
int hook_plus(int a, int b) {
    // 调用原函数(可选)
    int result = original_plus(a, b);
    return result - 100;
}

// Hook 它，通过函数指针
HookFunctionHandle* hookHandle = ArmMemHook::hook(&plus, (void*) hook_plus, (void**) &original_plus);

// 或是函数地址(假如你知道目标的绝对地址)
HookFunctionHandle* hookHandle = ArmMemHook::hook(reinterpret_cast<void*>(0x7F12345), (void*) hook_plus, (void**) &original_plus);

// 或是通过模块名和函数符号
HookFunctionHandle* hookHandle = ArmMemHook::hook("libmy-lib.so", "plus", (void*) hook_plus, (void**) &original_plus);

// 现在，当plus被调用时，会跳转到hook_plus函数
plus(1, 2); // -99

// 解除 Hook
ArmMemHook::unhook(hookHandle);

// 现在，当plus被调用时，什么都不会改变
plus(1, 2); // 3
```

## 内存搜索与读写

**受限于 SELinux 策略，所有内存操作都需要在 `AndroidManifest.xml` 中设置 `android:debuggable="true"` 或在 `build.gradle` 中强制启用 debuggable**  
**跨进程内存操作需要 root 权限或拥有相同共享用户 ID**  

> android:sharedUserId  
> 此常量自 API 级别 29 起已废弃。  
> 共享用户 ID 会在软件包管理器中导致具有不确定性的行为。因此，强烈建议您不要使用它们，并且我们在未来的 Android 版本中可能会移除它们。相反，请使用适当的通信机制（例如服务和 Content Provider），在共享组件之间实现互操作性。现有应用无法移除此值，因为不支持迁移共享用户 ID。在这些应用中，请添加 android:sharedUserMaxSdkVersion="32" 以免在新用户安装时使用共享用户 ID。
---
```cpp
// 这是受害者内存
int value = 1145141919;
float floatValue = 114514.1919f;
int getValue() {
    return value;
}
float getFloatValue() {
    return floatValue;
}
```
这里仅作示例，详细API请参考代码
### 对于 Java
```java
import dev1503.armmem.memory.*;

// 搜索内存
MemoryValueSet results = Memory.searchDword(1145141919, Memory.RANGE_C_DATA);
MemoryValueSet resultsFloat = Memory.searchFloat(114514.1919f, 0.1f, Memory.RANGE_C_DATA);
// 可以在第一个参数前传入一个pid，0.1f是误差半径

for (MemoryValue result : results) {
    int pid = result.getPid();
    int value = result.readDword();
    int address = result.getAddress();

    System.out.println("Dword Pid: " + pid + ", Value: " + value + ", Address: " + address);

    // 写入内存
    result.writeDword(123456789);
}

// 或者是一次性读取全部值
int[] dwordValues = results.getDwordValues(100); // 100表示最多读取100个值，默认全部读取

for (MemoryValue result : resultsFloat) {
    int pid = result.getPid();
    float value = result.readFloat();
    int address = result.getAddress();

    System.out.println("Float Pid: " + pid + ", Value: " + value + ", Address: " + address);

    // 写入内存
    result.writeFloat(123456.789f);
}

// 或者是一次性读取全部值
float[] floatValues = resultsFloat.getFloatValues(100);

// 直接读取和写入内存
// 下面的函数都可以在第一个参数传入pid，默认是当前进程

// 写入
Memory.writeDword(0x7F1234578, 123456789);
Memory.writeFloat(0x7F1234590, 123456.789f);
Memory.writeDouble(0x7F12345A8, 123456.789);

// 读取
int dwordValue = Memory.readDword(0x7F1234578);
float floatValueValue = Memory.readFloat(0x7F1234590);
double doubleValue = Memory.readDouble(0x7F12345A8);

// 当地址较多时，使用文件描述符读取内存，提高效率
int fd = Memory.openMemFile(pid); // pid默认是当前进程
for (int address = 0x7012234; address < 0x7511451; address++) {
    int value = Memory.readDword(address, fd);
    System.out.println("Address: " + address + ", Value: " + value);
}
// 记得关闭文件描述符
Memory.closeMemFile(fd);



// 从搜索结果再次搜索
MemoryValueSet resultsFromResults = results.searchDword(987456321); // 使用的是第一个结果的pid
```

可用的内存范围
```java
Memory.RANGE_C_HEAP
Memory.RANGE_JAVA_HEAP
Memory.RANGE_C_ALLOC
Memory.RANGE_C_DATA
Memory.RANGE_C_BSS
Memory.RANGE_ANONYMOUS
Memory.RANGE_CODE_APP 
Memory.RANGE_STACK
Memory.RANGE_ASHMEM
Memory.RANGE_OTHER 
```

### 对于 C++
和 Java 大同小异
```cpp
#include "armmem/memory.h"

std::vector<MemoryValue> results = ArmMemMemory::searchDword(1145141919, MemoryRange::C_DATA);
// 也是可以传入pid

for (MemoryValue result : results) {
    std::cout << "Dword Value: " << result.value.dwordValue << ", Address: " << result.address << std::endl;
    // 不同的是，MemoryValue只是一个结构体，不能直接写入内存
}

uintptr_t address = 0x7F123458;

// 写入内存
ArmMemMemory::writeDword(address, 3214567);

// 读取内存
bool success = false;
int dwordValue = ArmMemMemory::readDword(address, &success);

// 通过文件描述符读取内存
int fd = ArmMemMemory::openMemFile(pid); // pid默认是当前进程
int value = ArmMemMemory::readDword(address, fd, &success);
// 记得关闭文件描述符
close(fd);



// 从搜索结果再次搜索
std::vector<MemoryValue> resultsFromResults = ArmMemMemory::searchDword(pid, 987456321, results);
// 也可以传入地址数组
std::vector<uintptr_t> addresses = {0x7F123458, 0x7F1234590};
std::vector<MemoryValue> resultsFromAddresses = ArmMemMemory::searchDword(pid, 987456321, addresses);
```

可用的内存范围
```cpp
MemoryRange::C_HEAP
MemoryRange::JAVA_HEAP
MemoryRange::C_ALLOC
MemoryRange::C_DATA
MemoryRange::C_BSS
MemoryRange::ANONYMOUS
MemoryRange::CODE_APP 
MemoryRange::STACK
MemoryRange::ASHMEM
MemoryRange::OTHER 
```

详细API: [memory.h](https://github.com/1503Dev/ArmMem/blob/main/ArmMem/src/main/cpp/exports/armmem/memory.h)

## 许可证
开放源代码、使用和分发根据 [LGPL-2.1](LICENSE) 许可证  

这意味着  
- 动态链接  
  你可以自由地动态链接 ArmMem 库。在这种情况下，你的程序无需开源，只需在适当位置（如文档或关于页面）声明使用了 ArmMem，并标注其代码仓库的 URL。

- 静态链接  
  如果你静态链接了本库，在分发程序时，你必须提供程序的 **目标文件（Object Files）** 或 **源代码**，以允许用户能够通过重新链接来更换不同版本的 ArmMem 库。

- 修改库文件  
  **如果你修改了 ArmMem 的源代码，在分发时必须以 LGPL-2.1 协议公开库本身及其修改部分的所有源码。**