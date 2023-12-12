#version 450
#extension GL_EXT_mesh_shader : require
layout(row_major) uniform;
layout(row_major) buffer;

#line 27 0
layout(local_size_x = 3, local_size_y = 1, local_size_z = 1) in;
layout(max_vertices = 12) out;
layout(max_primitives = 4) out;
layout(triangles) out;
void main()
{

    SetMeshOutputsEXT(12U, 4U);

#line 46
    return;
}

