#ifndef ANALYZER_BASE64_H
#define ANALYZER_BASE64_H
#include <string>

namespace AVSAnalyzer {

    const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    class Base64 {
    private:
        inline bool is_base64(unsigned char c) {
            return (isalnum(c) || (c == '+') || (c == '/'));
        }
    public:
        void encode(unsigned char* in_data, int in_size, std::string& out_encoded) {

            int i = 0;
            int j = 0;
            unsigned char char_array_3[3];
            unsigned char char_array_4[4];

            while (in_size--) {
                char_array_3[i++] = *(in_data++);
                if (i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for (i = 0; (i < 4); i++)
                        out_encoded += base64_chars[char_array_4[i]];
                    i = 0;
                }
            }

            if (i)
            {
                for (j = i; j < 3; j++)
                    char_array_3[j] = '\0';

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; (j < i + 1); j++)
                    out_encoded += base64_chars[char_array_4[j]];

                while ((i++ < 3))
                    out_encoded += '=';

            }


        }
        void encode(const std::string& in_str, std::string& out_encoded) {
            int in_size = in_str.size();
            const char* in_data = in_str.data();

            int i = 0;
            int j = 0;
            unsigned char char_array_3[3];
            unsigned char char_array_4[4];

            while (in_size--) {
                char_array_3[i++] = *(in_data++);
                if (i == 3) {
                    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                    char_array_4[3] = char_array_3[2] & 0x3f;

                    for (i = 0; (i < 4); i++)
                        out_encoded += base64_chars[char_array_4[i]];
                    i = 0;
                }
            }

            if (i)
            {
                for (j = i; j < 3; j++)
                    char_array_3[j] = '\0';

                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (j = 0; (j < i + 1); j++)
                    out_encoded += base64_chars[char_array_4[j]];

                while ((i++ < 3))
                    out_encoded += '=';

            }


        }
        void decode(const std::string& in_str, std::string& out_decoded) {
            int in_len = in_str.size();
            int i = 0;
            int j = 0;
            int in_ = 0;

            uint8_t char_array_4[4], char_array_3[3];


            while (in_len-- && (in_str[in_] != '=') && is_base64(in_str[in_])) {
                char_array_4[i++] = in_str[in_]; in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = base64_chars.find(char_array_4[i]);

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        out_decoded += char_array_3[i];
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j < 4; j++)
                    char_array_4[j] = 0;

                for (j = 0; j < 4; j++)
                    char_array_4[j] = base64_chars.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++) out_decoded += char_array_3[j];
            }

        }
        std::string decode(const std::string& in_str) {
            std::string out_decoded;

            int in_len = in_str.size();
            int i = 0;
            int j = 0;
            int in_ = 0;

            uint8_t char_array_4[4], char_array_3[3];

            while (in_len-- && (in_str[in_] != '=') && is_base64(in_str[in_])) {
                char_array_4[i++] = in_str[in_]; in_++;
                if (i == 4) {
                    for (i = 0; i < 4; i++)
                        char_array_4[i] = base64_chars.find(char_array_4[i]);

                    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                    for (i = 0; (i < 3); i++)
                        out_decoded += char_array_3[i];
                    i = 0;
                }
            }

            if (i) {
                for (j = i; j < 4; j++)
                    char_array_4[j] = 0;

                for (j = 0; j < 4; j++)
                    char_array_4[j] = base64_chars.find(char_array_4[j]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (j = 0; (j < i - 1); j++) out_decoded += char_array_3[j];
            }

            return out_decoded;
        }
    };
}


#endif //ANALYZER_BASE64_H