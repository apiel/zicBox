// g++ ttf2font.cpp -o ttf2font -I/usr/local/include/freetype2 -I/usr/include/freetype2 -lfreetype

#include <cstdint>
#include <fstream>
#include <ft2build.h>
#include <iomanip>
#include <iostream>
#include <vector>
#include <regex>
#include FT_FREETYPE_H

int main(int argc, char* argv[])
{
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <font_path> <font_size> [<output_header_path>]" << std::endl;
        return 1;
    }

    std::string fontPath = argv[1];
    int fontSize = std::stoi(argv[2]);

    std::string outputHeaderPath;
    if (argc > 3) {
        outputHeaderPath = argv[3];
    } else {
        outputHeaderPath = fontPath.substr(0, fontPath.find_last_of(".")) + ".h";
        outputHeaderPath = std::regex_replace(outputHeaderPath, std::regex("-"), "");
    }

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Could not initialize FreeType library" << std::endl;
        return 1;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "Could not load font" << std::endl;
        return 1;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    std::ofstream headerFile(outputHeaderPath);
    if (!headerFile.is_open()) {
        std::cerr << "Could not open output header file" << std::endl;
        return 1;
    }

    // remove full path and get only filemame
    std::string fontName = outputHeaderPath.substr(outputHeaderPath.find_last_of("/\\") + 1);
    fontName = fontName.substr(0, fontName.find_last_of("."));

    headerFile << "#pragma once\n\n";
    headerFile << "#include <cstdint>\n\n";
    headerFile << "uint8_t " << fontName << "[] = {\n";
    headerFile << "    " << fontSize << ", " << fontSize << ", // " << fontSize << "x" << fontSize << " pixels\n";

    for (int i = 32; i < 128; i++) { // ASCII characters from 32 to 127
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cerr << "Could not load character " << i << std::endl;
            continue;
        }

        // headerFile << "    ";
        // for (int y = 0; y < fontSize; y++) {
        //     uint8_t byte = 0;
        //     for (int x = 0; x < fontSize; x++) {
        //         if (x < face->glyph->bitmap.width && y < face->glyph->bitmap.rows) {
        //             byte |= (face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x] > 0) ? (1 << (7 - x)) : 0;
        //         }
        //     }
        //     headerFile << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)byte << ", ";
        // }
        // headerFile << "// " << (char)i << "\n";
    }

    headerFile << "};\n";

    headerFile.close();
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    std::cout << "Font header generated successfully: " << outputHeaderPath << std::endl;

    return 0;
}
