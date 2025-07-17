#include <stdio.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb+");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    uint16_t checksum = 0;
    uint32_t offset = 0;

    int byte1, byte2;
    while ((byte1 = fgetc(fp)) != EOF && (byte2 = fgetc(fp)) != EOF) {
        if (offset != 0x80) {
            uint16_t word = ((uint8_t)byte1 << 8) | (uint8_t)byte2;
            checksum += word;
        }
        offset += 2;
    }

    if (fseek(fp, 0x80, SEEK_SET) != 0) {
        printf("Seek failed to reach 0x80\n");
        fclose(fp);
        return 1;
    }

    uint8_t hi = (uint8_t)(checksum >> 8);
    uint8_t lo = (uint8_t)(checksum & 0xFF);

    if (fputc(hi, fp) == EOF || fputc(lo, fp) == EOF) {
        printf("Failed to write to reach 0x80/0x81\n");
        fclose(fp);
        return 1;
    }

    fclose(fp);

    printf("Checksum: 0x%04X\nFile size: %i\n", checksum, offset);
    return 0;
}
