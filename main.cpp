// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <fstream>
#include "options_parser.h"

typedef struct {
    unsigned char jmp[3];
    char oem[8];
    unsigned short sector_size;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char number_of_fats;
    unsigned short root_dir_entries;
    unsigned short total_sectors_short; // if zero, later field is used
    unsigned char media_descriptor;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short number_of_heads;
    unsigned long hidden_sectors;
    unsigned long total_sectors_long;
    unsigned long logical_sectors;
    unsigned short drive_description;
    unsigned short version;
    unsigned long root_dir_start;
    unsigned short information_sector;
    unsigned short backup_boot_sector;
    unsigned char reserved[12];

    unsigned char drive_number;
    unsigned char current_head;
    unsigned char boot_signature;
    unsigned long volume_id;
    char volume_label[11];
    char fs_type[8];
    char boot_code[422];
    unsigned short boot_sector_signature;
} __attribute((packed)) Fat32BootSector;

typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned long start_sector;
    unsigned long length_sectors;
} __attribute((packed)) PartitionTable;


int main(int argc, char* argv[]) {
    if(argc < 2){
        std::cout<<"File path not specified"<<std::endl;
        return -1;
    }
    command_line_options_t command_line_options{argc, argv};
    std::string filename = command_line_options.get_filenames()[0];

    FILE *f = fopen(filename.c_str(),"rb");
    if(f == nullptr){
        std::cout<<strerror(errno)<<std::endl;
        std::cout<<filename.c_str()<<" Invalid file"<<std::endl;
    }

    PartitionTable pt[4];
    int i;
    fseek(f, 0x1BE, SEEK_SET);
    fread(pt, sizeof(PartitionTable), 4, f);

    for(i=0; i<4; i++) {
        if(pt[i].partition_type == 12 || pt[i].partition_type == 13){
            std::cout<<"FAT32 partition found"<<std::endl;
            break;
        }
    }

    std::cout<<pt[i].start_sector<<std::endl;
    Fat32BootSector bs;
    fseek(f, pt[i].start_sector, SEEK_SET);
    fread(&bs, sizeof(Fat32BootSector), 1, f);

    std::cout<<"Value: "<<bs.jmp<<std::endl;
    fclose(f);
    return 0;
}
