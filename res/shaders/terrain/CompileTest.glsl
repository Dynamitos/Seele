#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 35 0
layout(std430, binding = 0) buffer StructuredBuffer_uint64_t_0 {
    uint64_t _data[];
} pParams_bitFieldBuffer_0;

#line 8
const uint  OCBT_bit_mask_0[18] = { 4294967295U, 4294967295U, 4294967295U, 4294967295U, 4294967295U, 4294967295U, 4294967295U, 65535U, 65535U, 65535U, 255U, 4294967295U, 4294967295U, 65535U, 255U, 15U, 3U, 1U };

#line 33
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 33
    uint _S1 = gl_GlobalInvocationID.x;

    pParams_bitFieldBuffer_0._data[uint(_S1)] = uint64_t(OCBT_bit_mask_0[_S1]);
    return;
}

