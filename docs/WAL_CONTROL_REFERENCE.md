# WAL 控件源码索引、属性声明与 MAKI 导出清单

> 本文的“当前实现”描述为本轮修复前快照。最新状态请看 [WAL 扩展与启动修复](WAL_EXPANSION_FIXES_2026_09_24.md)。

日期：2026-09-24。解释与实现差距见 [WAL_CONTROL_PARSING_ANALYSIS.md](WAL_CONTROL_PARSING_ANALYSIS.md)。

本清单由本地源码声明提取，并结合工厂、编译配置、项目文件和关键 setter 复核。它是源码快照索引，不是运行时兼容认证；未编译 Winamp 本体。

## 1. 66 项 C++ 标签声明

计数按标签/工厂声明；Group/CfgGroup 属于解析器专门处理路径，Component 是别名，旧版与调试项也列出。默认“Widgets 注册”仍受对应 WASABI_WIDGETS_* 编译宏约束。

| 标签 | C++ 对象 | 注册／范围说明 | 标签源码 |
| --- | --- | --- | --- |
| `AddParams` | `AddParams` | Widgets 注册；以 gen_ff 编译宏为准 | [xuiaddparams.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiaddparams.cpp:7>) |
| `AlbumArt` | `AlbumArt` | gen_ff 附加服务；脚本类名 AlbumArtLayer | [AlbumArt.cpp:15](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/AlbumArt.cpp:15>) |
| `AnimatedLayer` | `AnimatedLayer` | Widgets 注册；以 gen_ff 编译宏为准 | [animlayer.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/animlayer.cpp:7>) |
| `BookmarkList` | `BookmarkList` | 类和标签存在；缺常规注册，set() 为占位 | [xuibookmarklist.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuibookmarklist.cpp:5>) |
| `Browser` | `ScriptBrowserWnd` | Widgets 注册；以 gen_ff 编译宏为准 | [xuibrowser.cpp:4](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mb/xuibrowser.cpp:4>) |
| `Button` | `Wasabi::Button` | Widgets 注册；以 gen_ff 编译宏为准 | [button.cpp:18](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/button.cpp:18>) |
| `CfgGroup` | `CfgGroup` | 由 SkinParser 专门处理；配置构建相关 | [group.cpp:31](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/group.cpp:31>) |
| `ColorThemes:List` | `ColorThemesList` | 静态声明与颜色主题构建相关 | [xuithemeslist.cpp:41](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuithemeslist.cpp:41>) |
| `ColorThemes:Mgr` | `NakedColorThemesList` | 与主题列表同源；无可视 NakedObject | [xuithemeslist.cpp:43](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuithemeslist.cpp:43>) |
| `Component` | `XuiWindowHolder` | WindowHolder 兼容别名 | [xuiwndholder.cpp:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiwndholder.cpp:9>) |
| `ComponentBucket` | `ComponentBucket2` | Widgets 注册；以 gen_ff 编译宏为准 | [compbuck2.cpp:26](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/compbuck2.cpp:26>) |
| `CustomObject` | `XuiCustomObject` | Widgets 注册；以 gen_ff 编译宏为准 | [xuicustomobject.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuicustomobject.cpp:6>) |
| `DownloadsList` | `DownloadsList` | 源文件静态 DECLARE_SERVICE，gen_ff.vcxproj 直接编译 | [xuidownloadslist.cpp:22](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuidownloadslist.cpp:22>) |
| `Edit` | `Edit` | Widgets 注册；以 gen_ff 编译宏为准 | [edit.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/edit.cpp:11>) |
| `EqBand` | `SEQBand` | Widgets 注册；以 gen_ff 编译宏为准 | [seqband.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/seqband.cpp:7>) |
| `EQPreAmp` | `SEQPreamp` | Widgets 注册；以 gen_ff 编译宏为准 | [seqpreamp.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/seqpreamp.cpp:6>) |
| `EQVis` | `SEQVis` | Widgets 注册；以 gen_ff 编译宏为准 | [seqvis.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/seqvis.cpp:11>) |
| `Gradient` | `XuiGradientWnd` | Widgets 注册；以 gen_ff 编译宏为准 | [xuigradientwnd.h:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuigradientwnd.h:9>) |
| `Grid` | `Grid` | Widgets 注册；以 gen_ff 编译宏为准 | [xuigrid.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuigrid.cpp:6>) |
| `Group` | `Group` | 由 SkinParser 专门处理；同时存在 XUI 声明 | [group.cpp:27](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/group.cpp:27>) |
| `GroupList` | `GroupList` | Widgets 注册；以 gen_ff 编译宏为准 | [grouplist.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/grouplist.cpp:7>) |
| `GroupXFade` | `GroupXFade` | 静态服务声明；gen_ff servicelink 引用 | [xuigroupxfade.h:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuigroupxfade.h:9>) |
| `GuiObject` | `GuiObjectWnd` | Widgets 注册；以 gen_ff 编译宏为准 | [guiobj.cpp:28](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/script/objects/guiobj.cpp:28>) |
| `HideObject` | `HideObject` | Widgets 注册；以 gen_ff 编译宏为准 | [xuihideobject.cpp:4](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuihideobject.cpp:4>) |
| `images` | `Wa2Slider` | _WIN32 下注册；旧式帧图滑块 | [xuiwa2slider.cpp:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/wa2/xuiwa2slider.cpp:10>) |
| `Layer` | `Layer` | Widgets 注册；以 gen_ff 编译宏为准 | [layer.cpp:30](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/layer.cpp:30>) |
| `LayoutStatus` | `XuiStatus` | Widgets 注册；以 gen_ff 编译宏为准 | [xuistatus.cpp:4](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuistatus.cpp:4>) |
| `List` | `ScriptList` | Widgets 注册；以 gen_ff 编译宏为准 | [xuilist.cpp:14](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuilist.cpp:14>) |
| `Menu` | `XuiMenu` | Widgets 注册；以 gen_ff 编译宏为准 | [xuimenu.cpp:15](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuimenu.cpp:15>) |
| `MouseRedir` | `MouseRedir` | Widgets 注册；以 gen_ff 编译宏为准 | [mouseredir.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mouseredir.cpp:8>) |
| `NStatesButton` | `NStatesTgButton` | Widgets 注册；以 gen_ff 编译宏为准 | [tgbutton.cpp:160](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/tgbutton.cpp:160>) |
| `ObjDirView` | `ScriptObjDirWnd` | Widgets 注册；以 gen_ff 编译宏为准 | [xuiobjdirwnd.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiobjdirwnd.cpp:5>) |
| `OSWndHost` | `XuiOSWndHost` | Widgets 注册；以 gen_ff 编译宏为准 | [xuioswndhost.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuioswndhost.cpp:8>) |
| `PanBar` | `SPanBar` | Widgets 注册；以 gen_ff 编译宏为准 | [spanbar.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/spanbar.cpp:5>) |
| `PlaylistDirectory` | `PlDirObject` | gen_ff 附加服务；播放列表目录 | [wa2pldirobj.cpp:17](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/wa2pldirobj.cpp:17>) |
| `PlaylistEditor` | `Wa2PlaylistEditor` | gen_ff 附加服务；播放器队列数据 | [wa2pledit.cpp:23](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/wa2pledit.cpp:23>) |
| `ProgressGrid` | `ProgressGrid` | 静态服务声明；gen_ff servicelink 引用 | [xuiprogressgrid.cpp:33](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiprogressgrid.cpp:33>) |
| `QueryDrag` | `QueryDrag` | 旧数据库控件；gen_ff 查询控件宏未开启 | [xuiquerydrag.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiquerydrag.cpp:7>) |
| `QueryLine` | `ScriptQueryLine` | 旧数据库控件；使用旧式 addParam 声明 | [xuiqueryline.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiqueryline.cpp:8>) |
| `QueryResults` | `ScriptQueryList` | 旧数据库控件；gen_ff 查询控件宏未开启 | [xuiquerylist.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiquerylist.cpp:8>) |
| `Rect` | `ScriptRect` | Widgets 注册；以 gen_ff 编译宏为准 | [xuirect.h:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuirect.h:10>) |
| `SeekBar` | `SSeeker` | Widgets 注册；以 gen_ff 编译宏为准 | [sseeker.cpp:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sseeker.cpp:10>) |
| `SendParams` | `SendParams` | Widgets 注册；以 gen_ff 编译宏为准 | [xuisendparams.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuisendparams.cpp:6>) |
| `Shadow` | `XuiShadowWnd` | 旧 imggen 组件声明；非普通 Widgets 注册链 | [shadowwnd.h:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/imgldr/imggen/shadowwnd.h:10>) |
| `Slider` | `PSliderWnd / media subclasses` | Widgets 注册；以 gen_ff 编译宏为准 | [pslider.cpp:315](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/pslider.cpp:315>) |
| `SongTicker` | `SongTicker` | gen_ff 附加服务；派生 Text | [wa2songticker.cpp:22](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/wa2songticker.cpp:22>) |
| `Status` | `SStatus` | Widgets 注册；以 gen_ff 编译宏为准 | [sstatus.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sstatus.cpp:6>) |
| `SvcWnd` | `SvcWnd` | 旧服务窗口；gen_ff 的 WASABI_WIDGETS_SVCWND 被注释 | [svcwnd.cpp:18](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/script/objects/svcwnd.cpp:18>) |
| `Text` | `Text` | Widgets 注册；以 gen_ff 编译宏为准 | [text.cpp:33](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/text.cpp:33>) |
| `TitleBar` | `Title` | Widgets 注册；以 gen_ff 编译宏为准 | [title.cpp:13](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/title.cpp:13>) |
| `ToggleButton` | `ToggleButton` | Widgets 注册；以 gen_ff 编译宏为准 | [tgbutton.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/tgbutton.cpp:6>) |
| `Tree` | `ScriptTree` | Widgets 注册；以 gen_ff 编译宏为准 | [xuitree.cpp:139](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitree.cpp:139>) |
| `Vis` | `SAWnd` | Widgets 注册；以 gen_ff 编译宏为准 | [sa.cpp:13](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sa.cpp:13>) |
| `VolBar` | `SVolBar` | Widgets 注册；以 gen_ff 编译宏为准 | [svolbar.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/svolbar.cpp:5>) |
| `Wasabi:CheckBox` | `ScriptCheckBox` | Widgets 注册；以 gen_ff 编译宏为准 | [xuicheckbox.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuicheckbox.cpp:7>) |
| `Wasabi:ComboBox` | `ComboBox` | Widgets 注册；以 gen_ff 编译宏为准 | [xuicombobox.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuicombobox.cpp:5>) |
| `Wasabi:DropDownList` | `DropDownList` | Widgets 注册；以 gen_ff 编译宏为准 | [xuidropdownlist.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuidropdownlist.cpp:5>) |
| `Wasabi:EditBox` | `EditBox` | 旧 C++ 包装声明；系统 XML 另有同名 xuitag，模板查找优先 | [xuieditbox.cpp:4](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuieditbox.cpp:4>) |
| `Wasabi:Frame` | `ScriptFrame` | Widgets 注册；以 gen_ff 编译宏为准 | [xuiframe.cpp:6](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiframe.cpp:6>) |
| `Wasabi:HistoryEditBox` | `HistoryEditBox` | Widgets 注册；以 gen_ff 编译宏为准 | [xuihistoryedit.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuihistoryedit.cpp:5>) |
| `Wasabi:PathPicker` | `ScriptPathPicker` | Widgets 注册；以 gen_ff 编译宏为准 | [xuipathpicker.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuipathpicker.cpp:5>) |
| `Wasabi:RadioGroup` | `ScriptRadioGroup` | Widgets 注册；以 gen_ff 编译宏为准 | [xuiradiogroup.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiradiogroup.cpp:7>) |
| `Wasabi:Stats` | `XuiStats` | 仅 WASABI_COMPILE_STATSWND；gen_ff 调试条件 | [xuistats.cpp:24](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/stats/xuistats.cpp:24>) |
| `Wasabi:TabSheet` | `ScriptTabSheet` | Widgets 注册；以 gen_ff 编译宏为准 | [xuitabsheet.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitabsheet.cpp:8>) |
| `Wasabi:TitleBox` | `ScriptTitleBox` | Widgets 注册；以 gen_ff 编译宏为准 | [xuititlebox.cpp:5](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuititlebox.cpp:5>) |
| `WindowHolder` | `XuiWindowHolder` | Widgets 注册；以 gen_ff 编译宏为准 | [xuiwndholder.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiwndholder.cpp:8>) |

## 2. 随附 XML 定义的 25 种 XUI 标签

实际存在 44 处定义，若同名模板出现在不同皮肤或系统资源中，它们不会同时全部启用。`Wasabi:EditBox` 与上表重叠；66+25 不能当作运行时控件总数。第三方 WAL 可以继续增加名称。

`embed_xui` 是向内部控件转发属性／接口的目标。空值表示该定义未指定，并不表示没有脚本逻辑。每行给出首个来源及该名称的所有源码定义位置。

| 标签 | Group ID | 首个定义的 embed_xui | 全部定义位置 |
| --- | --- | --- | --- |
| `Bento:InfoLine` | `bento.infodisplay.line` | `text` | [1. Big Bento/player-normal-mcv.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/player-normal-mcv.xml:556>)；[2. Bento/player-normal-mcv.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/player-normal-mcv.xml:556>) |
| `Bento:TabButton` | `bento.tabbutton` | `bento.tabbutton.mousetrap` | [1. Big Bento/player-normal-sui.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/player-normal-sui.xml:239>)；[2. Bento/player-normal-sui.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/player-normal-sui.xml:237>) |
| `menu:button_hover` | `menu.button.hover` | — | [1. Winamp Modern/window_menus.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/window_menus.xml:7>) |
| `menu:button_normal` | `menu.button.normal` | — | [1. Winamp Modern/window_menus.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/window_menus.xml:3>) |
| `menu:button_pressed` | `menu.button.pressed` | — | [1. Winamp Modern/window_menus.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/window_menus.xml:13>) |
| `PlaylistPro` | `PlaylistPro.xui` | `wdh.playlist` | [1. Winamp Modern/playlistpro.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/xml/playlistpro.xml:54>)；[2. Big Bento/playlistpro.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/playlistpro.xml:54>)；[3. Bento/playlistpro.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/playlistpro.xml:54>) |
| `Wasabi:Button` | `wasabi.button.group` | `wasabi.button` | [1. xui/button.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/button/button.xml:16>) |
| `Wasabi:EditBox` | `wasabi.edit` | `wasabi.edit.box` | [1. xui/editbox.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/editbox/editbox.xml:7>) |
| `Wasabi:EditBox2` | `wasabi.edits` | `wasabi.edit.box` | [1. Winamp Modern/playlistpro.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/xml/playlistpro.xml:31>)；[2. Big Bento/playlistpro.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/playlistpro.xml:31>)；[3. Bento/playlistpro.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/playlistpro.xml:31>) |
| `Wasabi:Editor` | `wasabi.multiline.edit` | `wasabi.edit.box` | [1. xui/editbox.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/editbox/editbox.xml:17>) |
| `Wasabi:HSlider` | `wasabi.slider.horizontal` | `slider.button` | [1. xui/slider.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/slider/slider.xml:7>) |
| `Wasabi:IconButton` | `wasabi.itb.xui` | `itb.button` | [1. xui/icontextbutton.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/browser/icontextbutton.xml:1>) |
| `Wasabi:MainFrame:NoStatus` | `wasabi.mainframe.nostatusbar` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:206>) |
| `Wasabi:MediaLibraryFrame:NoStatus` | `wasabi.medialibraryframe.nostatusbar` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:220>) |
| `Wasabi:PlaylistFrame:NoStatus` | `wasabi.playlistframe.nostatusbar` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:213>) |
| `Wasabi:StandardFrame:Modal` | `wasabi.standardframe.modal` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:502>)；[2. Big Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/standardframe.xml:133>)；[3. Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/standardframe.xml:134>)；[4. xui/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/standardframe/standardframe.xml:307>) |
| `Wasabi:StandardFrame:NoStatus` | `wasabi.standardframe.nostatusbar` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:450>)；[2. Big Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/standardframe.xml:115>)；[3. Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/standardframe.xml:116>)；[4. xui/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/standardframe/standardframe.xml:223>) |
| `Wasabi:StandardFrame:Static` | `wasabi.standardframe.static` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:515>)；[2. Big Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/standardframe.xml:147>)；[3. Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/standardframe.xml:148>)；[4. xui/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/standardframe/standardframe.xml:326>) |
| `Wasabi:StandardFrame:Status` | `wasabi.standardframe.statusbar` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:388>)；[2. Big Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Big Bento/xml/standardframe.xml:97>)；[3. Bento/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Bento/xml/standardframe.xml:98>)；[4. xui/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/standardframe/standardframe.xml:127>) |
| `Wasabi:Text` | `wasabi.text.group` | `wasabi.text` | [1. xui/text.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/text/text.xml:16>) |
| `Wasabi:TitleBar` | `wasabi.titlebar` | `window.titlebar.title` | [1. Winamp Modern/titlebar.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/titlebar/titlebar.xml:19>)；[2. xui/titlebar.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/titlebar/titlebar.xml:19>) |
| `Wasabi:ToggleButton` | `wasabi.togglebutton.group` | `wasabi.button` | [1. xui/button.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/button/button.xml:27>) |
| `Wasabi:VISFrame:NoStatus` | `wasabi.visframe.nostatusbar` | — | [1. Winamp Modern/standardframe.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/skins/Winamp Modern/standardframe/standardframe.xml:227>) |
| `Wasabi:VSlider` | `wasabi.slider.vertical` | `slider.button` | [1. xui/slider.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/slider/slider.xml:47>) |
| `Winamp:Browser` | `winamp.xui.browser` | `webbrowser` | [1. xui/browser.xml](<D:/Projects/Backup/TTPlayer/winamp/Src/resources/data/freeform/xml/wasabi/xml/xui/browser/browser.xml:104>) |

## 3. 各类直接声明的 XML 属性

这是各类自身的参数表，必须叠加父类属性。比如 AnimatedLayer 同时继承 Layer 和 GuiObject，Text 同时使用 TextBase。未知 Group 参数还可能转发到内部对象或 onSetXuiParam，不能仅凭此表拒绝一切额外属性。

属性保留源码拼写；多数原生参数名匹配不区分大小写。条件编译内的声明也列出，例如 APPBAR；原代码中的别名和占位也保留。表中出现参数名不保证每种构建都实现其效果，应继续核对链接的 setXuiParam。

### AlbumArt

来源：[AlbumArt.cpp:88](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/AlbumArt.cpp:88>)。

`NOTFOUNDIMAGE`, `SOURCE`, `VALIGN`, `ALIGN`, `STRETCHED`, `NOAUTOREFRESH`

### SongTicker

来源：[wa2songticker.cpp:51](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/wa2songticker.cpp:51>)。

`TICKER`

### ObjectActuator

来源：[objectactuator.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/objectactuator.cpp:7>)。

`GROUP`, `TARGET`

### AnimatedLayer

来源：[animlayer.cpp:88](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/animlayer.cpp:88>)。

`AUTOPLAY`, `AUTOREPLAY`, `DEBUG`, `ELEMENTFRAMES`, `END`, `FRAMEHEIGHT`, `FRAMEWIDTH`, `REALTIME`, `SPEED`, `START`

### Wasabi::Button

来源：[button.cpp:78](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/button.cpp:78>)。

`ACTION`, `ACTION_TARGET`, `ACTIVEIMAGE`, `BORDERS`, `CBTARGET`, `CENTER_IMAGE`, `DOWNIMAGE`, `HOVERIMAGE`, `IMAGE`, `PARAM`, `RETCODE`, `STYLE`, `TEXT`, `TEXTCOLOR`, `TEXTDIMMEDCOLOR`, `TEXTHOVERCOLOR`

### ComponentBucket2

来源：[compbuck2.cpp:29](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/compbuck2.cpp:29>)。

`LEFTMARGIN`, `RIGHTMARGIN`, `SPACING`, `VERTICAL`, `WNDTYPE`

### QueryDrag

来源：[xuiquerydrag.cpp:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiquerydrag.cpp:10>)。

`image`, `source`

### ScriptQueryList

来源：[xuiquerylist.cpp:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiquerylist.cpp:10>)。

`TITLE`

### DropDownList

来源：[dropdownlist.cpp:15](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/dropdownlist.cpp:15>)。

`FEED`, `ITEMS`, `LISTHEIGHT`, `MAXITEMS`, `SELECT`, `ANTIALIAS`

### Edit

来源：[edit.cpp:84](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/edit.cpp:84>)。

`ACTION`, `AUTOENTER`, `AUTOHSCROLL`, `AUTOSELECT`, `MULTILINE`, `PASSWORD`, `TEXT`, `VSCROLL`

### Group

来源：[group.cpp:35](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/group.cpp:35>)。

`AUTOHEIGHTSOURCE`, `AUTOWIDTHSOURCE`, `BACKGROUND`, `DRAWBACKGROUND`, `DEFAULT_W`, `DEFAULT_H`, `DESIGN_H`, `DESIGN_W`, `EMBED_XUI`, `INHERIT_CONTENT`, `INHERIT_GROUP`, `INSTANCEID`, `LOCKMINMAX`, `MAXIMUM_H`, `MAXIMUM_W`, `MINIMUM_H`, `MINIMUM_W`, `NAME`, `PROPAGATESIZE`, `XUITAG`

### HistoryEditBox

来源：[historyeditbox.cpp:4](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/historyeditbox.cpp:4>)。

`NAVBUTTONS`

### Layer

来源：[layer.cpp:33](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/layer.cpp:33>)。

`CURSOR`, `DBLCLICKACTION`, `DBLCLICKPARAM`, `IMAGE`, `INACTIVEIMAGE`, `QUALITY`, `REGION`, `RESIZE`, `SCALE`, `TILE`

### ScriptBrowserWnd

来源：[xuibrowser.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mb/xuibrowser.cpp:7>)。

`MAINMB`, `SCROLLBARS`, `TARGETNAME`, `URL`

### MouseRedir

来源：[mouseredir.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mouseredir.cpp:11>)。

`target`

### PSliderWnd

来源：[pslider.cpp:18](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/pslider.cpp:18>)。

`BARLEFT`, `BARMIDDLE`, `BARRIGHT`, `DOWNTHUMB`, `HIGH`, `HOTPOS`, `HOTRANGE`, `HOVERTHUMB`, `LOW`, `ORIENTATION`, `THUMB`, `STRETCHTHUMB`

### SAWnd

来源：[sa.cpp:78](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sa.cpp:78>)。

`COLORALLBANDS`, `COLORBAND1`, `COLORBAND2`, `COLORBAND3`, `COLORBAND4`, `COLORBAND5`, `COLORBAND6`, `COLORBAND7`, `COLORBAND8`, `COLORBAND9`, `COLORBAND10`, `COLORBAND11`, `COLORBAND12`, `COLORBAND13`, `COLORBAND14`, `COLORBAND15`, `COLORBAND16`, `COLORBANDPEAK`, `COLORALLOSC`, `COLOROSC1`, `COLOROSC2`, `COLOROSC3`, `COLOROSC4`, `COLOROSC5`, `CHANNEL`, `FLIPH`, `FLIPV`, `MODE`, `GAMMAGROUP`, `FALLOFF`, `PEAKFALLOFF`, `BANDWIDTH`, `FPS`, `COLORING`, `PEAKS`, `OSCSTYLE`

### SEQBand

来源：[seqband.cpp:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/seqband.cpp:10>)。

`BAND`, `PARAM`

### SEQVis

来源：[seqvis.cpp:13](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/seqvis.cpp:13>)。

`COLORBOTTOM`, `COLORMIDDLE`, `COLORPREAMP`, `COLORTOP`, `GAMMA`

### SSeeker

来源：[sseeker.cpp:13](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sseeker.cpp:13>)。

`INTERVAL`

### SStatus

来源：[sstatus.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sstatus.cpp:8>)。

`PLAYBITMAP`, `STOPBITMAP`, `PAUSEBITMAP`

### Text

来源：[text.cpp:45](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/text.cpp:45>)。

`ALTSHADOWCOLOR`, `ALTSHADOWX`, `ALTSHADOWY`, `ALTVALIGN`, `CBSOURCE`, `DEFAULT`, `DISPLAY`, `FORCEFIXED`, `FORCELOWERCASE`, `FORCELOCASE`, `FORCEUPCASE`, `FORCEUPPERCASE`, `NOGRAB`, `OFFSETX`, `OFFSETY`, `SHADOWCOLOR`, `SHADOWX`, `SHADOWY`, `SHOWLEN`, `TEXT`, `TICKER`, `TICKERSTEP`, `TIMECOLONWIDTH`, `TIMERHOURS`, `TIMEROFFSTYLE`, `VALIGN`, `WRAP`, `TIMERHOURSROLLOVER`

### TextBase

来源：[textbase.cpp:12](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/textbase.cpp:12>)。

`RCLICKPARAM`, `DBLCLICKPARAM`, `ALIGN`, `ALTANTIALIAS`, `ALTBOLD`, `ALTCOLOR`, `ALTFONT`, `ALTFONTSIZE`, `ALTITALIC`, `ANTIALIAS`, `BOLD`, `COLOR`, `DBLCLICKACTION`, `FONT`, `FONTSIZE`, `ITALIC`, `LEFTPADDING`, `RIGHTCLICKACTION`, `RIGHTPADDING`

### ToggleButton

来源：[tgbutton.cpp:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/tgbutton.cpp:9>)。

`AUTOTOGGLE`, `CFGVAL`

### NStatesTgButton

来源：[tgbutton.cpp:162](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/tgbutton.cpp:162>)。

`NSTATES`, `AUTOELEMENTS`, `CFGVALS`

### Title

来源：[title.cpp:15](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/title.cpp:15>)。

`BORDER`, `DBLCLICKACTION`, `MAXIMIZE`, `STREAKS`, `TITLE`

### Wa2Slider

来源：[xuiwa2slider.cpp:13](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/wa2/xuiwa2slider.cpp:13>)。

`IMAGES`, `IMAGESSPACING`, `SOURCE`

### BookmarkList

来源：[xuibookmarklist.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuibookmarklist.cpp:8>)。

未发现非空属性名；该表可能是旧占位。

### ScriptCheckBox

来源：[xuicheckbox.cpp:10](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuicheckbox.cpp:10>)。

`ACTION`, `ACTION_TARGET`, `PARAM`, `RADIOID`, `RADIOVAL`, `TEXT`

### XuiCustomObject

来源：[xuicustomobject.cpp:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuicustomobject.cpp:9>)。

`GROUPID`

### DownloadsList

来源：[xuidownloadslist.cpp:25](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuidownloadslist.cpp:25>)。

`NOHSCROLL`

### ScriptFrame

来源：[xuiframe.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiframe.cpp:8>)。

`ORIENTATION`, `LEFT`, `TOP`, `RIGHT`, `BOTTOM`, `FROM`, `WIDTH`, `HEIGHT`, `RESIZABLE`, `MAXWIDTH`, `MAXHEIGHT`, `MINWIDTH`, `MINHEIGHT`, `VBITMAP`, `VGRABBER`

### XuiGradientWnd

来源：[xuigradientwnd.cpp:21](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuigradientwnd.cpp:21>)。

`GRADIENT_X1`, `GRADIENT_Y1`, `GRADIENT_X2`, `GRADIENT_Y2`, `POINTS`, `GAMMAGROUP`

### Grid

来源：[xuigrid.cpp:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuigrid.cpp:9>)。

`TOPLEFT`, `TOP`, `TOPRIGHT`, `LEFT`, `MIDDLE`, `RIGHT`, `BOTTOMLEFT`, `BOTTOM`, `BOTTOMRIGHT`

### GroupXFade

来源：[xuigroupxfade.cpp:21](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuigroupxfade.cpp:21>)。

`GROUP`, `GROUPID`, `SPEED`

### HideObject

来源：[xuihideobject.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuihideobject.cpp:7>)。

`HIDE`

### ScriptList

来源：[xuilist.cpp:18](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuilist.cpp:18>)。

`ITEMS`, `MULTISELECT`, `AUTODESELECT`, `SELECT`, `FEED`, `HOVERSELECT`, `SORT`, `SELECTONUPDOWN`, `NUMCOLUMNS`, `COLUMNWIDTHS`, `COLUMNLABELS`

### XuiMenu

来源：[xuimenu.cpp:23](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuimenu.cpp:23>)。

`DOWN`, `HOVER`, `MENU`, `MENUGROUP`, `NEXT`, `NORMAL`, `PREV`

### ScriptObjDirWnd

来源：[xuiobjdirwnd.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiobjdirwnd.cpp:8>)。

`DEFAULTDISPLAY`, `DIR`, `DISPLAYTARGET`, `FORCEVIRTUAL`, `TARGET`

### XuiOSWndHost

来源：[xuioswndhost.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuioswndhost.cpp:11>)。

`HWND`, `OFFSETS`

### ProgressGrid

来源：[xuiprogressgrid.cpp:35](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiprogressgrid.cpp:35>)。

`ORIENTATION`, `INTERVAL`

### ScriptRect

来源：[xuirect.cpp:18](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuirect.cpp:18>)。

`COLOR`, `EDGES`, `FILLED`, `GAMMAGROUP`, `THICKNESS`

### XuiStatus

来源：[xuistatus.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuistatus.cpp:11>)。

`EXCLUDE`, `INCLUDEONLY`

### ScriptTabSheet

来源：[xuitabsheet.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitabsheet.cpp:11>)。

`CHILDREN`, `CONTENT_MARGIN_BOTTOM`, `CONTENT_MARGIN_LEFT`, `CONTENT_MARGIN_RIGHT`, `CONTENT_MARGIN_TOP`, `TYPE`, `WINDOWTYPE`

### ColorThemesList

来源：[xuithemeslist.cpp:46](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuithemeslist.cpp:46>)。

`NOHSCROLL`

### ScriptTitleBox

来源：[xuititlebox.cpp:8](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuititlebox.cpp:8>)。

`CENTERED`, `CONTENT`, `SUFFIX`, `TITLE`

### ScriptTree

来源：[xuitree.cpp:143](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitree.cpp:143>)。

`CHILDTABS`, `EXPANDROOT`, `FEED`, `ITEMS`, `SORTED`

### XuiWindowHolder

来源：[xuiwndholder.cpp:18](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiwndholder.cpp:18>)。

`PARAM`, `COMPONENT`, `HOLD`, `NOSHOWCMDBAR`, `NOANIMATEDRECTS`, `DISABLEANIMATEDRECTS`, `AUTOOPEN`, `AUTOCLOSE`, `AUTOFOCUS`, `AUTOAVAILABLE`

### GuiObjectWnd

来源：[guiobjwnd.cpp:11](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wnd/wndclass/guiobjwnd.cpp:11>)。

`ACTIVEALPHA`, `ALPHA`, `ANCHOR`, `APPBAR`, `CFGATTRIB`, `CURSOR`, `DROPTARGET`, `ENABLED`, `FITPARENT`, `FOCUSONCLICK`, `GHOST`, `H`, `ID`, `INACTIVEALPHA`, `MOVE`, `NOCONTEXTMENU`, `NODBLCLICK`, `NOLEFTCLICK`, `NOMOUSEMOVE`, `NORIGHTCLICK`, `NOTIFY`, `NOTIFY0`, `NOTIFY1`, `NOTIFY2`, `NOTIFY3`, `NOTIFY4`, `NOTIFY5`, `NOTIFY6`, `NOTIFY7`, `NOTIFY8`, `NOTIFY9`, `RECTRGN`, `REGIONOP`, `RELATH`, `RELATW`, `RELATX`, `RELATY`, `RENDERBASETEXTURE`, `SYSMETRICSX`, `SYSMETRICSY`, `SYSMETRICSW`, `SYSMETRICSH`, `SYSREGION`, `TABORDER`, `TOOLTIP`, `TRANSLATE`, `USERDATA`, `VISIBLE`, `W`, `WANTFOCUS`, `X`, `X1`, `X2`, `Y`, `Y1`, `Y2`

### Layout

来源：[layout.cpp:66](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wndmgr/layout.cpp:66>)。

`ALPHA`, `ALPHABACKGROUND`, `DESKTOPALPHA`, `FORCEALPHA`, `INDESKTOP`, `LINKHEIGHT`, `LINKWIDTH`, `LOCKTO`, `NOACTIVATION`, `NODOCK`, `NOOFFSCREENCHECK`, `NOPARENT`, `ONTOP`, `OSFRAME`, `OWNER`, `SNAPADJUSTBOTTOM`, `SNAPADJUSTLEFT`, `SNAPADJUSTRIGHT`, `SNAPADJUSTTOP`, `UNLINKED`, `RESIZABLE`, `SCALABLE`

### 非 XMLParamPair 路径补充

| 对象／结构 | 属性 | 源码 |
| --- | --- | --- | --- |
| Container | `name, id, dynamic, default_x, default_y, default_visible, canclose, nomenu, nofocusapponclose, primarycomponent` | [container.cpp:24](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wndmgr/container.cpp:24>) |
| SnapPoint | `id, x, y, relatx, relaty` | [snappnt.cpp:24](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wndmgr/snappnt.cpp:24>) |
| QueryLine | `querylist, query, auto` | [xuiqueryline.cpp:13](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiqueryline.cpp:13>) |
| SvcWnd | `DBLCLICKACTION, GUID`（旧接口） | [svcwnd.cpp:70](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/script/objects/svcwnd.cpp:70>) |
| Shadow | `TARGET`（旧接口） | [shadowwnd.cpp:9](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/imgldr/imggen/shadowwnd.cpp:9>) |
| Slider 工厂 | `action, param` 决定派生控件 | [pslider.cpp:315](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/pslider.cpp:315>) |
| SendParams / AddParams | `group, target` 加任意待转发属性；AddParams 拼接字符串 | [objectactuator.cpp:7](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/objectactuator.cpp:7>) |
| GroupDef 解析 | `id, xuitag, inherit_group, inherit_content, inherit_params`，另有 Group 属性 | [skinparse.cpp:1140](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/skinparse.cpp:1140>) |

没有单独参数表的包装对象，应查看其基类及 XML 模板。例如 GroupList 主要靠脚本创建项目，PathPicker/RadioGroup 依赖组合内容；不能把“无自有表”解释为无功能或无继承属性。

## 4. 控件相关 MAKI 方法／事件声明

格式为 `名称(参数数量)`。这里逐项记录原生导出表，仍需继承父类接口；没有列出的 System、Timer、Map、Region、Config 等属于另外的脚本对象体系。含 on 前缀的函数通常是事件，具体调度必须读其实现。`fake`、`callme` 等名称保留源码原貌，不视为成熟可用接口。旧 Browser 与现代 Browser 两套源码都保留，不能合并成一个未经验证的运行时签名表。

### AlbumArtScriptController — AlbumArt.cpp

来源：[AlbumArt.cpp:30](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/AlbumArt.cpp:30>)。

- `refresh(0)`, `onAlbumArtLoaded(1)`, `isLoading(0)`

### PlDirScriptObjectController — wa2pldirobj.cpp

来源：[wa2pldirobj.cpp:212](<D:/Projects/Backup/TTPlayer/winamp/Src/Plugins/General/gen_ff/wa2pldirobj.cpp:212>)。

- `showCurrentlyPlayingEntry(0)`, `getNumItems(0)`, `renameItem(2)`, `getItemName(1)`, `playItem(1)`, `enqueueItem(1)`, `refresh(0)`

### GuiObjectScriptController — guiobj.cpp

来源：[guiobj.cpp:1578](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/script/objects/guiobj.cpp:1578>)。

- `getId(0)`, `show(0)`, `hide(0)`, `onSetVisible(1)`, `isVisible(0)`, `setAlpha(1)`, `getAlpha(0)`, `setActiveAlpha(1)`
- `getActiveAlpha(0)`, `setInactiveAlpha(1)`, `getInactiveAlpha(0)`, `onLeftButtonDown(2)`, `onLeftButtonUp(2)`, `onRightButtonDown(2)`, `onRightButtonUp(2)`, `onRightButtonDblClk(2)`
- `onLeftButtonDblClk(2)`, `onMouseWheelUp(2)`, `onMouseWheelDown(2)`, `onMouseMove(2)`, `onEnterArea(0)`, `onLeaveArea(0)`, `isMouseOverRect(0)`, `onStartup(0)`
- `onChar(1)`, `onKeyDown(1)`, `onKeyUp(1)`, `setEnabled(1)`, `getEnabled(0)`, `onEnable(1)`, `resize(4)`, `onResize(4)`
- `isMouseOver(2)`, `getLeft(0)`, `getTop(0)`, `getWidth(0)`, `getHeight(0)`, `getGuiX(0)`, `getGuiY(0)`, `getGuiW(0)`
- `getGuiH(0)`, `getGuiRelatX(0)`, `getGuiRelatY(0)`, `getGuiRelatW(0)`, `getGuiRelatH(0)`, `clientToScreenX(1)`, `clientToScreenY(1)`, `clientToScreenW(1)`
- `clientToScreenH(1)`, `screenToClientX(1)`, `screenToClientY(1)`, `screenToClientW(1)`, `screenToClientH(1)`, `setTargetX(1)`, `setTargetY(1)`, `setTargetW(1)`
- `setTargetH(1)`, `setTargetA(1)`, `setTargetSpeed(1)`, `gotoTarget(0)`, `onTargetReached(0)`, `cancelTarget(0)`, `reverseTarget(1)`, `isGoingToTarget(0)`
- `setXmlParam(2)`, `getXmlParam(1)`, `init(1)`, `bringToFront(0)`, `bringToBack(0)`, `bringAbove(1)`, `bringBelow(1)`, `isActive(0)`
- `getParent(0)`, `getTopParent(0)`, `getInterface(1)`, `onAction(7)`, `getParentLayout(0)`, `runModal(0)`, `endModal(1)`, `popParentLayout(0)`
- `setStatusText(2)`, `findObject(1)`, `findObjectXY(2)`, `getName(0)`, `getAutoWidth(0)`, `getAutoHeight(0)`, `setFocus(0)`, `onGetFocus(0)`
- `onKillFocus(0)`, `sendAction(6)`, `onAccelerator(1)`, `cfg_getInt(0)`, `cfg_setInt(1)`, `cfg_getFloat(0)`, `cfg_setFloat(1)`, `cfg_getString(0)`
- `cfg_setString(1)`, `cfg_onDataChanged(0)`, `cfg_getItemGuid(0)`, `cfg_getAttributeName(0)`, `onDragEnter(0)`, `onDragOver(2)`, `onDragLeave(0)`

### SvcWndScriptController — svcwnd.cpp

来源：[svcwnd.cpp:26](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/script/objects/svcwnd.cpp:26>)。

- `getGUID(1)`, `getWac(0)`

### AnimLayerScriptController — animlayer.cpp

来源：[animlayer.cpp:14](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/animlayer.cpp:14>)。

- `setSpeed(1)`, `gotoFrame(1)`, `setStartFrame(1)`, `setEndFrame(1)`, `setAutoReplay(1)`, `play(0)`, `togglePause(0)`, `stop(0)`
- `pause(0)`, `isPlaying(0)`, `isPaused(0)`, `isStopped(0)`, `getStartFrame(0)`, `getEndFrame(0)`, `getLength(0)`, `getDirection(0)`
- `getAutoReplay(0)`, `getCurFrame(0)`, `onPlay(0)`, `onPause(0)`, `onResume(0)`, `onStop(0)`, `onFrame(1)`, `setRealtime(1)`

### ButtonScriptController — button.cpp

来源：[button.cpp:26](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/button.cpp:26>)。

- `onActivate(1)`, `setActivated(1)`, `getActivated(0)`, `onLeftClick(0)`, `onRightClick(0)`, `leftClick(0)`, `rightClick(0)`, `setActivatedNoCallback(1)`

### CompBucketScriptController — compbuck2.cpp

来源：[compbuck2.cpp:513](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/compbuck2.cpp:513>)。

- `getMaxWidth(0)`, `getMaxHeight(0)`, `getScroll(0)`, `setScroll(1)`, `getNumChildren(0)`, `enumChildren(1)`, `fake(0)`

### QueryListScriptController — xuiquerylist.cpp

来源：[xuiquerylist.cpp:82](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/db/xuiquerylist.cpp:82>)。

- `onResetQuery(0)`

### DropDownListScriptController — dropdownlist.cpp

来源：[dropdownlist.cpp:539](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/dropdownlist.cpp:539>)。

- `getItemSelected(0)`, `onSelect(2)`, `setListHeight(1)`, `openList(0)`, `closeList(0)`, `setItems(1)`, `addItem(1)`, `delItem(1)`
- `findItem(1)`, `getNumItems(0)`, `selectItem(2)`, `getItemText(1)`, `getSelected(0)`, `getSelectedText(0)`, `getCustomText(0)`, `deleteAllItems(0)`
- `setNoItemText(1)`

### EditScriptController — edit.cpp

来源：[edit.cpp:19](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/edit.cpp:19>)。

- `setText(1)`, `setAutoEnter(1)`, `getAutoEnter(0)`, `getText(0)`, `onEnter(0)`, `onAbort(0)`, `onIdleEditUpdate(0)`, `onEditUpdate(0)`
- `selectAll(0)`, `enter(0)`, `setIdleEnabled(1)`, `getIdleEnabled(0)`

### GroupScriptController — group.cpp

来源：[group.cpp:1291](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/group.cpp:1291>)。

- `getObject(1)`, `enumObject(1)`, `getNumObjects(0)`, `onCreateObject(1)`, `getMousePosX(0)`, `getMousePosY(0)`, `isLayout(0)`, `autoResize(0)`

### CfgGroupScriptController — group.cpp

来源：[group.cpp:1701](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/group.cpp:1701>)。

- `cfgGetInt(0)`, `cfgSetInt(1)`, `cfgGetFloat(0)`, `cfgSetFloat(1)`, `cfgGetString(0)`, `cfgSetString(1)`, `onCfgChanged(0)`, `cfgGetName(0)`
- `cfgGetGuid(0)`

### GroupListScriptController — grouplist.cpp

来源：[grouplist.cpp:136](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/grouplist.cpp:136>)。

- `instantiate(2)`, `getNumItems(0)`, `enumItem(1)`, `removeAll(0)`, `scrollToPercent(1)`, `setRedraw(1)`

### LayerScriptController — layer.cpp

来源：[layer.cpp:1319](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/layer.cpp:1319>)。

- `setRegionFromMap(3)`, `setRegion(1)`, `isInvalid(0)`, `onBeginResize(4)`, `onEndResize(4)`, `fx_setEnabled(1)`, `fx_getEnabled(0)`, `fx_onInit(0)`
- `fx_onFrame(0)`, `fx_onGetPixelR(4)`, `fx_onGetPixelD(4)`, `fx_onGetPixelX(4)`, `fx_onGetPixelY(4)`, `fx_onGetPixelA(4)`, `fx_setWrap(1)`, `fx_getWrap(0)`
- `fx_setRect(1)`, `fx_getRect(0)`, `fx_setBgFx(1)`, `fx_getBgFx(0)`, `fx_setClear(1)`, `fx_getClear(0)`, `fx_setSpeed(1)`, `fx_getSpeed(0)`
- `fx_setRealtime(1)`, `fx_getRealtime(0)`, `fx_setLocalized(1)`, `fx_getLocalized(0)`, `fx_setBilinear(1)`, `fx_getBilinear(0)`, `fx_setAlphaMode(1)`, `fx_getAlphaMode(0)`
- `fx_setGridSize(2)`, `fx_update(0)`, `fx_restart(0)`

### BrowserScriptController — scriptbrowser.cpp

来源：[scriptbrowser.cpp:17](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mb/scriptbrowser.cpp:17>)。

- `gotoUrl(1)`, `back(0)`, `forward(0)`, `home(0)`, `refresh(0)`, `setTargetName(1)`, `onBeforeNavigate(3)`, `onDocumentComplete(1)`

### BrowserScriptController — xuibrowser.cpp

来源：[xuibrowser.cpp:188](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mb/xuibrowser.cpp:188>)。

- `gotoUrl(1)`, `navigateUrl(1)`, `back(0)`, `forward(0)`, `home(0)`, `stop(0)`, `refresh(0)`, `scrape(0)`
- `setTargetName(1)`, `onBeforeNavigate(3)`, `onDocumentComplete(1)`, `onDocumentReady(1)`, `onNavigateError(2)`, `onMediaLink(1)`, `getDocumentTitle(0)`, `setCancelIEErrorPage(1)`
- `messageToMaki(5)`, `messageToJS(5)`

### MouseRedirScriptController — mouseredir.cpp

来源：[mouseredir.cpp:103](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/mouseredir.cpp:103>)。

- `setRegionFromMap(1)`, `setRegion(1)`, `setRedirection(1)`, `getRedirection(0)`

### SliderScriptController — pslider.cpp

来源：[pslider.cpp:189](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/pslider.cpp:189>)。

- `setPosition(1)`, `getPosition(0)`, `onSetPosition(1)`, `onPostedPosition(1)`, `onSetFinalPosition(1)`, `lock(0)`, `unlock(0)`

### VisScriptController — sa.cpp

来源：[sa.cpp:643](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sa.cpp:643>)。

- `onFrame(0)`, `setRealtime(1)`, `getRealtime(0)`, `setMode(1)`, `getMode(0)`, `nextMode(0)`

### EqVisScriptController — seqvis.cpp

来源：[seqvis.cpp:306](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/seqvis.cpp:306>)。

- `fake(0)`

### StatusScriptController — sstatus.cpp

来源：[sstatus.cpp:123](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/sstatus.cpp:123>)。

- `fake(0)`

### TextScriptController — text.cpp

来源：[text.cpp:1655](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/text.cpp:1655>)。

- `setText(1)`, `setAlternateText(1)`, `getText(0)`, `getTextWidth(0)`, `onTextChanged(1)`

### TgButtonScriptController — tgbutton.cpp

来源：[tgbutton.cpp:96](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/tgbutton.cpp:96>)。

- `onToggle(1)`, `getCurCfgVal(0)`

### TitleScriptController — title.cpp

来源：[title.cpp:199](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/title.cpp:199>)。

- `fake(0)`

### CheckBoxController — xuicheckbox.cpp

来源：[xuicheckbox.cpp:90](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuicheckbox.cpp:90>)。

- `onToggle(1)`, `setChecked(1)`, `isChecked(0)`, `setText(1)`, `getText(0)`

### FrameScriptController — xuiframe.cpp

来源：[xuiframe.cpp:160](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiframe.cpp:160>)。

- `setPosition(1)`, `getPosition(0)`

### GuiListScriptController — xuilist.cpp

来源：[xuilist.cpp:534](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuilist.cpp:534>)。

- `getNumItems(0)`, `getWantAutoDeselect(0)`, `setWantAutoDeselect(1)`, `onSetVisible(1)`, `setAutoSort(1)`, `next(0)`, `selectCurrent(0)`, `selectFirstEntry(0)`
- `previous(0)`, `pagedown(0)`, `pageup(0)`, `home(0)`, `end(0)`, `reset(0)`, `addColumn(3)`, `getNumColumns(0)`
- `getColumnWidth(1)`, `setColumnWidth(2)`, `getColumnLabel(1)`, `setColumnLabel(2)`, `getColumnNumeric(1)`, `setColumnDynamic(2)`, `isColumnDynamic(1)`, `setMinimumSize(1)`
- `addItem(1)`, `insertItem(2)`, `getLastAddedItemPos(0)`, `setSubItem(3)`, `deleteAllItems(0)`, `deleteByPos(1)`, `getItemLabel(2)`, `setItemLabel(2)`
- `setItemIcon(2)`, `getItemIcon(1)`, `setShowIcons(1)`, `getShowIcons(0)`, `setIconWidth(1)`, `getIconWidth(0)`, `setIconHeight(1)`, `getIconHeight(0)`
- `onIconLeftclick(3)`, `getItemSelected(1)`, `isItemFocused(1)`, `getItemFocused(0)`, `setItemFocused(1)`, `ensureItemVisible(1)`, `invalidateColumns(0)`, `scrollAbsolute(1)`
- `scrollRelative(1)`, `scrollLeft(1)`, `scrollRight(1)`, `scrollUp(1)`, `scrollDown(1)`, `getSubitemText(2)`, `getFirstItemSelected(0)`, `getNextItemSelected(1)`
- `selectAll(0)`, `deselectAll(0)`, `invertSelection(0)`, `invalidateItem(1)`, `getFirstItemVisible(0)`, `getLastItemVisible(0)`, `setFontSize(1)`, `getFontSize(0)`
- `jumpToNext(1)`, `scrollToItem(1)`, `resort(0)`, `getSortDirection(0)`, `getSortColumn(0)`, `setSortColumn(1)`, `setSortDirection(1)`, `getItemCount(0)`
- `setSelectionStart(1)`, `setSelectionEnd(1)`, `setSelected(2)`, `toggleSelection(2)`, `getHeaderHeight(0)`, `getPreventMultipleSelection(0)`, `setPreventMultipleSelection(1)`, `moveItem(2)`
- `onSelectAll(0)`, `onDelete(0)`, `onDoubleClick(1)`, `onLeftClick(1)`, `onSecondLeftClick(1)`, `onRightClick(1)`, `onColumnDblClick(3)`, `onColumnLabelClick(3)`
- `onItemSelection(2)`

### XuiMenuScriptController — xuimenuso.cpp

来源：[xuimenuso.cpp:17](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuimenuso.cpp:17>)。

- `setMenuGroup(1)`, `getMenuGroup(0)`, `setMenu(1)`, `getMenu(0)`, `spawnMenu(1)`, `cancelMenu(0)`, `setNormalId(1)`, `setDownId(1)`
- `setHoverId(1)`, `onOpenMenu(0)`, `onCloseMenu(0)`, `nextMenu(0)`, `previousMenu(0)`

### PathPickerScriptController — xuipathpicker.cpp

来源：[xuipathpicker.cpp:26](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuipathpicker.cpp:26>)。

- `getPath(0)`, `onPathChanged(1)`

### LayoutStatusController — xuistatus.cpp

来源：[xuistatus.cpp:60](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuistatus.cpp:60>)。

- `callme(1)`

### ScriptTabSheetController — xuitabsheet.cpp

来源：[xuitabsheet.cpp:148](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitabsheet.cpp:148>)。

- `getCurPage(0)`, `setCurPage(1)`

### GuiTreeScriptController — xuitree.cpp

来源：[xuitree.cpp:810](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitree.cpp:810>)。

- `getNumRootItems(0)`, `enumRootItem(1)`, `onLeftButtonDown(2)`, `onLeftButtonUp(2)`, `onRightButtonUp(2)`, `onMouseMove(2)`, `onWantAutoContextMenu(0)`, `onLeftButtonDblClk(2)`
- `onRightButtonDblClk(2)`, `onMouseWheelUp(2)`, `onMouseWheelDown(2)`, `onContextMenu(2)`, `onChar(1)`, `onKeyDown(1)`, `onItemRecvDrop(1)`, `onLabelChange(1)`
- `onItemSelected(1)`, `onItemDeselected(1)`, `onKillFocus(0)`, `jumpToNext(1)`, `ensureItemVisible(1)`, `getContentsWidth(0)`, `getContentsHeight(0)`, `addTreeItem(4)`
- `removeTreeItem(1)`, `moveTreeItem(2)`, `deleteAllItems(0)`, `expandItem(1)`, `expandItemDeferred(1)`, `collapseItem(1)`, `collapseItemDeferred(1)`, `selectItem(1)`
- `selectItemDeferred(1)`, `delItemDeferred(1)`, `hiliteItem(1)`, `unhiliteItem(1)`, `getCurItem(0)`, `hitTest(2)`, `editItemLabel(1)`, `cancelEditLabel(1)`
- `setAutoEdit(1)`, `getAutoEdit(0)`, `getByLabel(2)`, `setSorted(1)`, `getSorted(0)`, `sortTreeItems(0)`, `getSibling(1)`, `setAutoCollapse(1)`
- `setFontSize(1)`, `getFontSize(0)`, `getNumVisibleChildItems(1)`, `getNumVisibleItems(0)`, `enumVisibleItems(1)`, `enumVisibleChildItems(2)`, `enumAllItems(1)`, `getItemRectX(1)`
- `getItemRectY(1)`, `getItemRectW(1)`, `getItemRectH(1)`

### TreeItemScriptController — xuitree.cpp

来源：[xuitree.cpp:2099](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuitree.cpp:2099>)。

- `getNumChildren(0)`, `setLabel(1)`, `getLabel(0)`, `ensureVisible(0)`, `getNthChild(1)`, `getChild(0)`, `getChildSibling(1)`, `getSibling(0)`
- `getParent(0)`, `editLabel(0)`, `hasSubItems(0)`, `setSorted(1)`, `setChildTab(1)`, `isSorted(0)`, `isCollapsed(0)`, `isExpanded(0)`
- `invalidate(0)`, `isSelected(0)`, `isHilited(0)`, `setHilited(1)`, `collapse(0)`, `expand(0)`, `getTree(0)`, `onTreeAdd(0)`
- `onTreeRemove(0)`, `onSelect(0)`, `onDeselect(0)`, `onLeftDoubleClick(0)`, `onRightDoubleClick(0)`, `onChar(1)`, `onExpand(0)`, `onCollapse(0)`
- `onBeginLabelEdit(0)`, `onEndLabelEdit(1)`, `onContextMenu(2)`

### WindowHolderScriptController — xuiwndholder.cpp

来源：[xuiwndholder.cpp:181](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/skin/widgets/xuiwndholder.cpp:181>)。

- `getGUID(1)`, `setRegionFromMap(3)`, `setRegion(1)`, `getContent(0)`, `getComponentName(0)`

### EmbeddedXuiScriptController — embeddedxui.cpp

来源：[embeddedxui.cpp:83](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wnd/wndclass/embeddedxui.cpp:83>)。

- `getEmbeddedObject(0)`

### ContainerScriptController — container.cpp

来源：[container.cpp:653](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wndmgr/container.cpp:653>)。

- `onSwitchToLayout(1)`, `onBeforeSwitchToLayout(2)`, `onHideLayout(1)`, `onShowLayout(1)`, `getLayout(1)`, `getNumLayouts(0)`, `enumLayout(1)`, `getCurLayout(0)`
- `switchToLayout(1)`, `isDynamic(0)`, `show(0)`, `hide(0)`, `close(0)`, `toggle(0)`, `setName(1)`, `getName(0)`
- `getGuid(0)`, `setXmlParam(2)`, `onAddContent(3)`

### LayoutScriptController — layout.cpp

来源：[layout.cpp:2267](<D:/Projects/Backup/TTPlayer/winamp/Src/Wasabi/api/wndmgr/layout.cpp:2267>)。

- `onDock(1)`, `onUndock(0)`, `getScale(0)`, `setScale(1)`, `onScale(1)`, `setDesktopAlpha(1)`, `getDesktopAlpha(0)`, `isTransparencySafe(0)`
- `isLayoutAnimationSafe(0)`, `getContainer(0)`, `center(0)`, `onMove(0)`, `onEndMove(0)`, `snapAdjust(4)`, `getSnapAdjustTop(0)`, `getSnapAdjustLeft(0)`
- `getSnapAdjustRight(0)`, `getSnapAdjustBottom(0)`, `onUserResize(4)`, `setRedrawOnResize(1)`, `beforeRedock(0)`, `redock(0)`, `onMouseEnterLayout(0)`, `onMouseLeaveLayout(0)`
- `onSnapAdjustChanged(0)`

## 5. 阅读与实现时的注意点

- 原生工厂、XML 属性、脚本 GUID 接口、事件来源和实际绘制是五个不同层面。只实现其中一个，不能标记整个控件已完成。
- Tag 大小写不敏感不意味着任意前缀都可去掉；Wasabi:Button 等可能是 Group 模板。
- 系统 XML 和皮肤自身的同名定义存在优先级与继承关系，解析时应保留来源和定义链。
- 对第三方服务，完整源码索引也不能代替真实服务能力；独立实现需要明确的千千替代接口或“不支持”诊断。
