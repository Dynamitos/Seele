#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_KHR_memory_scope_semantics : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 178 0
layout(std430, binding = 2) buffer StructuredBuffer_uint_t_0 {
    uint _data[];
} pParams_classificationBuffer_0;

#line 178
layout(std430, binding = 9) buffer StructuredBuffer_uint_t_1 {
    uint _data[];
} pParams_cbtBuffer_0;

#line 63
layout(std430, binding = 10) buffer StructuredBuffer_uint64_t_0 {
    uint64_t _data[];
} pParams_bitFieldBuffer_0;

#line 119
const uint64_t  OCBT_bit_mask_0[18] = { 18446744073709551615UL, 18446744073709551615UL, 18446744073709551615UL, 18446744073709551615UL, 18446744073709551615UL, 18446744073709551615UL, 18446744073709551615UL, 65535UL, 65535UL, 65535UL, 255UL, 18446744073709551615UL, 18446744073709551615UL, 65535UL, 255UL, 15UL, 3UL, 1UL };

#line 141
const uint  OCBT_bit_count_0[18] = { 32U, 32U, 32U, 32U, 32U, 32U, 32U, 16U, 16U, 16U, 8U, 64U, 32U, 16U, 8U, 4U, 2U, 1U };

#line 97
const uint  OCBT_depth_offset_0[18] = { 0U, 32U, 96U, 224U, 480U, 992U, 2016U, 4064U, 6112U, 10208U, 18400U, 0U, 0U, 0U, 0U, 0U, 0U, 0U };

#line 163
shared uint  gs_cbtTree_0[831];


#line 171
void load_buffer_to_shared_memory_0(uint groupIndex_0)
{

#line 171
    uint e_0 = 0U;


    for(;;)
    {

#line 174
        if(int(e_0) < 13)
        {
        }
        else
        {

#line 174
            break;
        }
        uint target_element_0 = uint(13 * int(groupIndex_0) + int(e_0));
        if(int(target_element_0) < 831)
        {

#line 178
            gs_cbtTree_0[target_element_0] = pParams_cbtBuffer_0._data[uint(target_element_0)];

#line 177
        }

#line 174
        e_0 = e_0 + 1U;

#line 174
    }

#line 180
    controlBarrier(gl_ScopeWorkgroup, gl_ScopeWorkgroup, gl_StorageSemanticsShared, gl_SemanticsAcquireRelease);
    return;
}


#line 183
uint get_heap_element_0(uint id_0)
{

    uint real_heap_id_0 = id_0 - 1U;
    uint depth_0 = uint(log2(float(real_heap_id_0 + 1U)));


    uint first_bit_0 = OCBT_depth_offset_0[depth_0] + OCBT_bit_count_0[depth_0] * (real_heap_id_0 - ((1U << depth_0) - 1U));
    if(depth_0 < 11U)
    {



        return gs_cbtTree_0[first_bit_0 / 32U] >> first_bit_0 % 32U & uint(OCBT_bit_mask_0[depth_0]);
    }
    else
    {


        uint64_t target_bits_0 = pParams_bitFieldBuffer_0._data[uint(first_bit_0 / 64U)] >> first_bit_0 % 64U & OCBT_bit_mask_0[depth_0];


        return bitCount(uint(target_bits_0 >> 32)) + bitCount(uint(target_bits_0));
    }

#line 205
}




uint decode_bit_0(uint handle_0)
{

#line 210
    uint b_0;

#line 210
    uint heapElementID_0 = 1U;

#line 210
    uint currentDepth_0 = 0U;

#line 210
    uint bitCount_0 = handle_0;

#line 227
    for(;;)
    {

#line 227
        if(currentDepth_0 < 11U)
        {
        }
        else
        {

#line 227
            break;
        }

        uint heapValue_0 = get_heap_element_0(2U * heapElementID_0);


        if(bitCount_0 < heapValue_0)
        {

#line 233
            b_0 = 0U;

#line 233
        }
        else
        {

#line 233
            b_0 = 1U;

#line 233
        }


        uint _S1 = 2U * heapElementID_0 + b_0;


        uint _S2 = bitCount_0 - heapValue_0 * b_0;

#line 227
        uint currentDepth_1 = currentDepth_0 + 1U;

#line 227
        heapElementID_0 = _S1;

#line 227
        currentDepth_0 = currentDepth_1;

#line 227
        bitCount_0 = _S2;

#line 227
    }

#line 243
    uint _S3 = currentDepth_0 + 1U;


    uint64_t _S4 = pParams_bitFieldBuffer_0._data[uint(int(heapElementID_0) - 2048)];

#line 210
    uint _S5 = bitCount_0;

#line 210
    uint64_t mask_0 = 18446744073709551615UL;

#line 210
    currentDepth_0 = _S3;

#line 210
    bitCount_0 = 32U;

#line 210
    uint _S6 = _S5;

#line 249
    for(;;)
    {

#line 249
        if(currentDepth_0 < uint(log2(1.31072e+05)) + 1U)
        {
        }
        else
        {

#line 249
            break;
        }

#line 257
        uint64_t target_bits_1 = _S4 >> bitCount_0 * (2U * heapElementID_0 - 1U - ((1U << currentDepth_0) - 1U)) % 64U & mask_0;
        uint heapValue_1 = bitCount(uint(target_bits_1 >> 32)) + bitCount(uint(target_bits_1));


        if(_S6 < heapValue_1)
        {

#line 261
            b_0 = 0U;

#line 261
        }
        else
        {

#line 261
            b_0 = 1U;

#line 261
        }


        uint _S7 = 2U * heapElementID_0 + b_0;


        uint _S8 = _S6 - heapValue_1 * b_0;


        uint bitCount_1 = bitCount_0 / 2U;
        uint64_t _S9 = mask_0 >> bitCount_1;

#line 249
        uint currentDepth_2 = currentDepth_0 + 1U;

#line 249
        heapElementID_0 = _S7;

#line 249
        mask_0 = _S9;

#line 249
        currentDepth_0 = currentDepth_2;

#line 249
        bitCount_0 = bitCount_1;

#line 249
        _S6 = _S8;

#line 249
    }

#line 273
    return heapElementID_0 ^ 131072U;
}



uint decode_bit_complement_0(uint handle_1)
{

#line 278
    uint b_1;

#line 278
    uint heapElementID_1 = 1U;

#line 278
    uint currentDepth_3 = 0U;

#line 278
    uint c_0 = 65536U;

#line 278
    uint bitCount_2 = handle_1;

#line 299
    for(;;)
    {

#line 299
        if(currentDepth_3 < 11U)
        {
        }
        else
        {

#line 299
            break;
        }
        uint heapValue_2 = c_0 - get_heap_element_0(2U * heapElementID_1);
        if(bitCount_2 < heapValue_2)
        {

#line 302
            b_1 = 0U;

#line 302
        }
        else
        {

#line 302
            b_1 = 1U;

#line 302
        }

        uint _S10 = 2U * heapElementID_1 + b_1;
        uint _S11 = bitCount_2 - heapValue_2 * b_1;
        uint c_1 = c_0 / 2U;

#line 299
        uint currentDepth_4 = currentDepth_3 + 1U;

#line 299
        heapElementID_1 = _S10;

#line 299
        currentDepth_3 = currentDepth_4;

#line 299
        c_0 = c_1;

#line 299
        bitCount_2 = _S11;

#line 299
    }

#line 310
    uint _S12 = currentDepth_3 + 1U;


    uint64_t _S13 = pParams_bitFieldBuffer_0._data[uint(int(heapElementID_1) - 2048)];

#line 278
    uint _S14 = bitCount_2;

#line 278
    uint64_t mask_1 = 18446744073709551615UL;

#line 278
    currentDepth_3 = _S12;

#line 278
    bitCount_2 = 32U;

#line 278
    uint _S15 = _S14;

#line 316
    for(;;)
    {

#line 316
        if(currentDepth_3 < uint(log2(1.31072e+05)) + 1U)
        {
        }
        else
        {

#line 316
            break;
        }

#line 324
        uint64_t target_bits_2 = _S13 >> bitCount_2 * (2U * heapElementID_1 - 1U - ((1U << currentDepth_3) - 1U)) % 64U & mask_1;
        uint heapValue_3 = c_0 - bitCount(uint(target_bits_2 >> 32)) + bitCount(uint(target_bits_2));

        if(_S15 < heapValue_3)
        {

#line 327
            b_1 = 0U;

#line 327
        }
        else
        {

#line 327
            b_1 = 1U;

#line 327
        }

        uint _S16 = 2U * heapElementID_1 + b_1;
        uint _S17 = _S15 - heapValue_3 * b_1;
        uint c_2 = c_0 / 2U;


        uint bitCount_3 = bitCount_2 / 2U;
        uint64_t _S18 = mask_1 >> bitCount_3;

#line 316
        uint currentDepth_5 = currentDepth_3 + 1U;

#line 316
        heapElementID_1 = _S16;

#line 316
        mask_1 = _S18;

#line 316
        currentDepth_3 = currentDepth_5;

#line 316
        bitCount_2 = bitCount_3;

#line 316
        c_0 = c_2;

#line 316
        _S15 = _S17;

#line 316
    }

#line 338
    return heapElementID_1 ^ 131072U;
}



layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{

#line 343
    uint _S19 = gl_GlobalInvocationID.x;

    load_buffer_to_shared_memory_0(gl_LocalInvocationIndex);
    pParams_classificationBuffer_0._data[uint(_S19)] = decode_bit_0(_S19);
    pParams_classificationBuffer_0._data[uint(_S19 * 2U)] = decode_bit_complement_0(_S19);
    return;
}

