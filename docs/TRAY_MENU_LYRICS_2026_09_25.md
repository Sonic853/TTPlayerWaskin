# 系统栏右键与桌面歌词切换修复

## 结论

系统栏右键无响应是主程序与皮肤插件之间的消息路由问题。修复在 `rebuild/src/ui/player_window.cpp` 的 `PlayerWindow::HandleTrayCallback`，现有 `ttp_waskin.dll` 可直接使用，无需增加 DLL 接口。标题区保持原样。

## 原因与原版对照

原版伪代码位于 `reverse/decompiled/TTPlayer.exe.pseudo.c`：

- `FUN_0046fe1e` 收到 `WM_RBUTTONUP (0x205)` 后获取鼠标位置、激活主窗口，并向主 HWND 发送 `WM_CONTEXTMENU (0x7b)`；随后发送 `WM_NULL`，使菜单支持点击外部关闭。
- `CPlayerWnd_OnContextMenu (00460d88)` 判断目标为主 HWND，退出全屏并调用 `CPlayerWnd_ShowMainContextMenu`。
- `FUN_0044d48d` 返回窗口歌词时解除桌面歌词锁定、隐藏桌面绘制面、显示保留的歌词 HWND，并恢复当前正常/迷你模式的歌词可见状态。

重建版此前沿用了原版的系统栏消息转发。但是插件会对主 HWND 增加子类处理，消息会先经过 DLL：

1. `rebuild` 收到系统栏回调，重新发送携带屏幕坐标的 `WM_CONTEXTMENU`。
2. WSZ 的 `Skin::Message` 在该分支中只为 `lParam == -1` 的键盘请求发出菜单命令，其他请求直接返回 `0`。
3. 宿主因此收不到这次菜单请求。用户在皮肤上直接右键使用的是另一条 `WM_RBUTTONUP → TTP_SKIN_MENU` 路径，所以可能只有系统栏失效。
4. WAL 的 InlineAVS 内容也处理 `WM_CONTEXTMENU`；如果坐标落入内容矩形，系统栏请求可能被当成歌词/视觉区域的右键。这同样说明系统栏菜单不应依赖皮肤区域命中判断。

修复前的本地 WSZ 集成测试确实失败：`init=0 draw=0`，即没有创建、绘制弹出菜单。

## 修复方式

保留系统栏回调解码、图标 ID 校验、键盘坐标回退和退出过程保护，将重新发送 `WM_CONTEXTMENU` 改为直接调用宿主 `ShowContextMenu(point)`，并保留全屏退出及 `WM_NULL` 处理。

这样仍使用原来的主菜单构建、动态子菜单、菜单绘制和命令分发，但不会再次经过 DLL 的鼠标消息筛选。只打开或取消系统栏菜单不会显示已经隐藏/最小化的主窗口。

桌面歌词继续走宿主已有逻辑：

- `0x8039`：保存歌词窗几何，隐藏窗口歌词，显示桌面歌词。
- `0x8040 / 0x8041`：锁定/解锁桌面歌词。锁定后仍能通过系统栏操作。
- `0x8038`：先恢复隐藏或最小化的主窗口，再隐藏桌面歌词并恢复插件歌词窗口。
- 桌面歌词工具栏的返回命令与系统栏菜单共用该路径。

## 验证

本地 C++ 测试：`rebuild/tests/waskin/tray_lyrics_host_tests.inc`，通过 `waskin_host_tests --tray-only` 运行；加 `--modern-only` 使用 HeadAMP，加 `--no-plugin` 检查原生皮肤。

测试观测真实的 `WM_INITMENUPOPUP` 和菜单 `WM_DRAWITEM`，检查菜单确实包含当前模式下可用的歌词命令，再用菜单的标准 `WM_COMMAND` 通知路径执行命令。测试没有注入鼠标点击。

| 检查 | 结果 |
| --- | --- |
| WSZ、HeadAMP、原生皮肤 | 均通过 |
| 主窗口正常、最小化、隐藏时右键 | 正常创建并绘制主菜单；打开/取消菜单不恢复窗口 |
| 传统 `WM_RBUTTONUP` / `WM_CONTEXTMENU` 回调 | 通过 |
| v4 打包坐标和键盘 `(-1,-1)` 回调 | 通过；WAL 菜单锚点落入 InlineAVS 内容时仍打开主菜单 |
| 窗口歌词 → 桌面歌词 → 窗口歌词 | 通过；两种歌词面互斥，返回后位置和大小一致 |
| 桌面歌词锁定/解锁 | 通过；菜单随状态提供对应命令 |
| 普通最小化、最小化到系统栏、关闭到系统栏后返回 | 当前桌面上三种皮肤均通过；Explorer 确实接受了测试图标 |
| 桌面歌词工具栏返回 | 通过 |
| WSZ / WAL 上一轮窗口几何回归 | 通过；覆盖重复进入桌面模式、吸附/分离拖动、保存和重新排列 |
| Release 编译及 XP / Win7 静态导入检查 | 主程序、更新器通过；未新增系统 API |

沙箱中 Explorer 不接受测试图标，因此先检查了隐藏/最小化状态；随后在当前桌面重新运行，日志为 `Explorer accepted test icon=1`，完成了真正的系统栏隐藏策略验证。本轮没有运行 XP / Win7 虚拟机。

所有测试和日志留在本地，不上传、不进入发行包，Action 保持 `BUILD_TESTING=OFF`。日志与原产物备份位于 `rebuild/out/test-artifacts/tray-lyrics-fix/`。

## 本地产物

- 主程序：`rebuild/build/Release/TTPlayerRebuild.exe`。
- 通用发行包：`rebuild/build/Release/TTPlayerRebuild-2026.09.25.zip`。
- 包含主程序、更新器、已验证的 HTTPS 组件及内部校验清单；ZIP CRC、内部和外部 SHA-256 均已验证。
- `rebuild/build/Release/AddIn/ttp_waskin.dll` 保留现有版本，源码与接口均未改动。

| 文件 | SHA-256 |
| --- | --- |
| `TTPlayerRebuild.exe` | `4d62e806345956cf074c5735d9f1b67e46ccf11e9592fbd1df631d7df6aa76f3` |
| `TTPlayerRebuild-2026.09.25.zip` | `cc38217fb9dcff546c5012b548026297f02b07b4ad6b9ddf9243fc14a2d0cf7a` |
