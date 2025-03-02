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
    headerFile << "// First raw define the font height\n";
    headerFile << "// Then each line is a character\n";
    headerFile << "// Each character is a tuple of (width, bitmap)\n";
    headerFile << "// Bitmap is a series of pixels, where each pixel is a byte representing the alpha color for anti aliasing\n\n";
    headerFile << "const uint8_t " << fontName << "_data[] = {\n";
    headerFile << "    " << fontSize << ", // font height: " << fontSize << " pixels\n";

    for (int i = 32; i < 128; i++) { // ASCII characters from 32 to 127
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            std::cerr << "Could not load character " << i << std::endl;
            continue;
        }

        headerFile << "    /* " << (char)i << " */ " << face->glyph->bitmap.width << ", ";
        for (int y = 0; y < fontSize; y++) {
            for (int x = 0; x < face->glyph->bitmap.width; x++) {
                unsigned char *alpha = &face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
                headerFile << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)*alpha << ", ";
            }
        }
        headerFile << "\n";
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
