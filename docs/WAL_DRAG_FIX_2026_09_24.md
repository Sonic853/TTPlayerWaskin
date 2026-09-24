# WAL 窗口无法拖动修复

日期：2026-09-24。

## 原因和原版行为

原版 Winamp 的 `Src/Wasabi/api/skin/widgets/layer.cpp` 默认将 Layer 设置为可移动对象，并使用矩形区域；`api/script/objects/guiobj.cpp` 的 `guiobject_onLeftButtonDown` 先分发 MAKI 事件，再判断对象及布局是否允许移动。脚本通过完成事件可以阻止默认移动。

重建版 WAL 的按下处理提前执行 `SetCapture`，随后调用主程序的 `BeginSkinBackgroundDrag`。主程序再次捕获同一窗口时，同步触发 `WM_CAPTURECHANGED`，插件立即将刚开始的拖动取消。旧 DLL 的 HeadAMP 背景拖动测试可稳定复现：鼠标按下后已经收到结束回调，首次移动不起作用。

松开鼠标时也有类似的重入问题：主程序释放捕获会再次进入插件的捕获变化处理；如果尚未清除移动标志，就会重复发送拖动结束。

## 修复

- 使用宿主拖动接口时，由主程序统一捕获鼠标；插件不再提前捕获。沿用千千现有吸附、分离和窗口联动逻辑。
- 结束拖动前先清除插件的拖动状态，避免释放捕获时重入并重复结束。取消操作也会正确释放捕获。
- MAKI 按下事件仍先执行，保留脚本阻止默认移动的行为。已开始的拖动在后续鼠标移动时持续处理。
- 显式设置 `move` 的无图片 Layer 参与矩形命中，分别遵守 `move="0"` 和 `move="1"`。未声明该属性的无图片坐标辅助层保持原有处理，避免遮挡实际控件。
- 无可用宿主拖动回调时，插件按屏幕坐标移动自己的窗口。按钮、滑块和播放列表继续使用各自的鼠标处理。

改动位于 `waskin/src/modern.cpp`，未引入 Wasabi 服务。

## 验证

本地 C++ 测试在 `rebuild/tests/waskin`，不上传、不加入 Actions 或发布包。

- 旧 DLL 复现失败，新 DLL 通过相同断言。
- HeadAMP、Diablo II：首次移动立即响应，后续移动连续，释放及取消各结束一次。
- 合成皮肤：无图片 Layer 的 `move=0/1`，以及无宿主回调时的本地移动。
- 普通版和 XP／Win7 版真实主程序集成：窗口移动、吸附均衡器同步跟随、释放/取消、冷启动、MAKI 音量、原生列表选择、CF_HDROP 插入和皮肤切换均通过。
- 原有 8 个 WAL 样本的主要交互及部分加载回归通过。
- XP/Win7 静态导入检查通过（8 个 DLL、285 个导入）。

日志位于 `rebuild/out/test-artifacts/wal-drag`。本轮针对拖动及相关交互验证，未重新宣称全部 WAL 的外观与功能兼容。

VirtualBox Windows XP（5.1.2600）及 Windows 7 SP1 的真实主程序集成也均通过，包含上述新增拖动/吸附/释放/取消断言和已有交互回归。结果分别保存为 `xp-host-final.txt` 和 `win7-host-final.txt`。

## 交付

以下两处已更新为同一份共享 DLL：

- `rebuild/build/Release/AddIn/ttp_waskin.dll`
- `rebuild/out/legacy-xp/Release/AddIn/ttp_waskin.dll`

文件为 703488 字节，SHA-256：`542b35fd3bc58451c6ea5bd3a2048523dceec1c130883251ac2202b6ab936353`。

原 DLL 备份在 `rebuild/out/test-artifacts/before-wal-drag-20260924-172430`，交付记录在 `wal-drag/deployment.json`。沿用现有 EXE 和 `ttp_maki.dll`。
