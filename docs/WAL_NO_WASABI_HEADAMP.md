# WAL 不依赖 Wasabi 的可行性与 HeadAMP C++ 验证

日期：2026-09-24。约束：**不使用 Wasabi，其他必要组件允许使用**。以本地 Winamp 源码与 `Skin/waskin/HeadAMP.wal` 为依据。

后续接入已按用户要求拆成 `makivm/ttp_maki.dll` 与 `waskin/ttp_waskin.dll`。本文保留分析和原型阶段记录；当前实现、边界和验证见 [WAL 运行时](WAL_RUNTIME.md)。

## 1. 结论

**可行。可以在 `ttp_waskin.dll` 内独立实现 WAL 的 XML 界面运行时与 MAKI 字节码虚拟机，再连接现有 `TtpSkinHost`，不需要 Wasabi 服务管理器、服务工厂或 `gen_ff.dll`。**

但是，“不要 Wasabi”与“只执行 MAKI”是两个不同条件：

- 不要 Wasabi：可通过自行实现对象、控件、事件、绘制和宿主桥接达成，本次 C++ 原型已经验证。
- 只有 MAKI：不足以显示或操作 HeadAMP。脚本调用的是界面对象；窗口树、图集、命中、滑块动作、动画时钟与播放状态需要运行时提供。
- “任意 WAL 都能用”：本次没有证明。完整现代皮肤还涉及更多 XML 控件、XUI、动态对象、容器、组件、配置和脚本语义。

建议采用 **C++ / Win32 + 图片解码 + XML 对象树 + MAKI VM + TTPlayer 功能桥接**。Python 临时探针已移除，最终判断以本地 C++ 测试为准。

当前是可行性验证：**尚未把 WAL 支持接入发布用 `ttp_waskin.dll`**。现有 WSZ 的识别、默认皮肤和发布包没有因该原型改变。

## 2. 原版代码说明：MAKI 为什么不能单独包办界面

| 环节 | 本地源码证据 | 独立实现必须承担的职责 |
| --- | --- | --- |
| 原版现代皮肤插件初始化 | [gen_ff/main.cpp](../../winamp/Src/Plugins/General/gen_ff/main.cpp)，`init()` 首先通过 `IPC_GET_API_SERVICE` 获取 Wasabi；不存在即失败 | 不能直接加载原版 `gen_ff.dll` 后省略服务宿主。新实现应走自己的入口 |
| XML 创建对象、挂接脚本 | [skinparse.cpp](../../winamp/Src/Wasabi/api/skin/skinparse.cpp)，`XML_TAG_SCRIPT` 分支调用 `Script::addScript`，设置参数、父 Group，再关联脚本 | 先创建 XML 对象，再把脚本绑定到正确实例；同一 groupdef 可有多个实例，不能把 ID 当成全局单例 |
| 加载 `.maki` | [script.cpp](../../winamp/Src/Wasabi/api/script/script.cpp)，`Script::addScript()` 创建 SystemObject，调用 `VCPU::addScript` 和 `ObjectTable::checkScript` | 每份脚本独立变量、System 上下文、事件绑定和生命周期 |
| 字节码与方法链接 | [vcpu.cpp](../../winamp/Src/Wasabi/api/script/vcpu.cpp)，`addScript/runCode/callDLF`；[opcodes.h](../../winamp/Src/Wasabi/api/script/opcodes.h) | 解析 GUID 类型表、方法导入、变量、字符串、事件、代码；执行栈、分支、赋值、调用，按签名连接本地方法 |
| 原生对象控制器 | [objecttable.cpp](../../winamp/Src/Wasabi/api/script/objecttable.cpp)，`registerClass/setupDLF` | MAKI 方法本身没有“画一个 Button”的实现。Button、Group 等方法由对象控制器提供。独立实现可用静态注册表替代 Wasabi 服务发现 |
| 按钮默认动作 | [button.cpp](../../winamp/Src/Wasabi/api/skin/widgets/button.cpp)，`onLeftPush()` 触发脚本事件并处理动作 | `action="PLAY"` 并不自动变成 MAKI 播放函数；XML 动作也必须接到宿主 |
| 抽屉动画 | [guiobj.cpp](../../winamp/Src/Wasabi/api/script/objects/guiobj.cpp)，`guiobject_setTargetSpeed/gotoTarget/onTargetTimer` | 速度按 250 ms 单位量化，以计时器推进平滑位置，结束后触发 `onTargetReached` |
| 音量、EQ、状态 | [systemobj.cpp](../../winamp/Src/Wasabi/api/script/objects/systemobj.cpp)，`vcpu_getStatus/setVolume/setEqBand` 调用 `WASABI_API_MEDIACORE` | 用现有 TTPlayer 查询和命令代替原版媒体核心 |
| 内嵌组件 | HeadAMP XML 的 `<component param="guid:pl">`、`guid:avs` | GUID 是查找宿主内容的标识，脚本并不包含播放列表或 AVS 引擎 |

因此无需保留 Wasabi 的服务架构，但需要实现该皮肤实际使用的行为。直接复制 VCPU 源文件也不能自动解除依赖：它仍引用 SystemObject、ObjectTable、控制器、内存与调试等设施。

## 3. HeadAMP 包的实际依赖

样本 SHA-256：

```text
6117289aefe430bc38c448497600eec4c34cad96ce1173f3717defa5d7bc3f96
```

包内 41 个文件，总解压大小 518,283 字节：26 PNG、2 TTF、6 XML、3 `.m` 源文件、4 `.maki` 字节码。主入口为 `WinampAbstractionLayer version="0.8"`，主 layout 为 `main/mode-main`，逻辑尺寸 **760 × 394**。

XML 共定义 9 个 groupdef、1 个 container、1 个 layout；包含 43 个 button、13 个 slider、28 个 layer 和 2 个 component。这里是全包定义数量，包含标准窗口模板，不等于主界面实际实例数量。不能据此认为每个定义都已覆盖。

### 3.1 脚本清单

| 脚本 | 格式 | GUID 表项 | 方法导入 | 变量 | 事件绑定 | 代码字节 | 本次执行 |
| --- | --- | ---: | ---: | ---: | ---: | ---: | --- |
| `leftDrawerToggle.maki` | `0x17` | 40 | 23 | 96 | 29 | 2,902 | 是 |
| `playerMain.maki` | `0x17` | 40 | 30 | 55 | 10 | 1,468 | 是 |
| `rightDrawerToggle.maki` | `0x17` | 40 | 27 | 44 | 7 | 1,203 | 是 |
| `standardframe.maki` | `0x17` | 32 | 14 | 60 | 4 | 1,054 | 仅解析，未实例化标准附属窗口 |

40 个 GUID 表项包含编译器标准声明，**不代表主界面必须实现 40 种对象**。应分别统计类型表、方法导入、事件绑定和实际运行到的调用。

前三份脚本使用的主要对象是 System、Group、Layout、Container、Button、Layer、Slider、Component/WindowHolder 和 Vis。主要调用包括：

- 对象查找：`getScriptGroup/getContainer/getLayout/findObject/getObject`。
- 显示与属性：`show/hide/getXMLparam/setXMLparam`。
- 动画：`setTargetSpeed/setTargetX/gotoTarget/onTargetReached`。
- 事件：`onScriptLoaded/onLeftClick/leftClick/onPlay/onPause/onResume/onStop`。
- 播放控制：`getStatus/setVolume/setEqBand`，以及 XML `PLAY/PAUSE/STOP/PREV/NEXT/SEEK/VOLUME/PAN/EQ_BAND` 动作。
- 组件发现：`onGetCancelComponent/onLookForComponent`。

前三个字节码还带有编译器注入的运行时版本检查，方法表包含 `getRuntimeVersion/getPrivateInt/messageBox` 等，不能只扫描 `.m` 源码来推断依赖。原型提供测试版本值以通过该样本的版本检查；这不是对全部 5.666 API 的兼容承诺。未实现的运行路径报错，不将所有未知调用静默视作成功。

### 3.2 必须正确恢复的交互时序

1. **Play/Pause 同位置覆盖**：两个按钮均在 XML 中定义；启动和播放状态事件决定哪一个显示。只画 XML 会把两个状态叠在一起。
2. **跨脚本抽屉控制**：主面板 `eqToggle/plToggle` 查找其他 Group 的按钮，再调用 `leftClick()`。事件必须派发到对应 Group 的脚本实例。
3. **左抽屉**：x 从 207 动画到 0；右抽屉从 277 动画到 488；脚本设置的动画时间为 0.25 秒。
4. **右侧播放列表**：打开期间 `InlinePlaylist.w=0`，等 `RightDrawer.onTargetReached()` 才设为 172。立即展开组件或完全忽略完成事件都会显示错误。
5. **AVS 开关**：仅切换 `InlineAVS` 与 `vis` 的可见性；真实绘制内容必须由宿主提供。

### 3.3 单位转换不能直接照搬整数

| Winamp 侧 | 当前 TTPlayer 插件接口 | 接入要求 |
| --- | --- | --- |
| `System.setVolume(0…255)` | `TTP_SKIN_VOLUME` 0…100 | 缩放并钳制 |
| PAN 滑块 `position - 127` | balance -100…100 | 保留中点，端点钳制；不能直接减 128 |
| EQ band 0…9、值 -127…127 | `TTP_SKIN_EQ_VALUE + 1…10`、值 -12…12 | 转换段号与幅度；频率标签不等于 TTPlayer DSP 中心频率 |
| Seek 控件 0…65535；MAKI `scriptDivisor=256` | `TTP_SKIN_SEEK` 0…10000 | 拖动显示预览，松开提交一次 |
| Winamp 状态 playing=1、paused=-1 | TTPlayer playing=2、paused=3 | 查询与事件双向转换 |

依据：[svolbar.cpp](../../winamp/Src/Wasabi/api/skin/widgets/svolbar.cpp)、[spanbar.cpp](../../winamp/Src/Wasabi/api/skin/widgets/spanbar.cpp)、[seqband.cpp](../../winamp/Src/Wasabi/api/skin/widgets/seqband.cpp)、[sseeker.cpp](../../winamp/Src/Wasabi/api/skin/widgets/sseeker.cpp)、[pslider.cpp](../../winamp/Src/Wasabi/api/skin/widgets/pslider.cpp)。

## 4. 本次 C++ 测试做了什么

测试源码仅位于本地 `rebuild/tests/waskin`，不加入 Actions、发行包或上传：

- `wal_probe_runtime.h`：直接读 WAL ZIP；复用项目已有 DEFLATE、CRC 与路径检查辅助实现；MSXML6 解析 XML；独立 MAKI 0x17 解释器。
- `headamp_cpp.cpp`：XML 对象实例化、GDI+ PNG 图集绘制、透明像素命中、Win32 窗口输入、动画时钟，以及现有 `TtpSkinHost` 查询／排队命令接口。
- 没有执行 `.m` 源文件，没有调用 Python，也没有加载 Wasabi 或 Winamp DLL。原型不依赖先解压好的样本目录。

普通 C++ 构建与 XP 工具链构建均 **52/52 检查通过**：

- XML 单独加载时两种播放按钮同时可见；不给 MAKI 提供 XML 对象时初始化失败，验证对象运行时确实不可省略。
- 三份原始 `.maki` 执行；本轮分别执行 598、345、301 条指令，共 1,244 条。
- 播放／暂停／继续／停止、左右抽屉、中间动画位置、动画完成后的播放列表宽度。
- 音量两端、平衡两端、十段 EQ 的上下限和复位、定位只在释放时提交。
- 组件隐藏与发现事件返回正确 XML 对象。
- 创建真实 Win32 窗口，以窗口鼠标消息验证命中、按钮状态替换、抽屉开合、滑块捕获与宿主命令。没有操作用户当前播放器或注入桌面鼠标点击。
- 检查 PNG 裁切的源像素，绘制明确使用像素单位，避免 GDI+ 根据 PNG DPI 自动缩放而造成错位。

样本 `eb.close` 声明裁切 `(15,0,15,16)`，源图为 `29×48`，右边越界 1 像素；原型使用透明补齐，避免越界读取或拒绝整个皮肤。这是样本边界处理，未据此声称与原版每个像素完全一致。

XP 工具链产物为 x86、PE 子系统 5.01，**6 个系统 DLL、176 个静态导入通过 XP 与 Win7 导出表检查**。这仍不是 XP/Win7 真机运行证明；MSXML6 是 COM 运行时依赖，需要在目标环境可用。Python 仅用于项目原有的 PE 导入审计工具，不参与 C++ 原型的运行。

输出：

- `rebuild/out/test-artifacts/wal-headamp/cpp/cpp-report.txt`
- `rebuild/out/test-artifacts/wal-headamp/cpp/headamp-cpp-closed.png`
- `rebuild/out/test-artifacts/wal-headamp/cpp/headamp-cpp-open.png`
- `rebuild/out/test-artifacts/wal-headamp/cpp/headamp-cpp-ui.png`
- `rebuild/out/test-artifacts/wal-headamp/cpp-xp/cpp-report.txt`
- `rebuild/out/test-artifacts/wal-headamp/cpp-xp/legacy-imports.json`

**验证边界**：宿主后端是使用真实 ABI 的 C++ 测试后端，记录并执行模拟状态命令；尚未连接 TTPlayer 实际解码、音频输出或真实播放列表。图中的两个 `HOST PLACEHOLDER` 区域就是未接入的 AVS／播放列表，不能作为它们已经实现的证据。

## 5. 仍需实现的内容

| 能力 | 验证结果／剩余工作 |
| --- | --- |
| 主界面 PNG、布局、按钮、滑块、抽屉与脚本 | 最小 C++ 实现可行，已完成指定样本验证；还需迁入 DLL、处理销毁／重绑／切换／错误恢复 |
| 真实播放 | 现有 `TtpSkinHost` 已有所需基本命令，原型完成单位和状态适配；需接到真实宿主回归 |
| `guid:pl` 内嵌播放列表 | 需列表内容绘制、选择、滚动、菜单、拖入／拖出、曲目提示；复用 TTPlayer 数据和命令，不能只显示占位矩形 |
| `guid:avs` / `Vis` | 可接 TTPlayer 已有 visual/content/spectrum 回调。这样是千千视觉效果替代，不等于支持 Winamp AVS 引擎或 AVS 预设 |
| `standardframe.maki` | 仅成功解析，未执行；含 `getParam/getToken/onSetXuiParam/onNotify/newGroup/init/LayoutStatus.callme` 等额外路径 |
| 通用 WAL | groupdef 继承、多实例、XUI、相对布局、字体、标准窗口、配置对象、动态创建／删除、未覆盖的指令和 API 均需分批补齐 |
| 窗口系统 | 原型保留 Win32 外框，未复原完整异形顶层窗口、皮肤内拖动、吸附、分离、任务栏预览、布局持久化与多 DPI 行为 |
| 运行时边界 | 原型已有 ZIP/CRC/表项/跳转/栈/指令预算检查；还需生产级类型继承检查、实例生命周期、重入/卸载、定时器取消及异常恢复回归 |

## 6. 接入 DLL 的建议顺序

1. **先分离经典与现代运行时入口**：当前 [archive.cpp](../src/archive.cpp) 拒绝 `skin.xml` 且只平铺 BMP/TXT/CUR 等经典资源，[plugin.cpp](../src/plugin.cpp) 全部实例都强转为 `Skin`。新增现代运行时分派及保留目录的 ZIP 视图；继续保留经典校验与默认皮肤行为。
2. **迁入 HeadAMP 所需的 XML/PNG/控件子集**：由 DLL 创建对象树与绘制。优先使用 Win32/GDI+ 和已有 XML 设施，保持共用 x86 DLL 的旧系统目标；不引入 Wasabi 服务管理器。
3. **迁入受限 MAKI VM 与对象方法表**：GUID 类型、继承方法、每实例变量、事件和生命周期明确建模；未知必需功能要明确拒绝并报告，不能通过高版本号或空实现冒充兼容。
4. **连接真实 `TtpSkinHost`**：复用播放、定位、音量、EQ、列表和视觉功能；命令保持排队，避免脚本调用期间卸载 DLL。重新核对主窗口内嵌列表的拖放目标和内容矩形，必要时扩展通用宿主接口。
5. **完成真实播放列表与视觉内容**：先用千千已有功能填入 HeadAMP 的组件区域，再补歌词、右键、曲目提示及编辑行为。不要引入 Winamp 的播放引擎或通用插件宿主。
6. **最后扩大皮肤范围**：补标准窗口与脚本 API，逐个样本建立能力清单，双版本运行回归通过后再将对应 WAL 纳入正式列表。

这个顺序符合“尽量由 DLL 完成”的目标，也避免把 MAKI 虚拟机或皮肤专属对象放进 EXE。

## 7. 本地复现

在 `D:\Projects\Backup\TTPlayer\rebuild` 执行：

```powershell
cmake -S tests/waskin -B out/waskin-tests -A Win32
cmake --build out/waskin-tests --config Release --target headamp_cpp --parallel 4
& out/waskin-tests/Release/headamp_cpp.exe ../Skin/waskin/HeadAMP.wal out/test-artifacts/wal-headamp/cpp
```

打开可操作的 C++ 原型窗口：

```powershell
& out/waskin-tests/Release/headamp_cpp.exe ../Skin/waskin/HeadAMP.wal out/test-artifacts/wal-headamp/cpp --ui
```

这里的播放、音量、定位等控制测试后端状态，不会播放音频。点击皮肤最小化／关闭按钮可以最小化／关闭原型窗口。

XP 工具链复现（依赖沿用本地现有缓存）：

```powershell
cmake -S tests/waskin -B out/headamp-cpp-xp -A Win32 `
  -DWASKIN_PROBE_LEGACY=ON `
  -DFETCHCONTENT_SOURCE_DIR_TTPLAYER_YY_THUNKS=D:/Projects/Backup/TTPlayer/rebuild/out/compat-deps/yy `
  -DFETCHCONTENT_SOURCE_DIR_TTPLAYER_VC_LTL=D:/Projects/Backup/TTPlayer/rebuild/out/compat-deps/ltl
cmake --build out/headamp-cpp-xp --config Release --target headamp_cpp --parallel 4
& out/headamp-cpp-xp/Release/headamp_cpp.exe ../Skin/waskin/HeadAMP.wal out/test-artifacts/wal-headamp/cpp-xp
```
