libwc24 v1.1 by yellowstar6
This library allows Wii homebrew to use WiiConnect24. You need at least HBC 1.0.7 to use libwc24 when accessing HBC data dir for creating and reading wc24dl.vff which contains the downloaded title data WC24 content. In this directory tree is libwc24 and wc24app. The former is the actual library, and the latter is the WC24 test app. Each time libwc24 is compiled successfully, the header files and libwc24.a will be copied to $DEVKITPRO/libogc/include/wc24 and $DEVKITPRO/libogc/lib libwc24 can be also be installed by running this in the libwc24 directory: make install Optionally you can install libwc24 by copying the headers and library to the above libogc directories.
For more WC24 info, see the Wiibrew pages: http://wiibrew.org/wiki/WiiConnect24 http://wiibrew.org/wiki//shared2/wc24/nwc24dl.bin http://wiibrew.org/wiki/WiiConnect24/WC24_Content
The source is included with the download tarball, and is also available on SVN at: http://code.google.com/p/wmb-asm/source/checkout

These are the Internet server URLs that wc24app can install: http://iwconfig.net/~yellows8/wc24test.php http://iwconfig.net/~yellows8/wc24testmail.php
The former is the title data URL, the latter is the message board mail URL. To retrieve the php scripts source, add "?cid=source" to the URL.

Hackmii and Wiibrew mail URLs:
http://wc24.hackmii.com/index.php
http://iwconfig.net/~yellows8/wiibrew/news/index.php
http://iwconfig.net/~yellows8/wiibrew/releases/index.php

The source to these scripts like the wc24test scripts can be retrieved by adding "?cid=source" to the URLs.

Refrence the Wiibrew WC24_Content page for info on the required WC24 header when content length is less than 0x140 bytes. Also see the above URLs installed by wc24app for template WC24 content. The title data file is supposed to only contain the payload text when written to wc24dl.vff, however the whole file is written there. The cause of this is currently unknown, but the all-zero 0x140 bytes-long header isn't needed when the actual content length is larger than 0x140 bytes.

Features:
VFF files can be read and written, with LFN support.(Long filenames)
The download frequency is specified in minutes, see: http://wiibrew.org/wiki//shared2/wc24/nwc24dl.bin
RSA signature verification for content can be disabled.
Content can optionally be downloaded only in idle/"standby" mode with a flag bit.
Download content to a file in wc24dl.vff under HBC data dir.
Download message board mail "annoucements", like Nintendo's system update messages and channel messages.
Download content immediately. KD_SaveMail can be executed, to save mail immediately without being overwritten by other  mail downloaded immediately.
Get and set the KD UTC time. KD automatically gets the current UTC time via a time server.
With wc24app, a WC24 title boot mail entry can be installed. You need to use the WC24 title boot flag enable option in wc24app, to enable and use WC24 title booting. For the initial entry install, download immediately, then shutdown via wiimote when wc24app displays "Done", or with any official software. Once KD processes the mail in idle mode, KD would write the NANDBOOTINFO attachment to NAND, then wakeup the Wii so sysmenu boots the title specified in NANDBOOTINFO. The boot mail entry NANDBOOTINFO attachment causes sysmenu to boot HBC 1.0.7 once the the mail is processed etc. WC24 title booting can be used to fix some bricks, see: http://wiibrew.org/wiki/Brick#WC24_title_booting
Read/write nwc24msg config files, and read/write nwc24fl.bin. Wc24recv.mbx and wc24send.mbx can be mounted for reading/writing received mail or mail pending to be sent. Code to move mail elsewhere from wc24recv.mbx or to send mail with via wc24send.mbx and the WC24 mail engine can't be written until the mail ctl files are reverse engineered.
NAND title wc24boottitle can be used with WC24 title booting to boot homebrew and game discs with WC24, see the wc24boottitle README.
Wc24app adds anonymous tracking info to the installed entries' URLs for logging server access. This only contains the consoleID, which is unique to each Wii. This is used for both the test entries and the hackmii+wiibrew entries.
Added VFF creation param delvff, to specify whether to automatically delete the VFF when it exists or not. Wc24app passes 1 for delvff so when creating wc24dl.vff from wc24app wc24dl.vff is always deleted before creation.
Added wiibrew news, releases, and hackmii feed mail. New wiibrew news items, new releases, and new(or updated) hackmii posts would be downloaded when the entries are installed with wc24app. After the initial WC24 mail download, the server scripts determine what items to send in the mail if any by the items' dates and the WC24 HTTP Last-modified header.

Known bugs:
Bugs not listed here can be reported at: http://code.google.com/p/wmb-asm/issues/list

1) Reading/writing FAT16 VFFs doesn't work, FAT32 VFFs weren't tested.

Changelog:
v1.1:
The libwc24 v1.0 source tarball(not the pre-compiled library and dol) had a ISFS heap buffer overflow bug. Only 0x10 bytes for the size of a record was allocated for reading and writing records, but ISFS always reads/writes at least 0x20 bytes. This was fixed in libwc24 v1.1.
Added support for KD_SetNextWakeup and KD_GetTimeTriggers.
Added support for KD_Download flag sync UTC time.
KD_Open and KD_Close are now called by WC24_Init and WC24_Shutdown.
Fixed typo for the WC24_EHTTP304 define value. In libwc24 v1.0 this caused wc24app to skip VFF reading since wc24app/libwc24 didn't recognize the error from ent.error_code.
VFF files are now read and written with libff, with long filenames support. VFF files can be mounted, and can be used with a devoptab VFF device.
Added support for HBC 1.0.7 titleID. Wc24app uses the default titleID from libwc24, which is the titleID of the current title.
Added feature in wc24app to install a WC24 title boot mail entry, see features section.
Added libwc24 support for KD_SaveMail. Wc24app now calls this after downloading each mail entry, so that the following mail downloaded immediately doesn't overwrite the unsaved dlcnt.bin mail content.
Removed IOS reload code from wc24app, removed HBC identification code from libwc24 and wc24app.
Added support for KD_SaveMail, this is now called each time wc24app downloads mail immediately.
Added wc24mail code, for handling reading/writing nwc24msg config files, handling the msg board friends list, mounting the wc24recv.mbx/wc24send.mbx files, etc. See the features section.
Added app wc24boottitle, see features section.
Wc24app now adds anonymous tracking info to the installed entries' URLs for logging server access by WC24 from the Wiis with the new URLs installed. See the features section.
Added VFF creation param delvff, to specify whether to automatically delete the VFF when it exists or not. Wc24app passes 1 for delvff so when creating wc24dl.vff from wc24app wc24dl.vff is always deleted before creation.
Switched KD time functions input to u64 from struct kd_timebuf.
Added cnt_nextdl parameter to WC24_CreateRecord, see wc24.h for details.
Added wiibrew and hackmii mail entries installation, see features section.

v1.0:
Initial release.

Credits:
Bushing for VFF FAT size code: http://wiibrew.org/wiki/VFF
Elm Chan FatFs. libff ff.c, ff.h, option/syscall.c, and option/ccsbcs.c were modified to redefine f_* functions as fvff_* functions.
Mha for hosting the Wiibrew mail server scripts, and for hosting the content for wc24app test entries
Marcan for hosting the Hackmii mail server scripts

