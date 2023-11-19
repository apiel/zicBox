#ifndef _VAL_SERIALIZE_SNDFILE_H_
#define _VAL_SERIALIZE_SNDFILE_H_

#include "../mapping.h"

#define VAL_SERIALIZE_CHUNK_ID "ZIC_"

template <typename T>
class ValSerializeSndFile {
protected:
    struct ValSerialize {
        const char* _key;
        float initValue;
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
    std::vector<Val<T>*>& mapping;
    ValSerialize* serialize;

    ValSerializeSndFile(std::vector<Val<T>*>& mapping)
        : mapping(mapping)
    {
        serialize = new ValSerialize[mapping.size()];
    }

    ~ValSerializeSndFile()
    {
        delete[] serialize;
    }

    void loadSetting(const char* filename)
    {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            printf("Error: could not open file %s for reading\n", filename);
            return;
        }
        uint32_t chunkSize = findChunk(file);

        printf("::::::::::::: chunkSize: %d\n", chunkSize);

        if (chunkSize) {
            fread(serialize, chunkSize, 1, file);

            for (int i = 0; i < mapping.size(); i++) {
                // mapping[i]->set(serialize[i].initValue);
                printf(">>>> val: %s %f\n", mapping[i]->key(), serialize[i].initValue);
            }
        }

        fclose(file);
    }

    void saveSetting(FILE* file)
    {
        fwrite(VAL_SERIALIZE_CHUNK_ID, 4, 1, file);

        // uint32_t serializeSize = sizeof(serialize);
        uint32_t serializeSize = mapping.size() * sizeof(ValSerialize);
        fwrite(&serializeSize, 4, 1, file);
        fwrite(serialize, serializeSize, 1, file);

        // if serializeSize is not multiple of 4 bytes then add missing bytes
        uint32_t padding = 4 - (serializeSize % 4);
        if (padding != 4) {
            fwrite(&padding, 1, padding, file);
        }
    }

    void saveSetting(const char* filename)
    {
        FILE* file = fopen(filename, "ab");
        if (!file) {
            printf("Error: could not open file %s for writing\n", filename);
            return;
        }

        for (int i = 0; i < mapping.size(); i++) {
            serialize[i]._key = mapping[i]->key();
            serialize[i].initValue = mapping[i]->get();
        }

        uint32_t chunkSize = findChunk(file);

        printf("::::::::::::: chunkSize: %d\n", chunkSize);
        if (chunkSize) {
            printf(":::::::::::::copy\n");
            // if file exist make a copy
            char tmpFilename[256];
            sprintf(tmpFilename, "%s.tmp", filename);
            FILE* tmpFile = fopen(tmpFilename, "wb");

            long pos = ftell(file);
            char c;
            for (long i = 0; i < pos; i++) {
                fread(&c, 1, 1, file);
                fwrite(&c, 1, 1, tmpFile);
            }

            saveSetting(tmpFile);
            
            fseek(file, chunkSize, SEEK_CUR);
            while (fread(&c, 1, 1, file)) {
                fwrite(&c, 1, 1, tmpFile);
            }

            fclose(tmpFile);
        } else {
            fseek(file, 0, SEEK_END);
            saveSetting(file);
        }

        fclose(file);
    }
};

#endif