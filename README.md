# Behavior Data Injector — Universal SE/AE (`0.1.7`)

[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-737373?style=for-the-badge)](https://learn.microsoft.com/en-us/windows/)
[![Skyrim: 1.7.104.0](https://img.shields.io/badge/Skyrim-1.7.104.0-007ec6?style=for-the-badge)](#target-runtimes--compatibility-status)
[![Edition: Universal SE / AE](https://img.shields.io/badge/Edition-Universal%20SE%20%2F%20AE-00599C?style=for-the-badge)](#target-runtimes--compatibility-status)
[![SKSE: 2.3.1](https://img.shields.io/badge/SKSE-2.3.1-e87039?style=for-the-badge)](https://skse.silverlock.org/)
[![License: GPL-3.0](https://img.shields.io/badge/License-GPL--3.0-78b900?style=for-the-badge)](LICENSE)

Universal Skyrim SE/AE C++23 SKSE64 port of [max-su-2019/BehaviorDataInjector](https://github.com/max-su-2019/BehaviorDataInjector), supporting Skyrim 1.7.104.0 and older SE/AE runtimes in a single DLL (no `.esp`/`.esl` required).

## Target Runtimes & Compatibility Status

* **Mandatory Primary Target:** **Skyrim AE 1.7.104.0** (SKSE64 2.3.1, Address Library Format 5)
* **Universal Flat-Rim Baseline:** Single SE + AE DLL (`ENABLE_SKYRIM_SE=1`, `ENABLE_SKYRIM_AE=1`, `ENABLE_SKYRIM_VR=0`; declares `SKSE::StructCompatibility::Independent`, `NoStructUse`, and `AddressLibraryV5`)
* **Hook & Relocation Binding:** `REL::RelocationID(62640, 63585)` + `REL::Relocate(0x22A, 0x237)` (`0xE8` CALL instruction guard); on 1.7.104.0 additionally logs direct vs chained hook target against `REL::ID(63845)`.
* **Skyrim VR:** Explicitly rejected cleanly at `SKSEPlugin_Load`.

| Skyrim Runtime | Address Library | RVA Evidence (`Base` / `Hook` / `ID 63845`) | Status |
| :--- | :--- | :--- | :--- |
| **1.7.104.0** (Primary AE) | Format 5 | `0xBCD240` / `0xBCD477` / `0xBDE7B0` | Statically verified (`SkyrimSE.exe` SHA-256 `846efccf...` + AL v5); **User confirmed** |
| **1.6.1170.0** (Steam AE) | Format 2 | `0xBB0800` / `0xBB0A37` / `0xBC1D70` | Statically verified (Address Library); **Requires in-game test** |
| **1.6.1179.0** (GOG AE) | Format 2 | `0xBB22C0` / `0xBB24F7` / `0xBC3830` | Statically verified (Address Library); **Requires in-game test** |
| **1.5.97.0** (SE) | Format 1 | `0xAEDD10` / `0xAEDF3A` / `0xB4EA50` | Statically verified (Address Library); **Requires in-game test** |
| **Skyrim VR** | N/A | N/A | **Not supported** (rejected cleanly on load) |

## End-User Installation & Requirements

* **Required:** Matching **SKSE64**, **Address Library for SKSE Plugins**, and Microsoft Visual C++ Redistributable x64.
* **Installation:** Install via **Vortex** / **MO2** or extract `SKSE/Plugins/BehaviorDataInjector.dll` and `SKSE/Plugins/BehaviorDataInjector.ini` under `Data/`.
* **Configuration:** See [doc/How to create BDI config files.md](doc/How%20to%20create%20BDI%20config%20files.md) for `_BDI.json` project schemas and `DGV` console helper usage.
* **Note on Debug Symbols:** The confirmed release archive omits the original local build `.pdb` because that developer symbol artifact retained local build-tree paths; no private path exists in the distributed `BehaviorDataInjector.dll` or release archive.

## Build Requirements & Reproducible Build

* Windows x64, MSVC C++23 toolset (Visual Studio 2022/2026), Git, CMake `>= 3.26`
* **CommonLibSSE-NG v12.1.0** (`https://github.com/alandtse/CommonLibSSE-NG`, commit `8ffc2b74af015f837de53cc02b0cb470a15678e4`)
* **DKUtil** (`https://github.com/gottyduke/DKUtil`, commit `0ddab238cc8c4915b9e8fda90668d74e513d534a`)
* Dependencies via `vcpkg.json` (baseline `ee12231b20c95013c6638d845d04c91559a1d1ff`, triplet `x64-windows-static-md`) or `-DCMAKE_PREFIX_PATH` (`spdlog`, `fmt`, `rapidcsv`, `nlohmann-json`, `directxmath`, `directxtk`, `simpleini`, `toml11`, `xbyak`)

Run `Build-Release.cmd` (or configure CMake out-of-tree):

```powershell
cmake -S . -B build/release -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCommonLibPath="path/to/CommonLibSSE-NG-v12.1.0" `
  -DDKUtilPath="path/to/DKUtil" `
  -DCMAKE_PREFIX_PATH="path/to/deps"

cmake --build build/release --target BehaviorDataInjector --parallel
```

## Attribution & License

* Original **Behavior Data Injector** work & reverse engineering: **max-su-2019** and contributors ([BehaviorDataInjector](https://github.com/max-su-2019/BehaviorDataInjector)).
* Universal SE/AE compatibility port: DeadOnKeyboard1.
* Licensed under **GNU GPL v3.0 or later (`GPL-3.0-or-later`)** (see [LICENSE](LICENSE)) together with CommonLibSSE-NG's **Modding Exception** and **GPL-3.0 Linking Exception** (see [EXCEPTIONS.md](EXCEPTIONS.md)) and preserved MIT notices in [NOTICE](NOTICE).
