// g++ ttf2font.cpp -o ttf2font -I/usr/local/include/freetype2 -I/usr/include/freetype2 -lfreetype

#include <cstdint>
#include <fstream>
#include <ft2build.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <regex>
#include FT_FREETYPE_H

#define FONT_SIZE 8
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

void generateFontHeader(const std::string& fontPath, const std::string& outputHeaderPath)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Could not initialize FreeType library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "Could not load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, FONT_WIDTH, FONT_HEIGHT);

    std::ofstream headerFile(outputHeaderPath);
    if (!headerFile.is_open()) {
        std::cerr << "Could not open output header file" << std::endl;
        return;
    }

    headerFile << "#pragma once\n\n";
    headerFile << "#include <cstdint>\n\n";
    headerFile << "uint8_t GeneratedFont[] = {\n";
    headerFile << "    " << FONT_WIDTH << ", " << FONT_HEIGHT << ", // " << FONT_WIDTH << "x" << FONT_HEIGHT << " pixels\n";

    for (int i = 32; i < 128; i++) { // ASCII characters from 32 to 127
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cerr << "Could not load character " << i << std::endl;
            continue;
        }

        headerFile << "    ";
        for (int y = 0; y < FONT_HEIGHT; y++) {
            uint8_t byte = 0;
            for (int x = 0; x < FONT_WIDTH; x++) {
                if (x < face->glyph->bitmap.width && y < face->glyph->bitmap.rows) {
                    byte |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x] > 0) ? (1 << (7 - x)) : 0;
                }
            }
            headerFile << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)byte << ", ";
        }
        headerFile << "// " << (char)i << "\n";
    }

    headerFile << "};\n";

    headerFile.close();
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    std::cout << "Font header generated successfully: " << outputHeaderPath << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <font_path> [<output_header_path>]" << std::endl;
        return 1;
    }

    std::string fontPath = argv[1];

    std::string outputHeaderPath;
    if (argc > 2) {
        outputHeaderPath = argv[2];
    } else {
        outputHeaderPath = fontPath.substr(0, fontPath.find_last_of(".")) + ".h";
        outputHeaderPath = std::regex_replace(outputHeaderPath, std::regex("-"), "");
    }

    generateFontHeader(fontPath, outputHeaderPath);

    return 0;
}
