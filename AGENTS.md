# Repository Guidelines

## Project Structure & Module Organization
This repository is an Unreal Engine 5.4 game project (`VRTest.uproject`). Core gameplay C++ lives in `Source/VRTest` with public headers in `Public/` and implementations in `Private/`.

- Main module: `Source/VRTest`
- Editor/Game targets: `Source/VRTestEditor.Target.cs`, `Source/VRTest.Target.cs`
- Plugin code: `Plugins/BP2AI/Source/BP2AI`
- Assets and settings: `Content/`, `Config/`
- Generated/local artifacts (do not review for logic): `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`

## Build, Test, and Development Commands
Use Windows + UE5 command-line tools.

- Build editor target:
  `"<UE5_ROOT>\Engine\Build\BatchFiles\Build.bat" VRTestEditor Win64 Development "D:\Perforce\ShujiWorkspace\VRTest.uproject"`
- Launch editor:
  `"<UE5_ROOT>\Engine\Binaries\Win64\UnrealEditor.exe" "D:\Perforce\ShujiWorkspace\VRTest.uproject"`
- Run automated tests (headless):
  `"<UE5_ROOT>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Perforce\ShujiWorkspace\VRTest.uproject" -ExecCmds="Automation RunTests BP2AI; Quit" -unattended -nop4`

## Coding Style & Naming Conventions
- Follow Unreal C++ conventions: `A/U/F/I/E` prefixes, `PascalCase` types/functions, `b` prefix for booleans.
- Keep headers in `Public/` only when cross-module or Blueprint exposure is needed.
- Use tabs/UE default formatting in existing files; do not reformat unrelated code.
- Feature folders are preferred (for example `Game/`, `AI/`, `Skill/`, `Goap/`).

## Testing Guidelines
- Existing test-oriented code is concentrated in `Plugins/BP2AI/.../Test/` and files with `*Test*.cpp`.
- Guard test helpers for non-shipping builds (`#if !UE_BUILD_SHIPPING`).
- Name new tests by feature + `Test` suffix (example: `InventoryExportTest.cpp`).
- Validate both editor behavior and command-line execution when changing plugin export logic.

## Commit & Pull Request Guidelines
- Current history is mixed; prefer: `<type>: <short summary>` (for example `fix: prevent null AliveComponent crash`).
- One logical change per commit; include affected module (`VRTest`, `BP2AI`) in message body when useful.
- PRs should include: purpose, key file paths, test evidence (logs/screenshots), and linked task/issue.

## Refactor Notes (Current)
During ongoing refactor, ignore these legacy headers:
`MyActortest.h`, `MyCharacter_tessst.h`, `MyClassToGenerateSLN.h`, `MyFunctionLibrary.h`, `EnemyPatrolSplineComponent.h`, `BlueprintableSphereComponent.h`, `AISense_Player.h`, `AISenseConfig_Player.h`, `CLM_Character.h`, `Enemy_Base.h`, `MyActor_test.h`.
