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
    unsigned long logical_sectors_per_fat;
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


int validate_FAT32_boot_sector(Fat32BootSector bs){
    char name[5] = {'F','A','T','3','2'};
    for(int i = 0; i < 5; i++){
        if(bs.fs_type[i] != name[i]){
            std::cout<<"Structure is not FAT32"<<std::endl;
            return 0;
        }
    }
    if(bs.sector_size > 4096 || bs.sector_size<128){
        std::cout<<"Invalid sector size"<<std::endl;
        return 0;
    }
    if((bs.sectors_per_cluster !=1 && bs.sectors_per_cluster%2 != 0) || bs.sectors_per_cluster>128){
        std::cout<<"Invalid number of sectors per cluster"<<std::endl;
        return 0;
    }
    if(bs.hidden_sectors > 100 || bs.hidden_sectors>bs.total_sectors_long){
        std::cout<<"Too many hidden sectors"<<std::endl;
        return 0;
    }
    if(bs.number_of_fats > 100){
        std::cout<<"Too many hidden sectors"<<std::endl;
        return 0;
    }

    return 1;
}

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
        return -1;
    }


    Fat32BootSector bs;
    std::cout<<"Reading FAT32 boot sector from beginning of file..."<<std::endl;
    fread(&bs, sizeof(Fat32BootSector), 1, f);

    if(validate_FAT32_boot_sector(bs)){
        std::cout<<"Valid FAT32 boot sector found"<<std::endl;
        fclose(f);
        return 0;
    }

    std::cout<<"Default boot sector invalid. Checking backup boot sector..."<<std::endl;
    int backup = 6;
    if(bs.backup_boot_sector >2 && bs.backup_boot_sector < bs.reserved_sectors)
        backup = bs.backup_boot_sector;
    int sector_size = 512;
    if(bs.sector_size > 128 && bs.sector_size < 4096)
        sector_size = bs.sector_size;
    fseek(f,backup*sector_size,SEEK_SET);
    fread(&bs, sizeof(Fat32BootSector), 1, f);

    if(validate_FAT32_boot_sector(bs)){
        std::cout<<"Valid FAT32 backup boot sector found"<<std::endl;
        fclose(f);
        return 0;
    }
    long fat_start = bs.reserved_sectors*bs.sector_size;
    //validate that each entry in table is valid and there are no lost clusters(clusters chains that don't belong to files)
    long data_start = fat_start + bs.number_of_fats*bs.fat_size_sectors*bs.sector_size;
    char* old_table;
    for(int k = 0; k < bs.number_of_fats; k+= 1){
        char* table = (char*)malloc(bs.fat_size_sectors*bs.sector_size);
        fseek(f, fat_start+k*bs.fat_size_sectors*bs.sector_size, SEEK_SET);
        fread(table, sizeof(table), 1, f);
        for(unsigned long long i = 0;i<sizeof(table)/4;i++){
            unsigned long entry = (unsigned char)*(table + i*4);
            //check if entry is invalid/suspicious
            if(entry == 0xFFFFF6 || entry == 0xFFFFF0){
                return -1;
            }
            //check lost clusters
            unsigned long next_entry = (unsigned char)*(table + entry*4);
            while(next_entry != 0xFFFF0F){
                entry = next_entry;
                long cluster_start = data_start + bs.sectors_per_cluster*bs.sector_size*entry;
                char* cluster_data = (char*)malloc(bs.sectors_per_cluster*bs.sector_size);
                fseek(f,cluster_start,SEEK_SET);
                fread(cluster_data,sizeof(cluster_data),1,f);
                next_entry = (unsigned char)*(table + entry*4);
                bool has_data = false;
                for(long j = 0; j< bs.sectors_per_cluster*bs.sector_size;j++){
                    if((*cluster_data + j) != 0){
                        has_data = true;
                        break;
                    }
                }
                if(!has_data){
                    free(cluster_data);
                    return -1;
                }
                free(cluster_data);
            }
        }
        free(table);
    }

    std::cout<<"FAT32 not found, checking if FAT32 partition exists..."<<std::endl;
    PartitionTable pt[4];
    int i;
    fseek(f, 0xBE, SEEK_SET);
    fread(pt, sizeof(PartitionTable), 4, f);

    for(i=0; i<4; i++) {
        if(pt[i].partition_type == 12 || pt[i].partition_type == 13){
            std::cout<<"FAT32 partition found"<<std::endl;
            break;
        }
    }
    if(i==4){
        std::cout<<"No FAT32 partition found"<<std::endl;
        return -1;
    }

    fseek(f, pt[i].start_sector, SEEK_SET);
    fread(&bs, sizeof(Fat32BootSector), 1, f);

    if(validate_FAT32_boot_sector(bs)){
        std::cout<<"Valid FAT32 backup boot sector found"<<std::endl;
        fclose(f);
        return 0;
    }
    std::cout<<"File is invalid FAT32 disk image"<<std::endl;
    fclose(f);
    return -1;
}
