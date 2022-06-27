// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <fstream>
#include "options_parser.h"

typedef struct fat_extBS_32
{
    //extended fat32 stuff
    unsigned int		table_size_32;
    unsigned short		extended_flags;
    unsigned short		fat_version;
    unsigned int		root_cluster;
    unsigned short		fat_info;
    unsigned short		backup_BS_sector;
    unsigned char 		reserved_0[12];
    unsigned char		drive_number;
    unsigned char 		reserved_1;
    unsigned char		boot_signature;
    unsigned int 		volume_id;
    unsigned char		volume_label[11];
    unsigned char		fat_type_label[8];

    friend std::istream & operator >>(std::istream & in, fat_extBS_32 & p) {
        unsigned int		table_size_32;
        unsigned short		extended_flags;
        unsigned short		fat_version;
        unsigned int		root_cluster;
        unsigned short		fat_info;
        unsigned short		backup_BS_sector;
        unsigned char 		reserved_0[12];
        unsigned char		drive_number;
        unsigned char 		reserved_1;
        unsigned char		boot_signature;
        unsigned int 		volume_id;
        unsigned char		volume_label[11];
        unsigned char		fat_type_label[8];
        in >> table_size_32 >> extended_flags >> fat_version
        >> root_cluster >> fat_info >> backup_BS_sector >> reserved_0
        >> drive_number >> reserved_1 >> boot_signature >> volume_id >>
        volume_label >> fat_type_label;
        p.table_size_32 = table_size_32;
        p.extended_flags = extended_flags;
        p.fat_version = fat_version;
        p.root_cluster = root_cluster;
        p.fat_info = fat_info;
        p.backup_BS_sector = backup_BS_sector;
        p.drive_number = drive_number;
        p.reserved_1 = reserved_1;
        p.boot_signature = boot_signature;
        for(int i =0;i<12;i++ ){
            p.reserved_0[i] = reserved_0[i];
        }
        for(int i =0;i<11;i++ ){
            p.volume_label[i] = volume_label[i];
        }
        for(int i =0;i<8;i++ ){
            p.fat_type_label[i] = fat_type_label[i];
        }

        return in;
    }

}__attribute__((packed)) fat_extBS_32_t;

int main(int argc, char* argv[]) {
    command_line_options_t command_line_options{argc, argv};
    std::string filename = command_line_options.get_filenames()[0];

    std::ifstream file(filename, std::ios::binary);

    fat_extBS_32_t FAT_data;

    file >> FAT_data;
    std::cout<<FAT_data.backup_BS_sector<<std::endl;

    return 0;
}
