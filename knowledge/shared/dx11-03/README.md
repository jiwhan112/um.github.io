# DX11 03 pipeline CPU model
No actual GPU or D3D11 API usage. Demonstrates an explicit *lesson policy* for required state, DrawIndexed bounds, IA stride, index format, row-vector matrix math, and CPU-side SRV/RTV hazard avoidance. D3D11 itself may accept null PS/RTV/zero-index draws depending on circumstances: do not mistake this validator for the runtime spec. State binding and shaders must be tested separately on Windows.

`cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure -V`
Use `-DLAB_CXX_STANDARD=11` for compatibility build.

`d3d11_triangle_window.cpp` is an additional **Windows-only** complete indexed-triangle sample, compiling inline VS/PS HLSL through `D3DCompile`, binding input layout, immutable VB/IB, RTV, viewport, shaders, and DrawIndexed. It uses legacy discard swap effect to match the didactic sample, not modern flip-model guidance. **Not compiled, linked, or executed on this Linux host**. On Windows with Visual Studio/Windows SDK: `cmake -S . -B build -A x64 && cmake --build build --config Debug` then launch `dx11_pipeline_window.exe`. Record Windows SDK, D3DCompiler availability, HRESULTs, resize behavior, final image and Debug Layer messages.
