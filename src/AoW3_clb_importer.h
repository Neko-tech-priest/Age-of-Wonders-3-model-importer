// Default = 0
// Normal Map = 1
// Mask = 2

struct AoW3_clb_format
{
    uint8_t constant[12];//0x 43 52 4c 00 60 00 41 00 01 00 00 00
    uint32_t unknown_adress_1;
    uint8_t zero[8];
    uint8_t unknown[11];
    uint32_t constant_1;//0x 01
    uint32_t libraryName_length;
};
void AoW3_clb_read(const char* path, AoW3_Mesh*& meshes, uint32_t &meshesCount, Image*& textures, uint32_t &texturesCount);
