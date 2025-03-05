//Gabrielle Akers
//COS350 Program 2
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>

//COMPRESSION

//Find the file's size
long findSize(int fd){
        struct stat fileStat;
        if(fstat(fd, &fileStat) < 0){
                printf("Error finding file length\n");
                close(fd);
                exit(1);
        }
        return fileStat.st_size;
}

//Perform bitwise operations
void shiftBits(unsigned char *in, unsigned char *out){
        out[0] = (in[0] & 0x7F) | ((in[1] & 0x01) << 7);
        out[1] = ((in[1] >> 1) & 0x3F) | ((in[2] & 0x03) << 6);
        out[2] = ((in[2] >> 2) & 0x1F) | ((in[3] & 0x07) << 5);
        out[3] = ((in[3] >> 3) & 0x0F) | ((in[4] & 0x0F) << 4);
        out[4] = ((in[4] >> 4) & 0x07) | ((in[5] & 0x1F) << 3);
        out[5] = ((in[5] >> 5) & 0x03) | ((in[6] & 0x3F) << 2);
        out[6] = ((in[6] >> 6) & 0x01) | ((in[7] & 0x7F) << 1);
}

//Compress 8 bit file into 7 bit
void compress(char *filename){
        int file = open(filename, O_RDONLY);
        if(file < 0){
                printf("File failed to open properly\n");
                exit(1);
        }
        long size = findSize(file);
        //New file
        char output[256];
        snprintf(output, sizeof(output), "%s.z827", filename);
        //Read and write permission
        int out = creat(output, S_IRUSR | S_IWUSR);
        write(out, &size, sizeof(size));
        unsigned char bitsIn[8];
        unsigned char bitsOut[7];
        long readBytes;
        while((readBytes = read(file, bitsIn, 8)) > 0){
                if(readBytes < 8){
                        for(long i = readBytes; i < 8; i++){
                                bitsIn[i] = 0;
                        }
                }
                shiftBits(bitsIn, bitsOut);
                write(out, bitsOut, 7);
        }
        close(file);
        close(out);
        //Delete original file
        unlink(file);
}

//DECOMPRESSION

//Shift bits back
void shiftBack(unsigned char *in, unsigned char *out){
        out[0] = in[0] & 0x7F;
        out[1] = ((in[0] >> 7) & 0x01) | ((in[1] & 0x3F) << 1);
        out[2] = ((in[1] >> 6) & 0x03) | ((in[2] & 0x1F) << 2);
        out[3] = ((in[2] >> 5) & 0x07) | ((in[3] & 0x0F) << 3);
        out[4] = ((in[3] >> 4) & 0x0F) | ((in[4] & 0x07) << 4);
        out[5] = ((in[4] >> 3) & 0x1F) | ((in[5] & 0x03) << 5);
        out[6] = ((in[5] >> 2) & 0x3F) | ((in[6] & 0x01) << 6);
        out[7] = (in[6] >> 1) & 0x7F;
}

//Turn 7bit file into 8bit
void decompress(char *filename){
        int file = open(filename, O_RDONLY);
        if(file < 0){
                printf("File failed to open properly\n");
                exit(1);
        }
        //Find original file size
        long size;
        read(file, &size, sizeof(size));
        //Create decompressed file
        char output[256];
        strncpy(output, filename, strlen(filename) - 5);
        output[strlen(filename) - 5] = '\0';
        //Gives read and write permission
        int out = creat(output, S_IRUSR | S_IWUSR);
        unsigned char bitsIn[7];
        unsigned char bitsOut[8];
        ssize_t bytesRead;
        long writeBytes = 0;
        while((bytesRead = read(file, bitsIn, 7)) > 0 && writeBytes < size){
                shiftBack(bitsIn, bitsOut);
                ssize_t toWrite = (writeBytes + 8 > size) ? (size - writeBytes) : 8;
                write(out, bitsOut, toWrite);
                writeBytes += toWrite;
        }
        close(file);
        close(out);
        //Delete compressed file
        unlink(filename);
}

int main(int argc, char *argv[]){
        //Check to see if the file is compressed
        if(strstr(argv[1], ".z827") == NULL){
                compress(argv[1]);
        }
        else{
                decompress(argv[1]);
        }
        return 0;
}
