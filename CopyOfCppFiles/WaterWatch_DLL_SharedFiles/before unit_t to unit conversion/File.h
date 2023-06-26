#pragma once
#include "Precompiled.h"

// mode parm for Seek
typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

class cweeFile {
public:
	static int FS_WriteFloatString(char* buf, const char* fmt, va_list argPtr) {
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
	};

	virtual					~cweeFile(void) {};
	// Get the name of the file.
	virtual const char* GetName(void) {
		return "";
	};
	// Get the full file path.
	virtual const char* GetFullPath(void) {
		return "";
	};
	// Read data from the file to the buffer.
	virtual int				Read(void* buffer, int len) {
		return 0;
	};
	// Write data from the buffer to the file.
	virtual int				Write(const void* buffer, int len) {
		return 0;
	};
	// Returns the length of the file.
	virtual int				Length(void) {
		return 0;
	};
	// Returns offset in file.
	virtual int				Tell(void) {
		return 0;
	};
	// Forces flush on files being writting to.
	virtual void			ForceFlush(void) {
	};
	// Causes any buffered data to be written to the file.
	virtual void			Flush(void) {
	};
	// Seek on a file.
	virtual int				Seek(long offset, fsOrigin_t origin) {
		return -1;
	};
	// Go back to the beginning of the file.
	virtual void			Rewind(void) {
		Seek(0, FS_SEEK_SET);
	};
	virtual int				Printf(const char* fmt, ...) {
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
	};
	virtual int				VPrintf(const char* fmt, va_list arg) {
		char buf[MAX_PRINT_MSG];
		int length;

		length = cweeStr::vsnPrintf(buf, MAX_PRINT_MSG - 1, fmt, arg);
		return Write(buf, length);
	};
	virtual int				WriteFloatString(const char* fmt, ...) {
		char buf[MAX_PRINT_MSG];
		int len;
		va_list argPtr;

		va_start(argPtr, fmt);
		len = FS_WriteFloatString(buf, fmt, argPtr);
		va_end(argPtr);

		return Write(buf, len);
	};

	// Endian portable alternatives to Read(...)
	virtual int				ReadInt(int& value) {
		int result = Read(&value, sizeof(value));
		value = LittleLong(value);
		return result;
	};
	virtual int				ReadUnsignedInt(unsigned int& value) {
		int result = Read(&value, sizeof(value));
		value = LittleLong(value);
		return result;
	};
	virtual int				ReadShort(short& value) {
		int result = Read(&value, sizeof(value));
		value = LittleShort(value);
		return result;
	};
	virtual int				ReadUnsignedShort(unsigned short& value) {
		int result = Read(&value, sizeof(value));
		value = LittleShort(value);
		return result;
	};
	virtual int				ReadChar(char& value) {
		return Read(&value, sizeof(value));
	};
	virtual int				ReadUnsignedChar(unsigned char& value) {
		return Read(&value, sizeof(value));
	};
	virtual int				ReadFloat(float& value) {
		int result = Read(&value, sizeof(value));
		value = LittleFloat(value);
		return result;
	};
	virtual int				ReadBool(bool& value) {
		unsigned char c;
		int result = ReadUnsignedChar(c);
		value = c ? true : false;
		return result;
	};
	virtual int				ReadString(cweeStr& string) {
		int len;
		int result = 0;

		ReadInt(len);
		if (len >= 0) {
			string.Fill(' ', len);
			result = Read(&string[0], len);
		}
		return result;
	};

	// Endian portable alternatives to Write(...)
	virtual int				WriteInt(const int value) {
		int v = LittleLong(value);
		return Write(&v, sizeof(v));
	};
	virtual int				WriteUnsignedInt(const unsigned int value) {
		unsigned int v = LittleLong(value);
		return Write(&v, sizeof(v));
	};
	virtual int				WriteShort(const short value) {
		short v = LittleShort(value);
		return Write(&v, sizeof(v));
	};
	virtual int				WriteUnsignedShort(unsigned short value) {
		unsigned short v = LittleShort(value);
		return Write(&v, sizeof(v));
	};
	virtual int				WriteChar(const char value) {
		return Write(&value, sizeof(value));
	};
	virtual int				WriteUnsignedChar(const unsigned char value) {
		return Write(&value, sizeof(value));
	};
	virtual int				WriteFloat(const float value) {
		float v = LittleFloat(value);
		return Write(&v, sizeof(v));
	};
	virtual int				WriteBool(const bool value) {
		unsigned char c = value;
		return WriteUnsignedChar(c);
	};
	virtual int				WriteString(const char* string) {
		int len;

		len = strlen(string);
		WriteInt(len);
		return Write(string, len);
	};
};

typedef enum {
	FS_READ = 0,
	FS_WRITE = 1,
	FS_APPEND = 2
} fsMode_t;

class cweeFile_Memory  : public cweeFile {
	cweeFile_Memory(void) {
		name = "*unknown*";
		maxSize = 0;
		fileSize = 0;
		allocated = 0;
		granularity = 16384;

		mode = (1 << FS_WRITE);
		filePtr = NULL;
		curPtr = NULL;
	};	// file for writing without name
	cweeFile_Memory(const char* name) {
		this->name = name;
		maxSize = 0;
		fileSize = 0;
		allocated = 0;
		granularity = 16384;

		mode = (1 << FS_WRITE);
		filePtr = NULL;
		curPtr = NULL;
	};	// file for writing
	cweeFile_Memory(const char* name, char* data, int length) {
		this->name = name;
		maxSize = length;
		fileSize = 0;
		allocated = length;
		granularity = 16384;

		mode = (1 << FS_WRITE);
		filePtr = data;
		curPtr = data;
	};	// file for writing
	cweeFile_Memory(const char* name, const char* data, int length) {
		this->name = name;
		maxSize = 0;
		fileSize = length;
		allocated = 0;
		granularity = 16384;

		mode = (1 << FS_READ);
		filePtr = const_cast<char*>(data);
		curPtr = const_cast<char*>(data);
	};	// file for reading
	virtual					~cweeFile_Memory(void) {
		if (filePtr && allocated > 0 && maxSize == 0) {
			Mem_Free(filePtr);
		}
	};

	virtual const char* GetName(void) { return name.c_str(); }
	virtual const char* GetFullPath(void) { return name.c_str(); }
	virtual int				Read(void* buffer, int len) {

		if (!(mode & (1 << FS_READ))) {
			return 0;
		}

		if (curPtr + len > filePtr + fileSize) {
			len = filePtr + fileSize - curPtr;
		}
		memcpy(buffer, curPtr, len);
		curPtr += len;
		return len;
	};
	virtual int				Write(const void* buffer, int len) {

		if (!(mode & (1 << FS_WRITE))) {
			return 0;
		}

		int alloc = curPtr + len + 1 - filePtr - allocated; // need room for len+1
		if (alloc > 0) {
			if (maxSize != 0) {
				return 0;
			}
			int extra = granularity * (1 + alloc / granularity);
			char* newPtr = (char*)Mem_Alloc((size_t)(allocated + extra), TAG_NEW);
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
	};
	virtual int				Length(void) {
		return fileSize;
	};
	virtual int				Tell(void) {
		return (curPtr - filePtr);
	};
	virtual void			ForceFlush(void) {
	};
	virtual void			Flush(void) {
	};
	virtual int				Seek(long offset, fsOrigin_t origin) {

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
	};

	// changes memory file to read only
	virtual void			MakeReadOnly(void) {
		mode = (1 << FS_READ);
		Rewind();
	};
	// clear the file
	virtual void			Clear(bool freeMemory = true) {
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
	};
	// set data for reading
	void					SetData(const char* data, int length) {
		maxSize = 0;
		fileSize = length;
		allocated = 0;
		granularity = 16384;

		mode = (1 << FS_READ);
		filePtr = const_cast<char*>(data);
		curPtr = const_cast<char*>(data);
	};
	// returns const pointer to the memory buffer
	const char* GetDataPtr(void) const { return filePtr; }
	// set the file granularity
	void					SetGranularity(int g) { assert(g > 0); granularity = g; }

	//private:
	cweeStr					name;			// name of the file
	int						mode;			// open mode
	int						maxSize;		// maximum size of file
	int						fileSize;		// size of the file
	int						allocated;		// allocated size
	int						granularity;	// file granularity
	char* filePtr;		// buffer holding the file data
	char* curPtr;			// current read/write pointer
};

class cweeFile_Permanent  : public cweeFile {
	cweeFile_Permanent(void) {
		name = "invalid";
		o = NULL;
		mode = 0;
		fileSize = 0;
		handleSync = false;
	};
	virtual					~cweeFile_Permanent(void) {
		if (o) {
			fclose(o);
		}
	};

	virtual const char* GetName(void) { return name.c_str(); }
	virtual const char* GetFullPath(void) { return fullPath.c_str(); }
	virtual int				Read(void* buffer, int len) {
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
	};
	virtual int				Write(const void* buffer, int len) {
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
	};
	virtual int				Length(void) {
		return fileSize;
	};
	virtual int				Tell(void) {
		return ftell(o);
	};
	virtual void			ForceFlush(void) {
		setvbuf(o, NULL, _IONBF, 0);
	};
	virtual void			Flush(void) {
		fflush(o);
	};
	virtual int				Seek(long offset, fsOrigin_t origin) {
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
	};

	// returns file pointer
	FILE* GetFilePtr(void) { return o; }

	//private:
	cweeStr					name;			// relative path of the file - relative path
	cweeStr					fullPath;		// full file path - OS path
	int						mode;			// open mode
	int						fileSize;		// size of the file
	FILE* o;				// file handle
	bool					handleSync;		// true if written data is immediately flushed
};

class cweeFile_InZip  : public cweeFile {
	cweeFile_InZip(void) {
		//name = "invalid";
		//zipFilePos = 0;
		//fileSize = 0;
		//memset(&z, 0, sizeof(z));
	};
	virtual					~cweeFile_InZip(void) {
		//unzCloseCurrentFile(z);
		//unzClose(z);
	};

	virtual const char*		GetName(void) { return name.c_str(); }
	virtual const char*		GetFullPath(void) { return fullPath.c_str(); }
	virtual int				Read(void* buffer, int len) {
		//int l = unzReadCurrentFile(z, buffer, len);
		//return l;
		return 0;
	};
	virtual int				Write(const void* buffer, int len) {
		return 0;
	};
	virtual int				Length(void) {
		//return fileSize;
		return 0;
	};
	virtual int				Tell(void) {
		//return unztell(z);
		return 0;
	};
	virtual void			ForceFlush(void) {
	};
	virtual void			Flush(void) {
	};
	virtual int				Seek(long offset, fsOrigin_t origin) {
#define ZIP_SEEK_BUF_SIZE	(1<<15)
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
#undef ZIP_SEEK_BUF_SIZE
		return -1;
	};

	//private:
	cweeStr					name;			// name of the file in the pak
	cweeStr					fullPath;		// full file path including pak file name
	int						zipFilePos;		// zip file info position in pak
	int						fileSize;		// size of the file
	void* z;				// unzip info
};