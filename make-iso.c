// make-iso.c
// Erstellt eine ISO-Datei von CD
// 2020-01-08 KG Erstellt

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>

// Programmabbruch
BOOL done = FALSE;

// Pfad und Name der ISO-Datei
char isoFileName[MAX_PATH] = "cdrom.iso";

// HAndle des Laufwerks
HANDLE isoFileHandle = INVALID_HANDLE_VALUE;

// CD-Laufwerk
char drive[5] = "d:\\";

// Geraetename des CD-Laufwerks
char deviceName[10] = "";

// HAndle des Laufwerks
HANDLE deviceHandle = INVALID_HANDLE_VALUE;

// Kopierpuffer
unsigned char buffer[65536] = { 0 };

// gelesene Bytes
DWORD bytesRead = 0;

// geschriebene Bytes
DWORD bytesWritten = 0;

// Gesamt geschriebene Bytes
long long totalBytesWritten = 0;

// Prozent erledigt
int newpercent = 0;
int percent = 0;

// Ausgabepuffer
char progress[128] = "";

// Erzeugt einen String in der Form 1KB, 1MB, 1GB usw. fuer die
// angegebene Dateigroesse
char *fileSizeToString(long long filesize)
{
	static char buffer[50] = "0 Byte";
	char *suffix[] = { "B", "KB", "MB", "GB", "TB" };
	int i = 0;

	// Sonderfall: 0
	if (filesize == 0)
		return buffer;

	for (i = 0; i < 4; i++)
	{
		if (filesize < 1024)
		{
			sprintf(buffer, "%lld %s", filesize, suffix[i]);
			return buffer;
		}
		filesize /= 1024;
	}

	// Groessenangabe in TB
	sprintf(buffer, "%lld %s", filesize, suffix[i]);
	return buffer;
}

// Sucht das erste CDROM-Laufwerk und liefert den Laufwerksbuchstaben zurueck
// oder #, wenn kein CDROM-Laufwerk gefunden wurde
char findCDROM()
{
	int i = 0;
	char root[10] = "";
	UINT driveType = 0;

	// Laufwerksbuchstaben D..Z testen
	for (i = 68; i <= 90; i++)
	{
		sprintf(root, "%c:\\", i);
		driveType = GetDriveType(root);
		if (driveType == DRIVE_CDROM)
			return i;
	}
	return '#';
}


BOOL ConsoleCtrlHandlerProc(DWORD dwCtrlType)
{
	done = TRUE;
	return TRUE;
}

// Zeigt Informationen zum Programm an
void usage()
{
	char *usage =
		"make-iso 2020-01-08\n"
		"Erstellt eine ISO-Datei von CD\n"
		"Verwendung: make-iso filename [path]\n"
		"filename Dateiname der ISO-Datei\n"
		"path     (optional) Ausgabepfad für die ISO-Datei. Ohne Angabe wird das aktuelle\n"
		"         Verzeichnis verwendet\n"
		"Die zu kopierende CD wird im ersten vorhandenen CDROM-Laufwerk erwartet.\n";
		printf("%s\n", usage);
		exit(1);
}


// main
void main(int argc, char *argv[])
{
	int i = 0;
	DWORD lpSectorsPerCluster = 0;
	DWORD lpBytesPerSector = 0;
	DWORD lpNumberOfFreeClusters = 0;
	DWORD lpTotalNumberOfClusters = 0;
	long long totalBytes= 0;
	char cdrom = '#';
	int len = 0;

	// das erste CDROM-Laufwerk suchen
	cdrom = findCDROM();
	if (cdrom == '#')
	{
		printf("Kein CDROM-Laufwerk gefunden!\n");
		return;
	}

	// root des CDROM-Laufwerks
	sprintf(drive, "%c:\\", cdrom);

	// Geraetename des CDROM-Laufwerks
	sprintf(deviceName, "\\\\.\\%c:", cdrom);

	// Argumente pruefen
	if (argc < 2)
		usage();

	// aktuelles Verzeichnis fuer die ISO-Datei verwenden
	_getcwd(isoFileName, sizeof(isoFileName));

	// falls ein Pfad angegeben ist, wird dieser verwendet
	if (argc > 2)
	{
		strcpy(isoFileName, argv[2]);
		// ggf. abschliessenden BACKSLASH loeschen
		len = strlen(isoFileName);
		if (isoFileName[len] == '\\')
			isoFileName[len] = 0;
	}

	// Dateiname anfuegen
	strcat(isoFileName, "\\");
	strcat(isoFileName, argv[1]);

	// "iso" anfuegen, falls keine Erweiterung angegeben ist
	if (stricmp(isoFileName - 4, ".iso") != 0)
		strcat(isoFileName, ".iso");

	// Groesse der CDROM-Daten 
	GetDiskFreeSpace(drive, &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters);
	totalBytes = lpSectorsPerCluster * lpBytesPerSector * lpTotalNumberOfClusters;

	// Laufwerk oeffnen
	printf("Laufwerk oeffnen: %c:\n", drive[0]);
	deviceHandle = CreateFile(deviceName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (deviceHandle == INVALID_HANDLE_VALUE)
	{
		printf("Laufwerk kann nicht gelesen werden\n");
		return;
	}

	// ISO-Datei anlegen
	printf("ISO-Datei anlegen\n");
	isoFileHandle = CreateFile(isoFileName, GENERIC_READ|GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (isoFileHandle == INVALID_HANDLE_VALUE)
	{
		CloseHandle(deviceHandle);
		printf("ISO-Datei konnte nicht angelegt werden\n");
		return;
	}

	// Programmabbruch mit CTRL+C ermoeglichen
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandlerProc, TRUE);

	printf("Groesse: %s\n", fileSizeToString(totalBytes));
	printf("Kopieren...\r");
	while (!done)
	{
		ReadFile(deviceHandle, buffer, sizeof(buffer), &bytesRead, NULL);
		if (bytesRead == 0)
			break;
		WriteFile(isoFileHandle, buffer, bytesRead, &bytesWritten, NULL);
		totalBytesWritten += bytesWritten;
		newpercent =  totalBytesWritten * 100 / totalBytes;
		if (newpercent != percent)
		{
			percent = newpercent;
			sprintf(progress, "Kopieren... (%ld%%)", percent);
			printf("%s\r", progress);
		}
	}
	printf("\n");

	// Laufwerk schliessen
	printf("Laufwerk schliessen\n");
	CloseHandle(deviceHandle);

	// Zieldatei schliessen
	printf("ISO-Datei schliessen\n");
	CloseHandle(isoFileHandle);

	// Ende
	printf("OK\n");
}
