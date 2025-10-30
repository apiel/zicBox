#pragma once

#include "../mapping.h"

#define VAL_SERIALIZE_CHUNK_ID "ZIC_"

class ValSerializeSndFile {
protected:
    struct ValSerialize {
        std::string _key;
        float value;
    };

    uint32_t findChunk(FILE* file)
    {
        fseek(file, 0, SEEK_SET);

        uint32_t chunkID;
        uint32_t chunkSize;
        while (fread(&chunkID, 4, 1, file)) {
            if (chunkID == *(uint32_t*)VAL_SERIALIZE_CHUNK_ID) {
                fread(&chunkSize, 4, 1, file);
                return chunkSize;
            } else if (chunkID == *(uint32_t*)"data") {
                fread(&chunkSize, 4, 1, file);
                fseek(file, chunkSize, SEEK_CUR);
            }
        }

        return 0;
    }

public:
    std::vector<ValueInterface*>& mapping;

    ValSerializeSndFile(std::vector<ValueInterface*>& mapping)
        : mapping(mapping)
    {
    }

    void loadSetting(const char* filename)
    {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            printf("Error: could not open file %s for reading\n", filename);
            return;
        }
        uint32_t chunkSize = findChunk(file);

        if (chunkSize) {
            ValSerialize serialize[mapping.size()];
            fread(serialize, chunkSize, 1, file);

            for (int i = 0; i < mapping.size(); i++) {
                mapping[i]->set(serialize[i].value);
                // printf(">>>> val: %s %f\n", mapping[i]->key(), serialize[i].value);
            }
        }

        fclose(file);
    }

    void saveSetting(const char* filename)
    {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            printf("Error: could not open file %s for writing\n", filename);
            return;
        }

        uint32_t chunkSize = findChunk(file);
        long chunkPos = ftell(file);

        char tmpFilename[256];
        sprintf(tmpFilename, "%s.tmp", filename);
        FILE* tmpFile = fopen(tmpFilename, "wb");

        fseek(file, 0, SEEK_SET);
        for (long i = 0; i < chunkPos; i++) {
            char c;
            fread(&c, 1, 1, file);
            fwrite(&c, 1, 1, tmpFile);
        }

        ValSerialize serialize[mapping.size()];
        for (int i = 0; i < mapping.size(); i++) {
            serialize[i]._key = mapping[i]->key();
            serialize[i].value = mapping[i]->get();
        }
        uint32_t serializeSize = mapping.size() * sizeof(ValSerialize);

        if (!chunkSize) {
            fwrite(VAL_SERIALIZE_CHUNK_ID, 4, 1, tmpFile);
            fwrite(&serializeSize, 4, 1, tmpFile);
        }
        fwrite(&serialize, serializeSize, 1, tmpFile);
        // if serializeSize is not multiple of 4 bytes then add missing bytes
        uint32_t padding = 4 - (serializeSize % 4);
        if (padding != 4) {
            fwrite(&padding, 1, padding, tmpFile);
        }

        if (chunkSize) {
            // could this be a problem if padding is different than 4?
            // maybe padding should saved in serializeSize...
            fseek(file, chunkPos, SEEK_CUR);
            char c;
            while (fread(&c, 1, 1, file)) {
                fwrite(&c, 1, 1, tmpFile);
            }
        }

        fclose(tmpFile);
        fclose(file);

        remove(filename);
        rename(tmpFilename, filename);
    }
};
