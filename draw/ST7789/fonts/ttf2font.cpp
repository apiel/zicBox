// g++ ttf2font.cpp -o ttf2font -I/usr/local/include/freetype2 -I/usr/include/freetype2 -lfreetype

#include <cstdint>
#include <fstream>
#include <ft2build.h>
#include <iomanip>
#include <iostream>
#include <regex>
#include <vector>
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
        outputHeaderPath = fontPath.substr(0, fontPath.find_last_of(".")) + "_" + std::to_string(fontSize) + ".h";
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
    headerFile << "#include \"Font.h\"\n\n";

    headerFile << "const uint8_t " << fontName << "_font_height = " << fontSize << ";\n\n";

    for (int i = 32; i < 127; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cerr << "Could not load character " << i << std::endl;
            return 1;
        }
        headerFile << "const uint8_t " << fontName << "_char" << std::to_string(i) << "[] = { // \"" << (char)i << "\"\n";
        headerFile << "    " << std::to_string(face->glyph->bitmap.width) << ", // width: " << std::to_string(face->glyph->bitmap.width) << "\n";
        headerFile << "    " << std::to_string(fontSize - face->glyph->bitmap_top) << ", // margin-top: " << std::to_string(fontSize - face->glyph->bitmap_top) << "\n";
        headerFile << "    " << std::to_string(face->glyph->bitmap.rows) << ", // rows: " << std::to_string(face->glyph->bitmap.rows) << "\n";
        headerFile << "    ";
        for (int y = 0; y < fontSize; y++) {
            for (int x = 0; x < face->glyph->bitmap.width; x++) {
                unsigned char* alpha = &face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
                headerFile << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)*alpha << ", ";
            }
        }
        headerFile << "\n";
        headerFile << "};\n";
    }

    headerFile << "const uint8_t* " << fontName << "_data[] = {\n";
    headerFile << "    &" << fontName << "_font_height,\n";
    for (int i = 32; i < 127; i++) {
        headerFile << "    &" << fontName << "_char" << std::to_string(i) << "[0],\n";
    }
    headerFile << "};\n\n";

    headerFile << "Font " << fontName << " = {\n";
    headerFile << "    (void*)" << fontName << "_data\n";
    headerFile << "};\n\n";

    headerFile.close();
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    std::cout << "Font header generated successfully: " << outputHeaderPath << std::endl;

    return 0;
}
