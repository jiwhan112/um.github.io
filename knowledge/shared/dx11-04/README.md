# DX11 04 texture SRV/Sampler lesson

`main.cpp` is a portable CPU-only model for row pitch, texture sampling with Clamp/Wrap, multiplied vertex color, and simple per-mip view overlap. It does **not** implement Direct3D 11 texture objects or GPU sampling. `texture.hlsl` uses VS input POSITION/TEXCOORD, row_major WVP b0, Texture2D t0 and SamplerState s0; HLSL not compiled on Linux.

`cmake -S . -B build -DLAB_CXX_STANDARD=20 && cmake --build build && ctest --test-dir build -V`; change to 11 for compatibility. Windows texture rendering should verify SRV/sampler shader linkage and actual D3D11 API separately.
