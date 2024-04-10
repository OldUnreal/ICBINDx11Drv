#if 0
//
// Generated by Microsoft (R) HLSL Shader Compiler 10.1
//
//
// Buffer Definitions: 
//
// cbuffer CommonBuffer
// {
//
//   float4 XAxis;                      // Offset:    0 Size:    16
//   float4 YAxis;                      // Offset:   16 Size:    16
//   float4 ZAxis;                      // Offset:   32 Size:    16
//   int bDoRot;                        // Offset:   48 Size:     4
//   int bDoUVHack;                     // Offset:   52 Size:     4 [unused]
//   float2 Pad3;                       // Offset:   56 Size:     8 [unused]
//
// }
//
// cbuffer FrameVariables
// {
//
//   float4x4 Proj;                     // Offset:    0 Size:    64
//   float Gamma;                       // Offset:   64 Size:     4 [unused]
//   float2 ViewSize;                   // Offset:   68 Size:     8 [unused]
//   int bDoSelection;                  // Offset:   76 Size:     4 [unused]
//   int bOneXLightmaps;                // Offset:   80 Size:     4 [unused]
//   int bCorrectFog;                   // Offset:   84 Size:     4 [unused]
//   int bHDR;                          // Offset:   88 Size:     4 [unused]
//   int GammaMode;                     // Offset:   92 Size:     4 [unused]
//   float HDRExpansion;                // Offset:   96 Size:     4 [unused]
//   float ResolutionScale;             // Offset:  100 Size:     4 [unused]
//   float WhiteLevel;                  // Offset:  104 Size:     4 [unused]
//   int bDepthDraw;                    // Offset:  108 Size:     4 [unused]
//   float DepthDrawLimit;              // Offset:  112 Size:     4 [unused]
//   float GammaOffsetRed;              // Offset:  116 Size:     4 [unused]
//   float GammaOffsetBlue;             // Offset:  120 Size:     4 [unused]
//   float GammaOffsetGreen;            // Offset:  124 Size:     4 [unused]
//
// }
//
//
// Resource Bindings:
//
// Name                                 Type  Format         Dim      HLSL Bind  Count
// ------------------------------ ---------- ------- ----------- -------------- ------
// FrameVariables                    cbuffer      NA          NA            cb0      1 
// CommonBuffer                      cbuffer      NA          NA            cb4      1 
//
//
//
// Input signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// POSITION                 0   xyzw        0     NONE   float   xyzw
// TEXCOORD                 0   xyzw        1     NONE   float   xy  
// COLOR                    0   xyzw        2     NONE   float   xyzw
// COLOR                    1   xyzw        3     NONE   float       
// TEXCOORD                 1   xyzw        4     NONE   float       
// TEXCOORD                 2   xyzw        5     NONE   float       
//
//
// Output signature:
//
// Name                 Index   Mask Register SysValue  Format   Used
// -------------------- ----- ------ -------- -------- ------- ------
// SV_POSITION              0   xyzw        0      POS   float   xyzw
// TEXCOORD                 0   xy          1     NONE   float   xy  
// COLOR                    2     z         1     NONE   float     z 
// COLOR                    0   xyzw        2     NONE   float   xyzw
// COLOR                    1   xyzw        3     NONE   float   xyzw
// COLOR                    3   x           4     NONE     int   x   
//
vs_4_0
dcl_constantbuffer CB4[4], immediateIndexed
dcl_constantbuffer CB0[4], immediateIndexed
dcl_input v0.xyzw
dcl_input v1.xy
dcl_input v2.xyzw
dcl_output_siv o0.xyzw, position
dcl_output o1.xy
dcl_output o1.z
dcl_output o2.xyzw
dcl_output o3.xyzw
dcl_output o4.x
dcl_temps 1
dp4 r0.x, v0.xyzw, cb4[0].xyzw
dp4 r0.y, v0.xyzw, cb4[1].xyzw
dp4 r0.z, v0.xyzw, cb4[2].xyzw
movc r0.xyz, cb4[3].xxxx, r0.xyzx, v0.xyzx
mov r0.w, l(1.000000)
dp4 o0.x, r0.xyzw, cb0[0].xyzw
dp4 o0.y, r0.xyzw, cb0[1].xyzw
dp4 o0.z, r0.xyzw, cb0[2].xyzw
dp4 o0.w, r0.xyzw, cb0[3].xyzw
mov o1.z, r0.z
ge r0.x, l(1.000000), r0.z
and o4.x, r0.x, l(1)
mov o1.xy, v1.xyxx
mov o2.xyzw, v2.xyzw
mov o3.xyzw, l(0,0,0,0)
ret 
// Approximately 16 instruction slots used
#endif

const BYTE g_VertShader[] =
{
     68,  88,  66,  67, 165,  41, 
     23, 214, 185,  69, 163, 135, 
    143, 167,  85, 117, 129,  50, 
    208, 170,   1,   0,   0,   0, 
    168,   8,   0,   0,   5,   0, 
      0,   0,  52,   0,   0,   0, 
     92,   4,   0,   0,  20,   5, 
      0,   0, 208,   5,   0,   0, 
     44,   8,   0,   0,  82,  68, 
     69,  70,  32,   4,   0,   0, 
      2,   0,   0,   0, 120,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   0,   0,   0,   0,   4, 
    254, 255,   0,   1,   0,   0, 
    246,   3,   0,   0,  92,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   1,   0,   0,   0, 
    107,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,  70, 114,  97, 109, 
    101,  86,  97, 114, 105,  97, 
     98, 108, 101, 115,   0,  67, 
    111, 109, 109, 111, 110,  66, 
    117, 102, 102, 101, 114,   0, 
    107,   0,   0,   0,   6,   0, 
      0,   0, 168,   0,   0,   0, 
     64,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     92,   0,   0,   0,  16,   0, 
      0,   0, 148,   1,   0,   0, 
    128,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     56,   1,   0,   0,   0,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0,  64,   1, 
      0,   0,   0,   0,   0,   0, 
     80,   1,   0,   0,  16,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0,  64,   1, 
      0,   0,   0,   0,   0,   0, 
     86,   1,   0,   0,  32,   0, 
      0,   0,  16,   0,   0,   0, 
      2,   0,   0,   0,  64,   1, 
      0,   0,   0,   0,   0,   0, 
     92,   1,   0,   0,  48,   0, 
      0,   0,   4,   0,   0,   0, 
      2,   0,   0,   0, 100,   1, 
      0,   0,   0,   0,   0,   0, 
    116,   1,   0,   0,  52,   0, 
      0,   0,   4,   0,   0,   0, 
      0,   0,   0,   0, 100,   1, 
      0,   0,   0,   0,   0,   0, 
    126,   1,   0,   0,  56,   0, 
      0,   0,   8,   0,   0,   0, 
      0,   0,   0,   0, 132,   1, 
      0,   0,   0,   0,   0,   0, 
     88,  65, 120, 105, 115,   0, 
    171, 171,   1,   0,   3,   0, 
      1,   0,   4,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     89,  65, 120, 105, 115,   0, 
     90,  65, 120, 105, 115,   0, 
     98,  68, 111,  82, 111, 116, 
      0, 171,   0,   0,   2,   0, 
      1,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     98,  68, 111,  85,  86,  72, 
     97,  99, 107,   0,  80,  97, 
    100,  51,   0, 171,   1,   0, 
      3,   0,   1,   0,   2,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  20,   3,   0,   0, 
      0,   0,   0,   0,  64,   0, 
      0,   0,   2,   0,   0,   0, 
     28,   3,   0,   0,   0,   0, 
      0,   0,  44,   3,   0,   0, 
     64,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0,  68,   3,   0,   0, 
     68,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
    132,   1,   0,   0,   0,   0, 
      0,   0,  77,   3,   0,   0, 
     76,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0,  90,   3,   0,   0, 
     80,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0, 105,   3,   0,   0, 
     84,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0, 117,   3,   0,   0, 
     88,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0, 122,   3,   0,   0, 
     92,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0, 132,   3,   0,   0, 
     96,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0, 145,   3,   0,   0, 
    100,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0, 161,   3,   0,   0, 
    104,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0, 172,   3,   0,   0, 
    108,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
    100,   1,   0,   0,   0,   0, 
      0,   0, 183,   3,   0,   0, 
    112,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0, 198,   3,   0,   0, 
    116,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0, 213,   3,   0,   0, 
    120,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0, 229,   3,   0,   0, 
    124,   0,   0,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
     52,   3,   0,   0,   0,   0, 
      0,   0,  80, 114, 111, 106, 
      0, 171, 171, 171,   3,   0, 
      3,   0,   4,   0,   4,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  71,  97, 109, 109, 
     97,   0, 171, 171,   0,   0, 
      3,   0,   1,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  86, 105, 101, 119, 
     83, 105, 122, 101,   0,  98, 
     68, 111,  83, 101, 108, 101, 
     99, 116, 105, 111, 110,   0, 
     98,  79, 110, 101,  88,  76, 
    105, 103, 104, 116, 109,  97, 
    112, 115,   0,  98,  67, 111, 
    114, 114, 101,  99, 116,  70, 
    111, 103,   0,  98,  72,  68, 
     82,   0,  71,  97, 109, 109, 
     97,  77, 111, 100, 101,   0, 
     72,  68,  82,  69, 120, 112, 
     97, 110, 115, 105, 111, 110, 
      0,  82, 101, 115, 111, 108, 
    117, 116, 105, 111, 110,  83, 
     99,  97, 108, 101,   0,  87, 
    104, 105, 116, 101,  76, 101, 
    118, 101, 108,   0,  98,  68, 
    101, 112, 116, 104,  68, 114, 
     97, 119,   0,  68, 101, 112, 
    116, 104,  68, 114,  97, 119, 
     76, 105, 109, 105, 116,   0, 
     71,  97, 109, 109,  97,  79, 
    102, 102, 115, 101, 116,  82, 
    101, 100,   0,  71,  97, 109, 
    109,  97,  79, 102, 102, 115, 
    101, 116,  66, 108, 117, 101, 
      0,  71,  97, 109, 109,  97, 
     79, 102, 102, 115, 101, 116, 
     71, 114, 101, 101, 110,   0, 
     77, 105,  99, 114, 111, 115, 
    111, 102, 116,  32,  40,  82, 
     41,  32,  72,  76,  83,  76, 
     32,  83, 104,  97, 100, 101, 
    114,  32,  67, 111, 109, 112, 
    105, 108, 101, 114,  32,  49, 
     48,  46,  49,   0, 171, 171, 
     73,  83,  71,  78, 176,   0, 
      0,   0,   6,   0,   0,   0, 
      8,   0,   0,   0, 152,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   0,   0,   0,   0, 
     15,  15,   0,   0, 161,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   1,   0,   0,   0, 
     15,   3,   0,   0, 170,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   2,   0,   0,   0, 
     15,  15,   0,   0, 170,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   3,   0,   0,   0, 
     15,   0,   0,   0, 161,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   4,   0,   0,   0, 
     15,   0,   0,   0, 161,   0, 
      0,   0,   2,   0,   0,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,   5,   0,   0,   0, 
     15,   0,   0,   0,  80,  79, 
     83,  73,  84,  73,  79,  78, 
      0,  84,  69,  88,  67,  79, 
     79,  82,  68,   0,  67,  79, 
     76,  79,  82,   0,  79,  83, 
     71,  78, 180,   0,   0,   0, 
      6,   0,   0,   0,   8,   0, 
      0,   0, 152,   0,   0,   0, 
      0,   0,   0,   0,   1,   0, 
      0,   0,   3,   0,   0,   0, 
      0,   0,   0,   0,  15,   0, 
      0,   0, 164,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   3,  12, 
      0,   0, 173,   0,   0,   0, 
      2,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      1,   0,   0,   0,   4,  11, 
      0,   0, 173,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      2,   0,   0,   0,  15,   0, 
      0,   0, 173,   0,   0,   0, 
      1,   0,   0,   0,   0,   0, 
      0,   0,   3,   0,   0,   0, 
      3,   0,   0,   0,  15,   0, 
      0,   0, 173,   0,   0,   0, 
      3,   0,   0,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
      4,   0,   0,   0,   1,  14, 
      0,   0,  83,  86,  95,  80, 
     79,  83,  73,  84,  73,  79, 
     78,   0,  84,  69,  88,  67, 
     79,  79,  82,  68,   0,  67, 
     79,  76,  79,  82,   0, 171, 
     83,  72,  68,  82,  84,   2, 
      0,   0,  64,   0,   1,   0, 
    149,   0,   0,   0,  89,   0, 
      0,   4,  70, 142,  32,   0, 
      4,   0,   0,   0,   4,   0, 
      0,   0,  89,   0,   0,   4, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   4,   0,   0,   0, 
     95,   0,   0,   3, 242,  16, 
     16,   0,   0,   0,   0,   0, 
     95,   0,   0,   3,  50,  16, 
     16,   0,   1,   0,   0,   0, 
     95,   0,   0,   3, 242,  16, 
     16,   0,   2,   0,   0,   0, 
    103,   0,   0,   4, 242,  32, 
     16,   0,   0,   0,   0,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  50,  32,  16,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3,  66,  32,  16,   0, 
      1,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      2,   0,   0,   0, 101,   0, 
      0,   3, 242,  32,  16,   0, 
      3,   0,   0,   0, 101,   0, 
      0,   3,  18,  32,  16,   0, 
      4,   0,   0,   0, 104,   0, 
      0,   2,   1,   0,   0,   0, 
     17,   0,   0,   8,  18,   0, 
     16,   0,   0,   0,   0,   0, 
     70,  30,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      4,   0,   0,   0,   0,   0, 
      0,   0,  17,   0,   0,   8, 
     34,   0,  16,   0,   0,   0, 
      0,   0,  70,  30,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   4,   0,   0,   0, 
      1,   0,   0,   0,  17,   0, 
      0,   8,  66,   0,  16,   0, 
      0,   0,   0,   0,  70,  30, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   4,   0, 
      0,   0,   2,   0,   0,   0, 
     55,   0,   0,  10, 114,   0, 
     16,   0,   0,   0,   0,   0, 
      6, 128,  32,   0,   4,   0, 
      0,   0,   3,   0,   0,   0, 
     70,   2,  16,   0,   0,   0, 
      0,   0,  70,  18,  16,   0, 
      0,   0,   0,   0,  54,   0, 
      0,   5, 130,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     17,   0,   0,   8,  18,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,  17,   0,   0,   8, 
     34,  32,  16,   0,   0,   0, 
      0,   0,  70,  14,  16,   0, 
      0,   0,   0,   0,  70, 142, 
     32,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,  17,   0, 
      0,   8,  66,  32,  16,   0, 
      0,   0,   0,   0,  70,  14, 
     16,   0,   0,   0,   0,   0, 
     70, 142,  32,   0,   0,   0, 
      0,   0,   2,   0,   0,   0, 
     17,   0,   0,   8, 130,  32, 
     16,   0,   0,   0,   0,   0, 
     70,  14,  16,   0,   0,   0, 
      0,   0,  70, 142,  32,   0, 
      0,   0,   0,   0,   3,   0, 
      0,   0,  54,   0,   0,   5, 
     66,  32,  16,   0,   1,   0, 
      0,   0,  42,   0,  16,   0, 
      0,   0,   0,   0,  29,   0, 
      0,   7,  18,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   0,   0, 128,  63, 
     42,   0,  16,   0,   0,   0, 
      0,   0,   1,   0,   0,   7, 
     18,  32,  16,   0,   4,   0, 
      0,   0,  10,   0,  16,   0, 
      0,   0,   0,   0,   1,  64, 
      0,   0,   1,   0,   0,   0, 
     54,   0,   0,   5,  50,  32, 
     16,   0,   1,   0,   0,   0, 
     70,  16,  16,   0,   1,   0, 
      0,   0,  54,   0,   0,   5, 
    242,  32,  16,   0,   2,   0, 
      0,   0,  70,  30,  16,   0, 
      2,   0,   0,   0,  54,   0, 
      0,   8, 242,  32,  16,   0, 
      3,   0,   0,   0,   2,  64, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
     62,   0,   0,   1,  83,  84, 
     65,  84, 116,   0,   0,   0, 
     16,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      9,   0,   0,   0,   8,   0, 
      0,   0,   0,   0,   0,   0, 
      1,   0,   0,   0,   1,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   5,   0, 
      0,   0,   1,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0,   0,   0,   0,   0, 
      0,   0
};
