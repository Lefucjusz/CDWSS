#include "cd.h"
#include <dos.h>

/* MSCDEX request data types */
#pragma pack(1) //Align structs to byte

typedef struct
{
	uint8_t req_hdr_length; //Request header length
    uint8_t sub_unit; //Subunit number
    uint8_t command_code; //Command code
    uint16_t status; //Status of the command execution
    uint8_t reserved[8]; //Reserved for future use
} req_hdr_t; //Request header struct

typedef struct
{
	uint8_t function_code; //Function code
} tray_transport_t;

typedef struct
{
	uint8_t function_code; //Function code
	uint8_t lock_state; //State of the lock
} lock_t;

typedef struct
{
	uint8_t function_code; //Function code, for obtaining status = 6
	uint32_t status; //Device status value
} dev_status_t;

typedef struct
{
	uint8_t function_code; //Function code, for reading disc info = 10
    uint8_t first_track; //Number of the first track
    uint8_t last_track; //Number of the last track
    uint32_t last_sector; //Location of the last CD sector
} disc_info_t; //Disc info control block

typedef struct
{
	uint8_t function_code; //Function code, for reading track info = 11
    uint8_t track_no; //Number of the track to obtain information of
    uint32_t first_sector; //Location of the first track sector
    uint8_t track_format; //Information 
} track_info_t; //Track info control block

typedef struct
{
	req_hdr_t req_hdr; //Request header
	uint8_t media_descriptor; //Media descriptor
	uint32_t transfer_address; //Pointer to the control block
	uint16_t data_size; //Number of bytes to transfer
	uint16_t starting_sector; //Starting sector number
	uint32_t vol_id_ptr; //Pointer to vol ID
} ioctlio_t; //IOCTL I/O struct

typedef struct
{
	req_hdr_t req_hdr; //Request header
	uint8_t addressing_mode; //Addressing mode, 0 = High Sierra, 1 = Red Book
	uint32_t transfer_address; //Pointer to buffer for data read
	uint16_t read_length; //Number of sectors to read
	uint32_t starting_sector; //Sector to begin reading from
	uint8_t read_mode; //Reading mode, 0 = cooked, 1 = raw
	uint8_t interleave_size; //Usually ignored in modern drive, set 0
	uint8_t interleave_skip; //Usually ignored in modern drive, set 0
} read_long_t; //Read long struct

#pragma pack()

/* Functions */
static uint32_t cd_redbook_to_sector(uint32_t redbook)
{
	uint8_t min, sec, frame;
	uint32_t sector;
	frame = redbook & 0xFF;
	sec = (redbook >> 8) & 0xFF;
	min = (redbook >> 16) & 0xFF;
	sector = (((uint32_t)min*60 + (uint32_t)sec) * 75) + frame - 150; //As defined in Red Book
	return sector;
}

static void cd_request(uint16_t device, void* request) //Generates MSCDEX CD request
{
	union REGS reg;
	struct SREGS sreg;
	sreg.es = FP_SEG(request);
	reg.x.bx = FP_OFF(request);
	reg.x.cx = device;
	reg.x.ax = 0x1510; //MSCDEX CD-ROM driver request
	int86x(0x2F, &reg, &reg, &sreg); //MSCDEX interrupt
}

void cd_check_drives(uint16_t* drives, uint16_t* first_drive, uint16_t* mscdex_ver)
{
	union REGS reg;
	
	reg.x.ax = 0x1500; //MSCDEX number of CD-ROMs installed 
	reg.x.bx = 0; //Clear result
	int86(0x2F, &reg, &reg); //MSCDEX interrupt
	
	(*drives) = reg.x.bx; //Store number of drives
	
	if((*drives) == 0) //If no drives...
	{
		return; //...return
	}
	
	(*first_drive) = reg.x.cx; //Store number of first drive (A - 0, B - 1, C - 2...)
	
	reg.x.ax = 0x150C; //MSCDEX version
	int86(0x2F, &reg, &reg); //MSCDEX interrupt
	
	(*mscdex_ver) = reg.x.bx; //Store MSCDEX version in vvvvvvvv.ssssssss format (v - bit of version, s - bit of subversion)	
}

void cd_get_drive_letters(uint8_t* letters_array)
{
	union REGS reg;
	struct SREGS sreg;

	segread(&sreg);
	reg.x.ax = 0x150D; //MSCDEX read drive letters
	reg.x.bx = FP_OFF(letters_array);

	int86x(0x2F, &reg, &reg, &sreg); //MSCDEX interrupt
}	

uint16_t cd_init(uint16_t device)
{
	req_hdr_t cmd;
	
	cmd.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.command_code = 13; //Device open command code
	cmd.sub_unit = 0; //No subunit
	cmd.status = 0; //Clear status
	
	cd_request(device, &cmd);
	
	return cmd.status;	
}

uint16_t cd_deinit(uint16_t device)
{
	req_hdr_t cmd;
	
	cmd.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.command_code = 14; //Device close command code
	cmd.sub_unit = 0; //No subunit
	cmd.status = 0; //Clear status
	
	cd_request(device, &cmd);
	
	return cmd.status;	
}

void cd_close(uint16_t device)
{
	ioctlio_t cmd;
	tray_transport_t tray;
	
	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 12; //IOCTL output command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status
	
	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&tray; //Send tray command
	cmd.data_size = sizeof(tray);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;
	
	tray.function_code = 5; //Close tray
	
	cd_request(device, &cmd);		
}

void cd_open(uint16_t device)
{
	ioctlio_t cmd;
	tray_transport_t tray;
	
	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 12; //IOCTL output command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status
	
	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&tray; //Send tray command
	cmd.data_size = sizeof(tray);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;
	
	tray.function_code = 0; //Open tray
	
	cd_request(device, &cmd);	
}

void cd_lock(uint16_t device)
{
	ioctlio_t cmd;
	lock_t lock;
	
	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 12; //IOCTL output command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status
	
	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&lock; //Send tray command
	cmd.data_size = sizeof(lock);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;
	
	lock.function_code = 1; //Lock/unlock function code
	lock.lock_state = 1; //Lock device
	
	cd_request(device, &cmd);	
}

void cd_unlock(uint16_t device)
{
	ioctlio_t cmd;
	lock_t lock;
	
	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 12; //IOCTL output command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status
	
	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&lock; //Send tray command
	cmd.data_size = sizeof(lock);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;
	
	lock.function_code = 1; //Lock/unlock function code
	lock.lock_state = 0; //Unlock device
	
	cd_request(device, &cmd);
}

uint8_t cd_check_if_audiocd(uint16_t device)
{
	track_info_t track_info;
	ioctlio_t cmd;
	uint8_t toc_control;

	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 3; //IOCTL input command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status

	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&track_info; //Send track_info command
	cmd.data_size = sizeof(track_info);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;
	
	track_info.function_code = 11; //Audio track info
	track_info.track_no = 1; //Check for first track - this works only if whole CD contains audio tracks!

	cd_request(device, &cmd);
	
	toc_control = (track_info.track_format >> 4) & 0x0D; //Obtain control field of first track's TOC, ignore digital copy bit
	
	if((cmd.req_hdr.status & 0x8000) && (cmd.req_hdr.status & 0x0002) || (toc_control != 0x00)) //If there was 'Drive not ready' error or CD type is not Audio CD...
	{
		return 0; //...return improper CD code
	}
	
	return 1; //Return proper CD code
}

void cd_read_long(uint16_t device, uint32_t starting_sector, uint32_t sectors_to_read, void* buf) //Performs read long operation
{
	read_long_t cmd;

	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 128; //Read long command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status

	cmd.addressing_mode = 0; //Addressing mode High Sierra
	cmd.transfer_address = (uint32_t)buf;
	cmd.read_length = sectors_to_read;
	cmd.starting_sector = starting_sector;
	cmd.read_mode = 1; //Raw data read

	cd_request(device, &cmd);
}

void cd_get_disc_info(uint16_t device, uint8_t* tracks_no, uint32_t* sectors_no)
{
	disc_info_t disc_info;
	ioctlio_t cmd;

	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 3; //IOCTL input command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status

	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&disc_info; //Send disc_info command
	cmd.data_size = sizeof(disc_info);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;	
	
	disc_info.function_code = 10; //Audio disc info function code

	cd_request(device, &cmd);

	(*tracks_no) = disc_info.last_track;
	(*sectors_no) = cd_redbook_to_sector(disc_info.last_sector);
}

uint32_t cd_get_track_location(uint16_t device, uint8_t track)
{
	track_info_t track_info;
	ioctlio_t cmd;

	cmd.req_hdr.req_hdr_length = 13; //Request header size, 13 bytes
	cmd.req_hdr.command_code = 3; //IOCTL input command code
	cmd.req_hdr.sub_unit = 0; //No subunit
	cmd.req_hdr.status = 0; //Clear status

	cmd.media_descriptor = 0;
	cmd.transfer_address = (uint32_t)&track_info; //Send track info command
	cmd.data_size = sizeof(track_info);
	cmd.starting_sector = 0;
	cmd.vol_id_ptr = 0;	
	
	track_info.function_code = 11; //Audio track info function code
	track_info.track_no = track;

	cd_request(device, &cmd);

	return cd_redbook_to_sector(track_info.first_sector);
}

void cd_sectors_to_time(uint32_t sectors, uint8_t* hours, uint8_t* minutes, uint8_t* seconds)
{
	(*seconds) = (sectors/75)%60;
	(*minutes) = ((sectors/75)/60)%60;
	(*hours) = (sectors/75)/3600;
}
