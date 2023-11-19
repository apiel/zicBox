#ifndef _VAL_SERIALIZE_SNDFILE_H_
#define _VAL_SERIALIZE_SNDFILE_H_

#include "../mapping.h"
#include <sndfile.h>

#define SETTING_CHUNK_ID "ZICG"

template <typename T>
class ValSerializeSndFile {
protected:
    SF_INFO sfinfo;

    struct ValSerialize {
        const char* _key;
        float initValue;
    };

public:
    std::vector<Val<T>*>& mapping;
    ValSerialize *serialize;
    SF_CHUNK_INFO chunk;

    ValSerializeSndFile(std::vector<Val<T>*>& mapping)
        : mapping(mapping)
    {
        serialize = new ValSerialize[mapping.size()];

        chunk = {
            SETTING_CHUNK_ID,
            sizeof(SETTING_CHUNK_ID),
            (unsigned int)(mapping.size() * sizeof(ValSerialize)),
            serialize
        };
    }

    ~ValSerializeSndFile()
    {
        delete[] serialize;
    }

    void saveSetting(const char* filename)
    {
        SNDFILE* file = sf_open(filename, SFM_WRITE, &sfinfo);
        if (!file) {
            printf("Error: could not open file %s [%s]\n", filename, sf_strerror(file));
            return;
        }

        for (int i = 0; i < mapping.size(); i++) {
            serialize[i]._key = mapping[i]->key();
            serialize[i].initValue = mapping[i]->get();
        }

        int res = sf_set_chunk(file, &chunk);

        printf("------- wrote %d bytes err: %s\n", res, sf_strerror(file));
        printf("chunk len %d\n", chunk.datalen);

        sf_close(file);
    }

    void loadSetting(SNDFILE* file)
    {
        SF_CHUNK_ITERATOR* it = sf_get_chunk_iterator(file, &chunk);

        // int err = sf_get_chunk_size (it, &chunk) ;
        // printf("------- err %d bytes err: %s\n", err, sf_strerror(file));
        // printf("chunk len %d\n", chunk.datalen);

        int res = sf_get_chunk_data(it, &chunk);
        printf("------- loaded %d bytes err: %s\n", res, sf_strerror(file));

        for (int i = 0; i < mapping.size(); i++) {
            // mapping[i]->set(serialize[i].initValue);
            printf(">>>> val: %s %f\n", mapping[i]->key(), serialize[i].initValue);
        }
    }

    void loadSetting(const char* filename)
    {
        SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
        if (!file) {
            printf("Error: could not open file %s [%s]\n", filename, sf_strerror(file));
            return;
        }
        loadSetting(file);
    }
};

#endif