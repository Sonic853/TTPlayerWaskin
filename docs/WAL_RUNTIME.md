# WAL 与独立 MAKI VM 接入

原版的解析流程、全部控件分类及当前实现差距见 [WAL 控件源码分析](WAL_CONTROL_PARSING_ANALYSIS.md)；逐项标签、属性和 MAKI 方法见 [控件参考清单](WAL_CONTROL_REFERENCE.md)。

## 交付结构

```text
makivm/                         独立工程，生成 ttp_maki.dll
  include/ttp_maki.h             VM 的 C ABI
  src/vm.cpp                    解释器
waskin/                         独立工程，生成 ttp_waskin.dll
  src/maki_client.h             加载同目录 VM、校验 ABI、管理引用
  src/modern_objects.h          XML 和界面对象
  src/modern.cpp                现代布局、PNG、控件、原生方法和播放器桥接
  src/skin.cpp                  经典 WSZ 及缺失窗口的默认皮肤
```

运行时两份 DLL 都放在 `AddIn`。两版播放器共用这两份 x86 DLL；两工程可独立构建，waskin 不链接 maki 的导入库。

## 可见性与加载

1. 按 `ttp_waskin.dll` 自身的绝对路径寻找同目录 `ttp_maki.dll`。
2. 缺失、损坏、位数错误、入口缺失或 ABI 不匹配时，对宿主只声明 `.wsz`。WAL 探测/创建也会失败，WSZ 和内置默认皮肤仍可用。
3. VM 可用时声明 `.wsz;.wal`；核心 ZIP/XML/图片和主布局仍检查，缺失的可选资源、未知控件和不兼容脚本可降级跳过；`EQ_AUTO` 等未实现动作保留图片但不可点击。
4. 皮肤实例持有 VM 模块引用，先销毁全部脚本再卸载 VM。插件扫描将 VM 识别为运行时依赖，不当作音频插件报缺少入口。
5. 安装或移除 VM 后重启播放器，以刷新已缓存的提供方扩展名。

## HeadAMP 已接入内容

- 原始 XML 和 PNG，按像素裁剪；透明区域形成异形窗口，按绘制顺序和 alpha 命中控件。
- 每个 script 标签创建独立实例；执行原包的三份主布局 MAKI。未执行包中的标准窗口模板脚本。
- 播放/暂停/继续/停止通知、上一首/下一首、音量、平衡、EQ、最小化和关闭。
- 抽屉动画及 onTargetReached；按实际经过时间推进，定时器延迟不会让动画持续变慢。拖动进度条只预览，松开时通过千千命令提交一次。
- 内嵌真实播放列表：标题、选择、双击、滚轮、原生右键、歌曲信息提示、外部文件插入和原生 OLE 拖出。颜色取 WAL 的系统颜色定义。
- guid:avs 和 Vis 使用千千已有视觉回调，不实现 Winamp AVS 引擎或其预设。
- 主窗口位置、HeadAMP 两侧抽屉和视觉切换状态持久化；独立播放列表、均衡器和歌词窗口继续使用内置默认皮肤及已有功能。
- 宿主只补通用内嵌列表拖放路由、拖出来源以及依赖 DLL 扫描。XML、MAKI 和 HeadAMP 对象均不进入 EXE。

## 当前范围

当前为可扩展的 WAL 子集。57 个本地 WAL 均可识别，54 个允许完整或部分加载，并通过挂接、预览及布局保存恢复检查；这不代表所有外观与功能均已恢复。原有 8 个样本保留主要交互回归，并新增真实 Diablo II 的 EQ_AUTO 禁用验证。最新规则、XP 修复和剩余限制见 [部分加载说明](WAL_PARTIAL_LOADING_2026_09_24.md)；此前源码分析见 [新增皮肤检查报告](WAL_NEW_SKINS_AUDIT_2026_09_24.md)。

XML/布局、AnimatedLayer、Map、鼠标事件和原生播放桥接在 ttp_waskin.dll；MAKI 字节码在独立 ttp_maki.dll。未引入 Wasabi 服务。复杂多容器、系统模板、部分控件/配置/FX 仍未完成。

脚本初始化期间不发送播放等命令；运行失败保留诊断。ZIP、XML、图片、对象数、递归和指令预算有界。

## 验证与构建

在 TTPlayer 根目录分别构建：

```powershell
cmake -S makivm -B makivm/build -A Win32
cmake --build makivm/build --config Release --target ttp_maki --parallel 4
cmake -S waskin -B waskin/build -A Win32
cmake --build waskin/build --config Release --target ttp_waskin --parallel 4
```

本地 C++ 测试均在 `rebuild/tests/waskin`，不上传、不加入 Action：

- `maki_vm_tests.cpp`：独立 DLL ABI、声明类型转换、比较、类 GUID 签名、原生回调重入、解释执行及畸形字节码。
- `modern_dll_tests.cpp`：VM 缺失/损坏、HeadAMP 界面、命令、动画、进度、预览及布局恢复；包含 GUI 线程延迟 350 ms 后只投递一次定时器的回归场景。
- `modern_host_tests.inc`：普通与旧系统两套真实宿主代码，验证目录扫描、760×394 布局、真实设置/选择、CF_HDROP 在光标位置插入、保存恢复、原生/WSZ/WAL 切换。
- 原有 WSZ 控件、工具提示、默认皮肤、视频、ABI 尾部与反复销毁回归。

两份 DLL 通过 XP/Win7 静态导入检查；现已补充 VirtualBox XP、Win7 SP1 来宾中的 DLL 交互及主程序集成回归，结果见 [部分加载与 XP 修复](WAL_PARTIAL_LOADING_2026_09_24.md)。分析依据和原型记录见 [原始 HeadAMP 分析](WAL_NO_WASABI_HEADAMP.md)。

### 拆分后的校正

对照本地 Winamp `Src/Wasabi/api/script/objects/guiobj.cpp` 的 `guiobject_setTargetSpeed`、`guiobject_gotoTarget` 和 `onTargetTimer`：速度保持 250 ms 的量化单位，首次定时器建立起始时间，再用 `GetTickCount` 的经过时间计算 0～255 的正弦缓动进度；无符号时间差可跨越计数回绕。恢复保存的抽屉状态时直接完成目标，避免恢复过程停留在半展开位置。

该轮回归先在修正前 DLL 上复现整数赋值丢失声明类型、延迟定时器导致抽屉未展开的问题，再验证修正后的 DLL。后续部分加载和 XP 动态初始化修复见上文的最新说明。
