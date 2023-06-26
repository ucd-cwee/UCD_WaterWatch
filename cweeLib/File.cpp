#include "precompiled.h"
#pragma hdrstop

#ifdef MAX_PRINT_MSG
#undef MAX_PRINT_MSG
#endif
#define	MAX_PRINT_MSG		4096

/*
=================
FS_WriteFloatString
=================
*/
int FS_WriteFloatString(char* buf, const char* fmt, va_list argPtr) {
	long i;
	unsigned long u;
	double f;
	char* str;
	int index;
	cweeStr tmp, format;

	index = 0;

	while (*fmt) {
		switch (*fmt) {
		case '%':
			format = "";
			format += *fmt++;
			while ((*fmt >= '0' && *fmt <= '9') ||
				*fmt == '.' || *fmt == '-' || *fmt == '+' || *fmt == '#') {
				format += *fmt++;
			}
			format += *fmt;
			switch (*fmt) {
			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				f = va_arg(argPtr, double);
				if (format.Length() <= 2) {
					// high precision floating point number without trailing zeros
					sprintf(tmp, "%1.10f", f);
					tmp.StripTrailing('0');
					tmp.StripTrailing('.');
					index += sprintf(buf + index, "%s", tmp.c_str());
				}
				else {
					index += sprintf(buf + index, format.c_str(), f);
				}
				break;
			case 'd':
			case 'i':
				i = va_arg(argPtr, long);
				index += sprintf(buf + index, format.c_str(), i);
				break;
			case 'u':
				u = va_arg(argPtr, unsigned long);
				index += sprintf(buf + index, format.c_str(), u);
				break;
			case 'o':
				u = va_arg(argPtr, unsigned long);
				index += sprintf(buf + index, format.c_str(), u);
				break;
			case 'x':
				u = va_arg(argPtr, unsigned long);
				index += sprintf(buf + index, format.c_str(), u);
				break;
			case 'X':
				u = va_arg(argPtr, unsigned long);
				index += sprintf(buf + index, format.c_str(), u);
				break;
			case 'c':
				i = va_arg(argPtr, long);
				index += sprintf(buf + index, format.c_str(), (char)i);
				break;
			case 's':
				str = va_arg(argPtr, char*);
				index += sprintf(buf + index, format.c_str(), str);
				break;
			case '%':
				index += sprintf(buf + index, format.c_str());
				break;
			default:
				break;
			}
			fmt++;
			break;
		case '\\':
			fmt++;
			switch (*fmt) {
			case 't':
				index += sprintf(buf + index, "\t");
				break;
			case 'v':
				index += sprintf(buf + index, "\v");
				break;
			case 'n':
				index += sprintf(buf + index, "\n");
				break;
			case '\\':
				index += sprintf(buf + index, "\\");
				break;
			default:
				break;
			}
			fmt++;
			break;
		default:
			index += sprintf(buf + index, "%c", *fmt);
			fmt++;
			break;
		}
	}

	return index;
}

/*
=================================================================================

cweeFile

=================================================================================
*/

/*
=================
cweeFile::GetName
=================
*/
const char* cweeFile::GetName(void) {
	return "";
}

/*
=================
cweeFile::GetFullPath
=================
*/
const char* cweeFile::GetFullPath(void) {
	return "";
}

/*
=================
cweeFile::Read
=================
*/
int cweeFile::Read(void* buffer, int len) {
	return 0;
}

/*
=================
cweeFile::Write
=================
*/
int cweeFile::Write(const void* buffer, int len) {
	return 0;
}

/*
=================
cweeFile::Length
=================
*/
int cweeFile::Length(void) {
	return 0;
}

/*
=================
cweeFile::Tell
=================
*/
int cweeFile::Tell(void) {
	return 0;
}

/*
=================
cweeFile::ForceFlush
=================
*/
void cweeFile::ForceFlush(void) {
}

/*
=================
cweeFile::Flush
=================
*/
void cweeFile::Flush(void) {
}

/*
=================
cweeFile::Seek
=================
*/
int cweeFile::Seek(long offset, fsOrigin_t origin) {
	return -1;
}

/*
=================
cweeFile::Rewind
=================
*/
void cweeFile::Rewind(void) {
	Seek(0, FS_SEEK_SET);
}

/*
=================
cweeFile::Printf
=================
*/
int cweeFile::Printf(const char* fmt, ...) {
	char buf[MAX_PRINT_MSG];
	int length;
	va_list argptr;

	va_start(argptr, fmt);
	length = cweeStr::vsnPrintf(buf, MAX_PRINT_MSG - 1, fmt, argptr);
	va_end(argptr);

	// so notepad formats the lines correctly
	cweeStr	work(buf);
	work.Replace("\n", "\r\n");

	return Write(work.c_str(), work.Length());
}

/*
=================
cweeFile::VPrintf
=================
*/
int cweeFile::VPrintf(const char* fmt, va_list args) {
	char buf[MAX_PRINT_MSG];
	int length;

	length = cweeStr::vsnPrintf(buf, MAX_PRINT_MSG - 1, fmt, args);
	return Write(buf, length);
}

/*
=================
cweeFile::WriteFloatString
=================
*/
int cweeFile::WriteFloatString(const char* fmt, ...) {
	char buf[MAX_PRINT_MSG];
	int len;
	va_list argPtr;

	va_start(argPtr, fmt);
	len = FS_WriteFloatString(buf, fmt, argPtr);
	va_end(argPtr);

	return Write(buf, len);
}

/*
 =================
 cweeFile::ReadInt
 =================
 */
int cweeFile::ReadInt(int& value) {
	int result = Read(&value, sizeof(value));
	value = LittleLong(value);
	return result;
}

/*
 =================
 cweeFile::ReadUnsignedInt
 =================
 */
int cweeFile::ReadUnsignedInt(unsigned int& value) {
	int result = Read(&value, sizeof(value));
	value = LittleLong(value);
	return result;
}

/*
 =================
 cweeFile::ReadShort
 =================
 */
int cweeFile::ReadShort(short& value) {
	int result = Read(&value, sizeof(value));
	value = LittleShort(value);
	return result;
}

/*
 =================
 cweeFile::ReadUnsignedShort
 =================
 */
int cweeFile::ReadUnsignedShort(unsigned short& value) {
	int result = Read(&value, sizeof(value));
	value = LittleShort(value);
	return result;
}

/*
 =================
 cweeFile::ReadChar
 =================
 */
int cweeFile::ReadChar(char& value) {
	return Read(&value, sizeof(value));
}

/*
 =================
 cweeFile::ReadUnsignedChar
 =================
 */
int cweeFile::ReadUnsignedChar(unsigned char& value) {
	return Read(&value, sizeof(value));
}

/*
 =================
 cweeFile::ReadFloat
 =================
 */
int cweeFile::ReadFloat(float& value) {
	int result = Read(&value, sizeof(value));
	value = LittleFloat(value);
	return result;
}

/*
 =================
 cweeFile::ReadBool
 =================
 */
int cweeFile::ReadBool(bool& value) {
	unsigned char c;
	int result = ReadUnsignedChar(c);
	value = c ? true : false;
	return result;
}

/*
 =================
 cweeFile::ReadString
 =================
 */
int cweeFile::ReadString(cweeStr& string) {
	int len;
	int result = 0;

	ReadInt(len);
	if (len >= 0) {
		string.Fill(' ', len);
		result = Read(&string[0], len);
	}
	return result;
}

/*
 =================
 cweeFile::WriteInt
 =================
 */
int cweeFile::WriteInt(const int value) {
	int v = LittleLong(value);
	return Write(&v, sizeof(v));
}

/*
 =================
 cweeFile::WriteUnsignedInt
 =================
 */
int cweeFile::WriteUnsignedInt(const unsigned int value) {
	unsigned int v = LittleLong(value);
	return Write(&v, sizeof(v));
}

/*
 =================
 cweeFile::WriteShort
 =================
 */
int cweeFile::WriteShort(const short value) {
	short v = LittleShort(value);
	return Write(&v, sizeof(v));
}

/*
 =================
 cweeFile::WriteUnsignedShort
 =================
 */
int cweeFile::WriteUnsignedShort(const unsigned short value) {
	unsigned short v = LittleShort(value);
	return Write(&v, sizeof(v));
}

/*
 =================
 cweeFile::WriteChar
 =================
 */
int cweeFile::WriteChar(const char value) {
	return Write(&value, sizeof(value));
}

/*
 =================
 cweeFile::WriteUnsignedChar
 =================
 */
int cweeFile::WriteUnsignedChar(const unsigned char value) {
	return Write(&value, sizeof(value));
}

/*
 =================
 cweeFile::WriteFloat
 =================
 */
int cweeFile::WriteFloat(const float value) {
	float v = LittleFloat(value);
	return Write(&v, sizeof(v));
}

/*
 =================
 cweeFile::WriteBool
 =================
 */
int cweeFile::WriteBool(const bool value) {
	unsigned char c = value;
	return WriteUnsignedChar(c);
}

/*
 =================
 cweeFile::WriteString
 =================
 */
int cweeFile::WriteString(const char* value) {
	int len;

	len = strlen(value);
	WriteInt(len);
	return Write(value, len);
}



/*
=================================================================================

cweeFile_Memory

=================================================================================
*/


/*
=================
cweeFile_Memory::cweeFile_Memory
=================
*/
cweeFile_Memory::cweeFile_Memory(void) {
	name = "*unknown*";
	maxSize = 0;
	fileSize = 0;
	allocated = 0;
	granularity = 16384;

	mode = (1 << FS_WRITE);
	filePtr = NULL;
	curPtr = NULL;
}

/*
=================
cweeFile_Memory::cweeFile_Memory
=================
*/
cweeFile_Memory::cweeFile_Memory(const char* name) {
	this->name = name;
	maxSize = 0;
	fileSize = 0;
	allocated = 0;
	granularity = 16384;

	mode = (1 << FS_WRITE);
	filePtr = NULL;
	curPtr = NULL;
}

/*
=================
cweeFile_Memory::cweeFile_Memory
=================
*/
cweeFile_Memory::cweeFile_Memory(const char* name, char* data, int length) {
	this->name = name;
	maxSize = length;
	fileSize = 0;
	allocated = length;
	granularity = 16384;

	mode = (1 << FS_WRITE);
	filePtr = data;
	curPtr = data;
}

/*
=================
cweeFile_Memory::cweeFile_Memory
=================
*/
cweeFile_Memory::cweeFile_Memory(const char* name, const char* data, int length) {
	this->name = name;
	maxSize = 0;
	fileSize = length;
	allocated = 0;
	granularity = 16384;

	mode = (1 << FS_READ);
	filePtr = const_cast<char*>(data);
	curPtr = const_cast<char*>(data);
}

/*
=================
cweeFile_Memory::~cweeFile_Memory
=================
*/
cweeFile_Memory::~cweeFile_Memory(void) {
	if (filePtr && allocated > 0 && maxSize == 0) {
		Mem_Free(filePtr);
	}
}

/*
=================
cweeFile_Memory::Read
=================
*/
int cweeFile_Memory::Read(void* buffer, int len) {

	if (!(mode & (1 << FS_READ))) {
		return 0;
	}

	if (curPtr + len > filePtr + fileSize) {
		len = filePtr + fileSize - curPtr;
	}
	memcpy(buffer, curPtr, len);
	curPtr += len;
	return len;
}

/*
=================
cweeFile_Memory::Write
=================
*/
int cweeFile_Memory::Write(const void* buffer, int len) {

	if (!(mode & (1 << FS_WRITE))) {
		return 0;
	}

	int alloc = curPtr + len + 1 - filePtr - allocated; // need room for len+1
	if (alloc > 0) {
		if (maxSize != 0) {
			return 0;
		}
		int extra = granularity * (1 + alloc / granularity);
		char* newPtr = (char*)Mem_Alloc((size_t)(allocated + extra),TAG_NEW);
		if (allocated) {
			memcpy(newPtr, filePtr, allocated);
		}
		allocated += extra;
		curPtr = newPtr + (curPtr - filePtr);
		if (filePtr) {
			Mem_Free(filePtr);
		}
		filePtr = newPtr;
	}
	memcpy(curPtr, buffer, len);
	curPtr += len;
	fileSize += len;
	filePtr[fileSize] = 0; // len + 1
	return len;
}

/*
=================
cweeFile_Memory::Length
=================
*/
int cweeFile_Memory::Length(void) {
	return fileSize;
}

/*
=================
cweeFile_Memory::Tell
=================
*/
int cweeFile_Memory::Tell(void) {
	return (curPtr - filePtr);
}

/*
=================
cweeFile_Memory::ForceFlush
=================
*/
void cweeFile_Memory::ForceFlush(void) {
}

/*
=================
cweeFile_Memory::Flush
=================
*/
void cweeFile_Memory::Flush(void) {
}

/*
=================
cweeFile_Memory::Seek

  returns zero on success and -1 on failure
=================
*/
int cweeFile_Memory::Seek(long offset, fsOrigin_t origin) {

	switch (origin) {
	case FS_SEEK_CUR: {
		curPtr += offset;
		break;
	}
	case FS_SEEK_END: {
		curPtr = filePtr + fileSize - offset;
		break;
	}
	case FS_SEEK_SET: {
		curPtr = filePtr + offset;
		break;
	}
	default: {
		return -1;
	}
	}
	if (curPtr < filePtr) {
		curPtr = filePtr;
		return -1;
	}
	if (curPtr > filePtr + fileSize) {
		curPtr = filePtr + fileSize;
		return -1;
	}
	return 0;
}

/*
=================
cweeFile_Memory::MakeReadOnly
=================
*/
void cweeFile_Memory::MakeReadOnly(void) {
	mode = (1 << FS_READ);
	Rewind();
}

/*
=================
cweeFile_Memory::Clear
=================
*/
void cweeFile_Memory::Clear(bool freeMemory) {
	fileSize = 0;
	granularity = 16384;
	if (freeMemory) {
		allocated = 0;
		Mem_Free(filePtr);
		filePtr = NULL;
		curPtr = NULL;
	}
	else {
		curPtr = filePtr;
	}
}

/*
=================
cweeFile_Memory::SetData
=================
*/
void cweeFile_Memory::SetData(const char* data, int length) {
	maxSize = 0;
	fileSize = length;
	allocated = 0;
	granularity = 16384;

	mode = (1 << FS_READ);
	filePtr = const_cast<char*>(data);
	curPtr = const_cast<char*>(data);
}


/*
=================================================================================

cweeFile_Permanent

=================================================================================
*/

/*
=================
cweeFile_Permanent::cweeFile_Permanent
=================
*/
cweeFile_Permanent::cweeFile_Permanent(void) {
	name = "invalid";
	o = NULL;
	mode = 0;
	fileSize = 0;
	handleSync = false;
}

/*
=================
cweeFile_Permanent::~cweeFile_Permanent
=================
*/
cweeFile_Permanent::~cweeFile_Permanent(void) {
	if (o) {
		fclose(o);
	}
}

/*
=================
cweeFile_Permanent::Read

Properly handles partial reads
=================
*/
int cweeFile_Permanent::Read(void* buffer, int len) {
	int		block, remaining;
	int		read;
	byte* buf;
	int		tries;

	if (!(mode & (1 << FS_READ))) {
		return 0;
	}

	if (!o) {
		return 0;
	}

	buf = (byte*)buffer;

	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		read = fread(buf, 1, block, o);
		if (read == 0) {
			// we might have been trying to read from a CD, which
			// sometimes returns a 0 read on windows
			if (!tries) {
				tries = 1;
			}
			else {

				return len - remaining;
			}
		}

		if (read == -1) {
		}

		remaining -= read;
		buf += read;
	}

	return len;
}

/*
=================
cweeFile_Permanent::Write

Properly handles partial writes
=================
*/
int cweeFile_Permanent::Write(const void* buffer, int len) {
	int		block, remaining;
	int		written;
	byte* buf;
	int		tries;

	if (!(mode & (1 << FS_WRITE))) {
		return 0;
	}

	if (!o) {
		return 0;
	}

	buf = (byte*)buffer;

	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		written = fwrite(buf, 1, block, o);
		if (written == 0) {
			if (!tries) {
				tries = 1;
			}
			else {
				return 0;
			}
		}

		if (written == -1) {
			return 0;
		}

		remaining -= written;
		buf += written;
		fileSize += written;
	}
	if (handleSync) {
		fflush(o);
	}
	return len;
}

/*
=================
cweeFile_Permanent::ForceFlush
=================
*/
void cweeFile_Permanent::ForceFlush(void) {
	setvbuf(o, NULL, _IONBF, 0);
}

/*
=================
cweeFile_Permanent::Flush
=================
*/
void cweeFile_Permanent::Flush(void) {
	fflush(o);
}

/*
=================
cweeFile_Permanent::Tell
=================
*/
int cweeFile_Permanent::Tell(void) {
	return ftell(o);
}

/*
================
cweeFile_Permanent::Length
================
*/
int cweeFile_Permanent::Length(void) {
	return fileSize;
}

/*
=================
cweeFile_Permanent::Seek

  returns zero on success and -1 on failure
=================
*/
int cweeFile_Permanent::Seek(long offset, fsOrigin_t origin) {
	int _origin;

	switch (origin) {
	case FS_SEEK_CUR: {
		_origin = SEEK_CUR;
		break;
	}
	case FS_SEEK_END: {
		_origin = SEEK_END;
		break;
	}
	case FS_SEEK_SET: {
		_origin = SEEK_SET;
		break;
	}
	default: {
		_origin = SEEK_CUR;
		break;
	}
	}

	return fseek(o, offset, _origin);
}


/*
=================================================================================

cweeFile_InZip

=================================================================================
*/

/*
=================
cweeFile_InZip::cweeFile_InZip
=================
*/
cweeFile_InZip::cweeFile_InZip(void) {
	//name = "invalid";
	//zipFilePos = 0;
	//fileSize = 0;
	//memset(&z, 0, sizeof(z));
}

/*
=================
cweeFile_InZip::~cweeFile_InZip
=================
*/
cweeFile_InZip::~cweeFile_InZip(void) {
	//unzCloseCurrentFile(z);
	//unzClose(z);
}

/*
=================
cweeFile_InZip::Read

Properly handles partial reads
=================
*/
int cweeFile_InZip::Read(void* buffer, int len) {
	//int l = unzReadCurrentFile(z, buffer, len);
	//return l;
	return 0;
}

/*
=================
cweeFile_InZip::Write
=================
*/
int cweeFile_InZip::Write(const void* buffer, int len) {
	return 0;
}

/*
=================
cweeFile_InZip::ForceFlush
=================
*/
void cweeFile_InZip::ForceFlush(void) {
}

/*
=================
cweeFile_InZip::Flush
=================
*/
void cweeFile_InZip::Flush(void) {
}

/*
=================
cweeFile_InZip::Tell
=================
*/
int cweeFile_InZip::Tell(void) {
	//return unztell(z);
	return 0;
}

/*
================
cweeFile_InZip::Length
================
*/
int cweeFile_InZip::Length(void) {
	//return fileSize;
	return 0;
}

/*
=================
cweeFile_InZip::Seek

  returns zero on success and -1 on failure
=================
*/
#define ZIP_SEEK_BUF_SIZE	(1<<15)

int cweeFile_InZip::Seek(long offset, fsOrigin_t origin) {
	//int res, i;
	//char* buf;

	//switch (origin) {
	//case FS_SEEK_END: {
	//	offset = fileSize - offset;
	//}
	//case FS_SEEK_SET: {
	//	// set the file position in the zip file (also sets the current file info)
	//	unzSetCurrentFileInfoPosition(z, zipFilePos);
	//	unzOpenCurrentFile(z);
	//	if (offset <= 0) {
	//		return 0;
	//	}
	//}
	//case FS_SEEK_CUR: {
	//	buf = (char*)_alloca16(ZIP_SEEK_BUF_SIZE);
	//	for (i = 0; i < (offset - ZIP_SEEK_BUF_SIZE); i += ZIP_SEEK_BUF_SIZE) {
	//		res = unzReadCurrentFile(z, buf, ZIP_SEEK_BUF_SIZE);
	//		if (res < ZIP_SEEK_BUF_SIZE) {
	//			return -1;
	//		}
	//	}
	//	res = i + unzReadCurrentFile(z, buf, offset - i);
	//	return (res == offset) ? 0 : -1;
	//}
	//default: {
	//	break;
	//}
	//}
	return -1;
}
