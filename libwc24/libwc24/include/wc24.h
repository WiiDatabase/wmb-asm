/*
libwc24 is licensed under the MIT license:
Copyright (c) 2009 and 2010 yellowstar6

Permission is hereby granted, free of charge, to any person obtaining a copy of this
software and associated documentation files (the Software), to deal in the Software
without restriction, including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED AS IS, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef _H_WC24
#define _H_WC24

#include "types.h"

#include "kd.h"
#include "vff.h"
#include "wc24mail.h"

#ifdef __cplusplus
   extern "C" {
#endif

#ifndef BIT
#define BIT(n) 1<<n
#endif

#define WC24_TYPE_UNK 1
#define WC24_TYPE_MSGBOARD 2//E-Mail downloaded to msg board.(Plus MIME data.)
#define WC24_TYPE_TITLEDATA 3//Content is is downloaded to title's data dir.
#define WC24_TYPE_EMPTY 0xff

#define WC24_FLAGS_RSA_WC24PUBKMOD BIT(1)//Use RSA public key from wc24pubk.mod in title's data dir instead of default key, if content is encrypted.
#define WC24_FLAGS_RSA_VERIFY_DISABLE BIT(2)//Skip RSA signature verification.
#define WC24_FLAGS_AES_WC24PUBKMOD BIT(3)//Use AES key from wc24pubk.mod in title's data dir instead of default key.
#define WC24_FLAGS_IDLEONLY BIT(4)//Only download this entry in idle mode.
#define WC24_FLAGS_MAIL_DLFREQHDR_RANGESKIP BIT(30)//For mail entries, when the X-Wii-Download-Interval header in mail is used when nwc24dl_header.reserved_mailnum <= ent.index, skip the range check on the header value frequency. If this block fails, the header parser function returns -4: if(input_freq - 180 <= 9900).
#define WC24_FLAGS_HB WC24_FLAGS_RSA_VERIFY_DISABLE//Default flags for homebrew, which is the RSA verify disable flag.

#define WC24_SUBTASK_FLAGS_ENABLE BIT(0)//Use subTasks

#define WC24_RECORD_FLAGS_DEFAULT 0xc0//Unknown, varies, this is from HATE.

#define LIBWC24_EINIT -0x2001//From WC24_Init, this means WC24 was already initalized, from WC24_Shutdown, this means WC24 was already shutdown, or wasn't initalized. From other functions, this means WC24 wasn't initalized.
#define LIBWC24_ENOENT -0x2002//From WC24 find functions, meaning no entry/record found.

//The below errors are from nwc24dl_entry error_code written to by KD.
//This error code list is also available at: http://wiibrew.org/wiki/WiiConnect24#WC24_Errors
//Errors in the -1072xx range might be errors from KD_Download, where xx is the retval of KD_Download.

#define WC24_EINVALVFF -107243//Invalid wc24dl.vff.
#define WC24_EVFFPATH -107216//The VFF subdirectories which the file path is under doesn't exist.
#define WC24_ESIGFAIL -107245//RSA signature verification failed. Or KD_CreateRSA failed.
#define WC24_EHTTP304 -107304//HTTP 304.
#define WC24_EHTTP404 -117404//HTTP 404. Errors in the -117xxx range seem to be HTTP errors, where xxx is the HTTP error/status code.
// Errors with 5-digits are usually general Wifi/Internet errors, which can be looked up with Nintendo's tool: http://www.nintendo.com/consumer/systems/wii/en_na/errors/index.jsp

#define WC24_URLMAXSZ 0xec
#define WC24_VFFPATHMAXSZ 0x40

typedef struct _nwc24dl_header
{
	u32 magic;//"WcDl" 0x5763446c
	u32 unk4;//0x1
	u32 filler[2];
	u16 unk10;//0x20
	u16 reserved_mailnum;//Max num of reserved mail entries, starting at the first entry. 0x8
	u16 max_entries;//Max num of entries/records that can be stored in nwc24dl.bin, always 0x78.
	u8 reserved[0x6a];
} __attribute__((packed)) nwc24dl_header;

typedef struct _nwc24dl_record
{
	u32 ID;//Titleid low, except for sysmenu. Zero for empty.
	u32 next_dl;//Timestamp for next time to download.
	u32 last_modified;//Timestamp for server date last modified. Zero when download failed, with entry error_code less than zero.
	u8  record_flags;//?
	u8  filler[3];
} __attribute__((packed)) nwc24dl_record;

typedef struct _nwc24dl_entry
{
	u16 index;//Zero-based.
	u8 type;
	u8 record_flags;//?
	u32 flags;
	u32 ID;//Usually titleid low, but it doesn't need to match a NAND titleid like announcement mail.
	u64 titleid;
	u16 group_id;
	u16 unk16;
	u16 cnt_nextdl;//Number of downloads for this entry until WC24 starts downloading the next entry with the same ID? It's unknown what happens when this number is reached with only one entry for the ID. This is decreased by KD everytime it downloads the entry, it's unknown how KD resets this value once it's zero.
	u16 total_errors;//Zero for no error for the last dl. Unknown for subTasks.
	u16 dl_freq_perday;//Download frequency in minutes per day.
	u16 dl_freq_days;//Dl frequency in minutes, for when the next day of downloading begins.(Starting at midnight of dl day. Usually 0x5a0 for daily.)
	s32 error_code;//Zero for no error, negative WC24 error code otherwise.
	u8 unk24SubTask;
	u8 unk25SubTask;//Must be non zero to use subTasks?
	u8 subTaskFlags;
	u8 unk27SubTask;
	u32 subTaskBitmask;//Bitmask subTask ID enable. Only subTasks with IDs/numbers with matching set bits are used.
	u16 unk2cSubTask;
	u16 unk2eSubTask;//0x5a0 sometimes for entries with subTasks? Might be related to dl time?
	u32 dl_timestamp;//Last dl timestamp. Zero when download failed, with error_code less than zero, except for error  WC24_EHTTP304. This timestamp is the last dl timestamp for when the download succeeded without any error, with HTTP 200 OK.
	u32 subTaskTimestamps[32];//Timestamps of last dl time for each subTask?
	char url[WC24_URLMAXSZ];
	char filename[WC24_VFFPATHMAXSZ];//Filename inside wc24dl.vff, without a leading root directory slash. This path can probably can be a path with sub-directories.
	u8 unk1e0[0x1d];
	u8 NHTTP_RootCA;//Usually zero.
	u16 unk1fe;
} __attribute__((packed)) nwc24dl_entry;

extern nwc24dl_header *NWC24DL_Header;

s32 WC24_Init(
#ifndef HW_RVL
char *basedir
#endif
);
s32 WC24_Shutdown();
s32 WC24_OpenNWC4DLBin();//Call this and WC24_CloseNWC4DLBin when using read/write functions to keep nwc24dl.bin opened between r/w functions, instead of opening/closing it for each r/w function.
s32 WC24_CloseNWC4DLBin();
s32 WC24_ReadRecord(u32 index, nwc24dl_record *rec);
s32 WC24_WriteRecord(u32 index, nwc24dl_record *rec);
s32 WC24_ReadEntry(u32 index, nwc24dl_entry *ent);
s32 WC24_WriteEntry(u32 index, nwc24dl_entry *ent);
s32 WC24_FindRecord(u32 id, nwc24dl_record *rec);//When find functions are successful, they return the index of the record/entry.
s32 WC24_FindEntry(u32 id, char *url, nwc24dl_entry *ent, int cmpwith_strstr);//When cmpwith_strstr is zero, use strncmp to compare the URLs, otherwise use strstr.
s32 WC24_FindEmptyRecord(nwc24dl_record *rec);
s32 WC24_FindEmptyEntry(nwc24dl_entry *ent);
s32 WC24_CreateRecord(nwc24dl_record *rec, nwc24dl_entry *ent, u32 id, u64 titleid, u16 group_id, u8 type, u8 record_flags, u32 flags, u16 dl_freq_perday, u16 dl_freq_days, u16 cnt_nextdl, char *url, char *filename);//Uses an old entry and record with same ID and URL if it exists, otherwise an empty record/entry is used. Returns index. The dl_freq parameters are in minutes, see: http://wiibrew.org/wiki//shared2/wc24/nwc24dl.bin When id or titleid is zero, the current titleid is used for those fields. The cnt_nextdl field seems to be the number of downloads until WC24 starts downloading the next entry with the same ID, this can be zero to use the default values.
s32 WC24_DeleteRecord(u32 index);//Deletes a record and entry.
s32 WC24_MntCreateDataDirVFF(char *path, u32 filesize, int delvff);//Create a VFF when filesize is not zero, mount it otherwise. Path is relative to current title data dir without an leading '/'. When mounting the mntname is the path filename. See vff.h for the delvff param.
s32 WC24_CreateWC24DlVFF(u32 filesize, int delvff);//Create wc24dl.vff in the current title's data dir. See vff.h for the delvff param.
s32 WC24_MountWC24DlVFF();//Mounts wc24dl.vff in the current title's data dir, as mntname wc24dl.vff.
s32 WC24_CreateWC24ScrVFF(u32 filesize, int delvff);//Create wc24scr.vff in the current title's data dir, used to store CHANS scripts that process certain mail meant for this title specified by the X-Wii-AppID mail header. See vff.h for the delvff param.
s32 WC24_MountWC24ScrVFF();//Mounts wc24scr.vff in the current title's data dir, as mntname wc24scr.vff.
time_t WC24_TimestampToSeconds(u32 timestamp);
u32 WC24_SecondsToTimestamp(time_t timestamp);
void WC24_SetTitleID(u64 titleid);//Sets the titleid to use for WC24_CreateWC24DlVFF and WC24_MountWC24DlVFF. WC24_Init uses ES_GetTitleID for the titleid.
u64 WC24_GetTitleID();//Get the titleid used for WC24_CreateWC24DlVFF and WC24_MountWC24DlVFF.

#ifdef __cplusplus
   }
#endif

#endif
