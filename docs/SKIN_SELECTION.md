# 原生与 Plugin Skin 选择分别保存

配置文件 `TTPlayerRebuild.xml` 的 `Skin` 节点使用两个属性：

```xml
<Skin PackageName="TT2012.skn" CustomPackageName="waskin\classic.wsz"/>
```

- `PackageName`：原生皮肤。插件未接管的窗口（例如歌词窗口、桌面歌词工具栏）继续使用此皮肤的资源。
- `CustomPackageName`：插件皮肤选择，必须是相对于 `Skin` 的完整路径。由对应 DLL 的目录声明和后缀声明决定是否能够加载。

选择 Winamp 时只修改第二个属性，保留原生选择及已经加载的原生皮肤资源。启动时先加载原生皮肤，再让 DLL 接管主窗、播放列表和均衡器。原生皮肤没有均衡器布局时，也不会为了显示 Winamp 均衡器而替换整个原生皮肤。

应用原生皮肤后清空 Winamp 选择，包括重新选择当前记住的同一原生皮肤或默认皮肤：

```xml
<Skin PackageName="TT2012.skn" CustomPackageName=""/>
```

写入空属性会清除插件选择。无效皮肤切换不改变当前成功加载的选择。保存配置时删除已废弃的 `WinampPackageName` 属性。

## 旧字段与失败恢复

- 按此次要求，不保留旧插件配置的读取兼容：`WinampPackageName` 被忽略；写在 `PackageName`／`SkinFile` 中的旧插件选择也不迁移。
- `CustomPackageName` 缺失、为空或路径非法时，不从旧属性补回。只写文件名的旧选择不再被推断到 `waskin`。
- DLL 暂时缺失或 Winamp 包读取失败时，显示已选原生皮肤，并保留独立的 Winamp 请求；用户应用原生皮肤则明确清空该请求。
- 原生包本身缺失时才回退到内置默认原生皮肤。
- 运行 Winamp 时，窗口布局保存到该 Winamp 包的侧边配置（例如 `Skin/waskin/classic.wsz.xml`），不把 Winamp 的窗口尺寸写入原生包的 `.skn.xml`。

## DLL 定义名称、目录和类型

`TtpSkinPlugin` 返回：

```cpp
name = L"Winamp";           // 标签与菜单分组名称
skin_directory = L"waskin"; // 相对于 EXE 旁的 Skin 目录
extensions = L".wsz;.wal"; // 分号分隔，忽略大小写
```

宿主分别为各插件生成标签和菜单，并按插件身份过滤皮肤。扫描、拖入安装、命令行导入、启动恢复和侧边配置均使用 DLL 声明的目录；另一个插件可以声明自己的名称、子目录和扩展名。

子目录不得是绝对路径、上级跳转或原生 `Skin/new` 根目录；`.skn`、`.zip` 保留给原生皮肤。声明无效或缺少目录／类型声明的旧 DLL 不会成为当前宿主的可用皮肤插件，需要配套更新 DLL。DLL 仍支持旧宿主按 v1 前缀长度获取接口，接口新增字段不会越界写入旧缓冲区。

本地测试覆盖旧字段不迁移、新字段保存与清空、双插件名称与目录／后缀声明、重启恢复、歌词资源保持、无 DLL 后备和换肤失败恢复。测试位于 `rebuild/tests/waskin`，不分发、不在 Actions 执行。

### 本次验证（2026-09-19）

- 普通版与 XP／Win7 版 Release 构建、两版完整宿主测试、DLL 独立测试通过。
- 额外测试 DLL 声明 `Local Test Skin`、`provider-fixture`、`.testskin`，验证两组标签／菜单独立、只扫描声明目录、新扩展名及配置路径正常保存。
- 缺少／无效 DLL 的回退测试和原生 `skin_profile_tests` 通过。
- 统一 DLL 已安装到两版 `Release/AddIn`，三处 SHA-256 相同；XP／Win7 静态导入审计通过。运行测试使用当前 Windows 环境，尚未进行 XP／Win7 真机或虚拟机测试。
