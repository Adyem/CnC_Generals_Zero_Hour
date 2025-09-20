# WWLib Win32 Touch Points

This document catalogs every use of the `win.h` façade inside WWLib so the upcoming SFML/portable replacements can be scoped accurately.

## Global façade (`win.h` / `win.cpp`)
* `win.cpp` owns the globals exported by `win.h`: `ProgramInstance`, `MainWindow`, and the focus flag `GameInFocus`, with a debug-only Win32 error printer that depends on `FormatMessage`/`LocalFree`.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/win.cpp†L19-L49】【F:Generals/Code/Libraries/Source/WWVegas/WWLib/win.h†L62-L95】
* The GeneralsMD tree mirrors this definition verbatim, so both game targets share the same Win32 façade today.【F:GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/win.cpp†L19-L49】

## Message pump and accelerator registry (`msgloop.*`)
* `msgloop.cpp` keeps global vectors of modeless dialog `HWND`s and accelerators (`HWND` + `HACCEL` pairs) and exposes `Add_Modeless_Dialog`, `Remove_Modeless_Dialog`, `Add_Accelerator`, and `Remove_Accelerator` around those Win32 handles.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/msgloop.cpp†L42-L199】【F:Generals/Code/Libraries/Source/WWVegas/WWLib/msgloop.cpp†L220-L252】
* `Windows_Message_Handler` itself drives the Win32 message pump using `PeekMessage`, `GetMessage`, `TranslateMessage`, `DispatchMessage`, the accelerator table, and any registered dialog/handler intercepts.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/msgloop.cpp†L97-L152】
* `msgloop.h` re-exports these entry points (and the `Message_Intercept_Handler` function pointer) to the rest of WWLib behind direct `<windows.h>` inclusions.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/msgloop.h†L40-L58】

## Keyboard system
* `WWKeyboardClass::Fill_Buffer_From_System` calls `Windows_Message_Handler` whenever its ring buffer has space, so the keyboard queue depends on the Win32 message pump to stay current.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/keyboard.cpp†L509-L538】
* `WWKeyboardClass::Message_Handler` consumes raw Win32 events (`HWND`, `MSG`, `WM_*`, `VK_*`) and performs Win32 coordinate translation via `ClientToScreen`, forwarding the results into the engine’s keyboard/mouse queues.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/keyboard.cpp†L574-L719】

## Mouse façade
* `WWMouseClass`’s constructor and state store an `HWND` for confinement/capture duties, so any replacement must hand it a window abstraction in place of the Win32 handle.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/wwmouse.h†L49-L118】

## DirectDraw glue
* `ddraw.cpp` binds DirectDraw to the Win32 window by passing `MainWindow` into `SetCooperativeLevel` and by using the global for all debug/error `MessageBox` reporting.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/ddraw.cpp†L247-L275】【F:Generals/Code/Libraries/Source/WWVegas/WWLib/ddraw.cpp†L327-L343】
* The exported `Set_Video_Mode` signature is still declared with an `HWND`, even though the implementation ignores the parameter after `MainWindow` has been initialized.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/ddraw.cpp†L348-L387】【F:Generals/Code/Libraries/Source/WWVegas/WWLib/misc.h†L51-L58】

## Resource loading helpers
* `data.cpp` routes all string-resource and raw-resource lookups through the process-global `ProgramInstance` via `LoadString`, `FindResource`, and `LoadResource`, making the HINSTANCE façade mandatory for localized text/assets today.【F:Generals/Code/Libraries/Source/WWVegas/WWLib/data.cpp†L244-L283】

## Summary of duplication
* Every touch point above has a byte-for-byte duplicate under `GeneralsMD/Code/Libraries/Source/WWVegas/WWLib`, so any abstraction layer must update both trees in lock-step once the new platform layer is available.【F:GeneralsMD/Code/Libraries/Source/WWVegas/WWLib/win.cpp†L19-L49】
