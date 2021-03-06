/*
wc24emu is licensed under the MIT license:
Copyright (c) 2009 - 2010 yellowstar6

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <wc24/wc24.h>

int readonly = 0;
time_t curtime;
time_t entrytime;
nwc24dl_record dlrec;
nwc24dl_entry dlent;
char nandbase[256];
char str[256];
char emailaddr[256];

void savemail(char *mailfn, time_t lastmodtime);

void DisplayEntry()
{
	int i;
	unsigned int temp, temp2;
	int stemp;
	char id[10];
	struct tm *time;
	time_t dltime;
	printf("Found entry index %x\n", be16toh(dlent.index));

	printf("Type: ");
	switch(dlent.type)
	{
		case WC24_TYPE_MSGBOARD:
			printf("mail");
		break;

		case WC24_TYPE_TITLEDATA:
			printf("title data");
		break;

		default:
			printf("unknown %x", dlent.type);
		break;
	}
	printf("\n");
	printf("record_flags: %x\n", dlent.record_flags);
	
	temp = be32toh(dlent.flags);
	printf("flags: %x ", temp);
	for(i=0; i<32; i++)
	{
		if(temp & (1<<i))
		{
			switch(i)
			{
				case 1:
					printf("RSA_WC24PUBKMOD ");
				break;

				case 2:
					printf("RSA_DISABLE ");
				break;

				case 3:
					printf("AES_WC24PUBKMOD ");
				break;

				case 4:
					printf("IDLEONLY ");
				break;

				case 30:
					printf("MAIL_DLFREQHDR_RANGESKIP ");
				break;

				default:
					printf("Unknown flag bit %x ", i);
				break;
			}
		}
	}
	printf("\n");

	temp = be32toh(dlent.ID);
	memset(id, 0, 5);
	memcpy(id, &dlent.ID, 4);
	printf("ID: %08x(%s)\n", temp, id);

	dltime = WC24_TimestampToSeconds(be32toh(dlrec.next_dl));
	time = localtime(&dltime);
	if(dltime)printf("next_dl: %s", asctime(time));
	if(!dltime)printf("next_dl is zero this because was never downloaded.\n");

	dltime = WC24_TimestampToSeconds(be32toh(dlrec.last_modified));
	time = localtime(&dltime);
	if(dltime)printf("last_modified: %s", asctime(time));
	if(!dltime)printf("last_modified is zero, this was never downloaded or an error occurred.\n");

	temp = (unsigned int)be64toh(dlent.titleid);
	temp2 = ((unsigned int)be32toh(dlent.titleid));
	memset(id, 0, 10);
	if(temp > 0x21<<24)
	{
		memcpy(id, (void*)(((int)&dlent.titleid) + 4), 4);
	}
	else
	{
		snprintf(id, 9, "%08x", temp);
	}
	printf("titleid: %08x%08x(%08x-%s)\n", temp2, temp, temp2, id);

	temp = be16toh(dlent.group_id);
	memset(id, 0, 5);
	memcpy(id, &dlent.group_id, 2);
	printf("group_id: %04x(%s)\n", temp, id);

	printf("cnt_nextdl: %x\n", be16toh(dlent.cnt_nextdl));
	printf("total_errors: %x\n", be16toh(dlent.total_errors));
	printf("dl_freq_perday: %x\n", be16toh(dlent.dl_freq_perday));
	printf("dl_freq_days: %x\n", be16toh(dlent.dl_freq_days));
	
	stemp = (int)be32toh(dlent.error_code);
	if(stemp==0)printf("error_code is zero, either this wasn't downloaded yet or the download was successful.\n");
	if(stemp!=0)
	{
		printf("error_code: %d ", stemp);
		if(stemp==WC24_EINVALVFF)printf("EINVALVFF");
		if(stemp==WC24_EVFFPATH)printf("EVFFPATH");
		if(stemp==WC24_ESIGFAIL)printf("ESIGFAIL");
		if(stemp==WC24_EHTTP304)printf("EHTTP304");
		if((stemp + 117000) < 0 && (stemp + 117000) > -1000)printf("HTTP %d", (abs(stemp) - 117000));
		printf("\n");
	}

	printf("unk24SubTask: %x\n", dlent.unk24SubTask);
	printf("unk25SubTask: %x\n", dlent.unk25SubTask);
	printf("subTaskFlags: %x\n", dlent.subTaskFlags);
	printf("unk27SubTask: %x\n", dlent.unk27SubTask);
	temp = be32toh(dlent.subTaskBitmask);
	printf("subTaskBitmask: %x\n", temp);

	printf("unk2cSubTask: %x\n",  be16toh(dlent.unk2cSubTask));
	printf("unk2eSubTask: %x\n",  be16toh(dlent.unk2eSubTask));

	dltime = WC24_TimestampToSeconds(be32toh(dlent.dl_timestamp));
	time = localtime(&dltime);
	if(dltime)printf("dl_timestamp: %s", asctime(time));
	if(!dltime)printf("dl_timestamp is zero this because was never downloaded or an error occurred.\n");

	for(i=0; i<32; i++)
	{
		if(temp & (1<<i))
		{
			dltime = WC24_TimestampToSeconds(be32toh(dlent.subTaskTimestamps[i]));
			time = localtime(&dltime);
			if(dltime)printf("subTaskTimestamps[%x]: %s", i, asctime(time));
			if(dltime==0)printf("subTaskTimestamps[%x] is zero since this subTask was never downloaded or an error occurred.\n", i);
		}
	}

	memset(str, 0, 256);
	strncpy(str, dlent.url, WC24_URLMAXSZ);
	printf("URL: %s\n", str);

	if(dlent.type!=WC24_TYPE_MSGBOARD)
	{
		memset(str, 0, 256);
		strncpy(str, dlent.filename, WC24_VFFPATHMAXSZ);
		printf("VFF path: %s\n", str);
	}

	printf("NHTTP_RootCA: %x\n", dlent.NHTTP_RootCA);

	printf("\n");
}

void ProcessEntry()
{
	struct tm *entrytmtime;
	char *filename;
	char *temp;
	int i;
	struct stat mailstats;
	unsigned int flags;
	int retval;
	curtime = time(NULL);
	entrytime = WC24_TimestampToSeconds(be32toh(dlrec.last_modified));
	if(entrytime)entrytmtime = gmtime(&entrytime);
	printf("Found next entry to dl.\n");
	DisplayEntry();

	for(i=strlen(dlent.url); i>0 && dlent.url[i]!='/'; i--);
	i++;
	filename = &dlent.url[i];

	if(entrytime)
	{
		memset(str, 0, 256);
		snprintf(str, 255, "touch --date=\"%sUTC\" %s", asctime(entrytmtime), filename);
		temp = strchr(str, 0x0a);
		if(temp)*temp = ' ';
		printf("%s\n", str);
		system(str);
	}
	else
	{
		unlink(filename);
	}

	flags = be32toh(dlent.flags);
	memset(str, 0, 256);
	if((flags & (WC24_FLAGS_RSA_WC24PUBKMOD | WC24_FLAGS_AES_WC24PUBKMOD))==0)
	{
		snprintf(str, 255, "getwiimsg 049 en wc24msgboardkey.bin --cache %s", dlent.url);
	}
	else
	{
		snprintf(str, 255, "getwiimsg 049 en %s/title/%08x/%08x/data/wc24pubk.mod --cache %s", nandbase, (u32)(be64toh(dlent.titleid) >> 32), (u32)be64toh(dlent.titleid), dlent.url);
	}
	printf("%s\n", str);
	retval = WEXITSTATUS(system(str));
	if(retval!=0 && retval!=3 && retval!=34 && retval!=44 && retval!=11)
	{
		printf("HTTP or wc24decrypt/getwiimsg fail.\n");
		return;
	}

	if(retval!=3 && retval!=34 && retval!=44 && retval!=11)
	{
		stat(filename, &mailstats);
		savemail("mail0.eml", mailstats.st_mtime);
		printf("New mail downloaded.\n");
	}

	if(retval==0 || retval==3)dlrec.last_modified = htobe32(WC24_SecondsToTimestamp(mailstats.st_mtime));
	dlrec.next_dl = htobe32(WC24_SecondsToTimestamp(curtime) + be16toh(dlent.dl_freq_perday));
	//dlent.dl_timestamp = htobe32(WC24_SecondsToTimestamp(curtime));

	WC24_WriteRecord(be16toh(dlent.index), &dlrec);
	WC24_WriteEntry(be16toh(dlent.index), &dlent);
}

void savemail(char *mailfn, time_t lastmodtime)
{
	struct stat mailstats;
	struct tm *modtime;
	unsigned char *buf;
	FILE *fmail;
	char *staticstr;
	int fromlen = 0;
	char *fromstr, *home;
	char curdir[256];
	stat(mailfn, &mailstats);
	modtime = gmtime(&lastmodtime);

	memset(str, 0, 256);
	snprintf(str, 255, "recvmail/%08x", dlent.ID);
	mkdir("recvmail", 0777);
	mkdir(str, 0777);

	memset(str, 0, 256);
	snprintf(str, 255, "recvmail/%08x/%d-%d-%d_%d-%d.eml", dlent.ID, modtime->tm_mon+1, modtime->tm_mday, modtime->tm_yday, modtime->tm_hour, modtime->tm_min);

	buf = (unsigned char*)malloc(mailstats.st_size);
	memset(buf, 0, mailstats.st_size);
	fmail = fopen(mailfn, "rb");
	fread(buf, 1, mailstats.st_size, fmail);
	fclose(fmail);

	fmail = fopen(str, "wb");
	fwrite(buf, 1, mailstats.st_size, fmail);
	fclose(fmail);

	if(strlen(emailaddr)>0)
	{
		fmail = fopen("tmpmail", "wb");
		fwrite(buf, 1, mailstats.st_size, fmail);
		fclose(fmail);

		memset(str, 0, 256);
		snprintf(str, 255, "sendmail %s < %s", emailaddr, "tmpmail");
		printf("%s\n", str);
		system(str);
	}

	home = getenv("HOME");
	if(home)
	{
		memset(str, 0, 256);
		snprintf(str, 255, "%s/Desktop/new_wc24recvmail", home);
		memset(curdir, 0, 256);
		getcwd(curdir, 255);
		strncat(curdir, "/recvmail", 255);
		symlink(curdir, str);
	}
}

void DlQuene()
{
	int i;
	curtime = time(NULL);

	for(i=0; i<be16toh(NWC24DL_Header->max_entries); i++)
	{
		WC24_ReadRecord(i, &dlrec);
		WC24_ReadEntry(i, &dlent);
		if(dlrec.ID==0 && dlent.type==WC24_TYPE_EMPTY)continue;

		if(readonly)
		{
			DisplayEntry();
		}
		else
		{

			if(dlent.type!=WC24_TYPE_MSGBOARD)continue;//Only process mail entries for now.

			entrytime = WC24_TimestampToSeconds(be32toh(dlrec.next_dl));
			if(curtime<entrytime)continue;

			ProcessEntry();
			break;
		}
	}
}

int main(int argc, char **argv)
{
	int retval;
	int argi;
	printf("wc24emu v1.0 by yellowstar6\n");
	if(argc<2)
	{
		printf("App to emulate WC24.\n");
		printf("Usage:\n");
		printf("wc24emu <nand_dump_path> <options>\n");
		printf("To only read and display nwc24dl.bin entries, specify a direct path to a nwc24dl.bin.\nThe displayed timestamps use localtime.\n");
		printf("Options:\n");
		printf("--read: Only read and display entries, don't emulate.(Default when direct nwc24dl.bin path is used.)\n");
		printf("--email=<email address> Send raw downloaded email to this email address with sendmail.\n");
		return 0;
	}

	memset(emailaddr, 0, 256);
	if(argc>2)
	{
		for(argi=2; argi<argc; argi++)
		{
			if(strcmp(argv[argi], "--parse")==0)readonly = 1;
			if(strncmp(argv[argi], "--email=", 8)==0)strncpy(emailaddr, &argv[argi][8], 255);
		}
	}
	if(strstr(argv[1], ".bin"))readonly = 1;

	memset(nandbase, 0, 256);
	strncpy(nandbase, argv[1], 255);
	retval = WC24_Init(argv[1]);
	if(retval<0)
	{
		printf("WC24_Init failed %d\n", retval);
		return 0;
	}

	DlQuene();

	WC24_Shutdown();
	
	return 0;
}

