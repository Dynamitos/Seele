slangc ForwardPlus.slang -target spirv -entry vertexMain -profile vs_6_0 -o vertex.spv -I lib/ -D USE_INSTANCING=0 -D NUM_MATERIAL_TEXCOORDS=1
slangc ForwardPlus.slang -target spirv -entry fragmentMain -profile ps_6_0 -o fragment.spv -I lib/ -D USE_INSTANCING=0 -D NUM_MATERIAL_TEXCOORDS=1