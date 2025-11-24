/** Description:
This C++ program functions as a dedicated utility for converting standard font files (like TrueType) into a format suitable for direct integration into other C++ projects, often for use in environments with limited resources, such as embedded systems.

The core job of the program is to generate a comprehensive C++ header file containing the raw visual data of a font.

The process starts by requiring two pieces of information from the user: the path to the font file and the exact desired size (in pixels). It then uses a powerful external library specialized for handling typography (FreeType) to load and interpret the font's complex outlines at the specified size.

Once initialized, the program systematically examines every common printable character (letters, numbers, and standard symbols). For each character, it instructs the font engine to draw the character and provide a grayscale pixel map, detailing the character's visual shape.

This crucial data—including the character's precise width, its vertical position, and the intensity value of every single pixel—is then extracted. The program writes this numerical pixel information into a new header file as organized, constant lists of numbers.

The final output is a single, structured header file that contains all the character data, along with pointers that link everything together into a usable "Font" structure. This output allows other C++ programs to use the font without needing to access the original font file, making deployment simpler and faster.

sha: d2607b576964e55ee5756169b78d9c47391456947f73d82a2f3d6834f4669a4f 
*/
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
        outputHeaderPath = outputHeaderPath.substr(outputHeaderPath.find_last_of("/\\") + 1);
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
        std::cerr << "Could not open output header file: " << outputHeaderPath << std::endl;
        return 1;
    }

    // remove full path and get only filemame
    std::string fontName = outputHeaderPath.substr(0, outputHeaderPath.find_last_of("."));

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
        int top = fontSize - face->glyph->bitmap_top;
        if (top < 0) {
            top = 0;
        } 
        headerFile << "    " << std::to_string(top) << ", // margin-top: " << std::to_string(top) << "\n";
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
