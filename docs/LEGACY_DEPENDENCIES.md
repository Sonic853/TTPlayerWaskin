# Legacy DLL dependencies

The shared XP-compatible `ttp_waskin.dll`, used by both player editions, includes unmodified portions of:

- YY-Thunks 1.2.2, MIT, Copyright (c) 2018 Chuyu-Team.
  [Corresponding source](https://github.com/Chuyu-Team/YY-Thunks/tree/v1.2.2).
  See [YY-Thunks-LICENSE.txt](licenses/YY-Thunks-LICENSE.txt).
- VC-LTL 5.3.1, Eclipse Public License 2.0.
  [Corresponding source](https://github.com/Chuyu-Team/VC-LTL5/tree/v5.3.1),
  [source archive](https://github.com/Chuyu-Team/VC-LTL5/archive/refs/tags/v5.3.1.zip).
  See [VC-LTL-LICENSE.txt](licenses/VC-LTL-LICENSE.txt).

VC-LTL also identifies Microsoft components under their respective licenses:
[Visual Studio](https://visualstudio.microsoft.com/license-terms/) and
[Windows SDK CRT source](https://www.nuget.org/packages/Microsoft.Windows.SDK.CRTSource/10.0.22621.3/License).

The independently implemented `ttp_waskin` sources remain under the accompanying MIT `LICENSE`.

The minimal Actions/Release ZIP contains only `AddIn/ttp_waskin.dll` and
`SHA256SUMS.txt`. License texts remain in this repository; they are not embedded
in the DLL. CMake installation additionally copies the text files alongside
this notice to `licenses/ttp_waskin`.
