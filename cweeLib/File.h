
#ifndef __FILE_H__
#define __FILE_H__

/*
==============================================================

  File Streams.

==============================================================
*/

// mode parm for Seek
typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

class cweeFileSystemLocal;


class cweeFile {
public:
	virtual					~cweeFile(void) {};
	// Get the name of the file.
	virtual const char* GetName(void);
	// Get the full file path.
	virtual const char* GetFullPath(void);
	// Read data from the file to the buffer.
	virtual int				Read(void* buffer, int len);
	// Write data from the buffer to the file.
	virtual int				Write(const void* buffer, int len);
	// Returns the length of the file.
	virtual int				Length(void);
	// Returns offset in file.
	virtual int				Tell(void);
	// Forces flush on files being writting to.
	virtual void			ForceFlush(void);
	// Causes any buffered data to be written to the file.
	virtual void			Flush(void);
	// Seek on a file.
	virtual int				Seek(long offset, fsOrigin_t origin);
	// Go back to the beginning of the file.
	virtual void			Rewind(void);
	// Like fprintf.
	virtual int				Printf(const char* fmt, ...) id_attribute((format(printf, 2, 3)));
	// Like fprintf but with argument pointer
	virtual int				VPrintf(const char* fmt, va_list arg);
	// Write a string with high precision floating point numbers to the file.
	virtual int				WriteFloatString(const char* fmt, ...) id_attribute((format(printf, 2, 3)));

	// Endian portable alternatives to Read(...)
	virtual int				ReadInt(int& value);
	virtual int				ReadUnsignedInt(unsigned int& value);
	virtual int				ReadShort(short& value);
	virtual int				ReadUnsignedShort(unsigned short& value);
	virtual int				ReadChar(char& value);
	virtual int				ReadUnsignedChar(unsigned char& value);
	virtual int				ReadFloat(float& value);
	virtual int				ReadBool(bool& value);
	virtual int				ReadString(cweeStr& string);

	// Endian portable alternatives to Write(...)
	virtual int				WriteInt(const int value);
	virtual int				WriteUnsignedInt(const unsigned int value);
	virtual int				WriteShort(const short value);
	virtual int				WriteUnsignedShort(unsigned short value);
	virtual int				WriteChar(const char value);
	virtual int				WriteUnsignedChar(const unsigned char value);
	virtual int				WriteFloat(const float value);
	virtual int				WriteBool(const bool value);
	virtual int				WriteString(const char* string);
};


class cweeFile_Memory : public cweeFile {
	friend class			cweeFileSystemLocal;

public:
	cweeFile_Memory(void);	// file for writing without name
	cweeFile_Memory(const char* name);	// file for writing
	cweeFile_Memory(const char* name, char* data, int length);	// file for writing
	cweeFile_Memory(const char* name, const char* data, int length);	// file for reading
	virtual					~cweeFile_Memory(void);

	virtual const char* GetName(void) { return name.c_str(); }
	virtual const char* GetFullPath(void) { return name.c_str(); }
	virtual int				Read(void* buffer, int len);
	virtual int				Write(const void* buffer, int len);
	virtual int				Length(void);
	virtual int				Tell(void);
	virtual void			ForceFlush(void);
	virtual void			Flush(void);
	virtual int				Seek(long offset, fsOrigin_t origin);

	// changes memory file to read only
	virtual void			MakeReadOnly(void);
	// clear the file
	virtual void			Clear(bool freeMemory = true);
	// set data for reading
	void					SetData(const char* data, int length);
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

class cweeFile_Permanent : public cweeFile {
	friend class			cweeFileSystemLocal;

public:
	cweeFile_Permanent(void);
	virtual					~cweeFile_Permanent(void);

	virtual const char* GetName(void) { return name.c_str(); }
	virtual const char* GetFullPath(void) { return fullPath.c_str(); }
	virtual int				Read(void* buffer, int len);
	virtual int				Write(const void* buffer, int len);
	virtual int				Length(void);
	virtual int				Tell(void);
	virtual void			ForceFlush(void);
	virtual void			Flush(void);
	virtual int				Seek(long offset, fsOrigin_t origin);

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


class cweeFile_InZip : public cweeFile {
	friend class			cweeFileSystemLocal;

public:
	cweeFile_InZip(void);
	virtual					~cweeFile_InZip(void);

	virtual const char*		GetName(void) { return name.c_str(); }
	virtual const char*		GetFullPath(void) { return fullPath.c_str(); }
	virtual int				Read(void* buffer, int len);
	virtual int				Write(const void* buffer, int len);
	virtual int				Length(void);
	virtual int				Tell(void);
	virtual void			ForceFlush(void);
	virtual void			Flush(void);
	virtual int				Seek(long offset, fsOrigin_t origin);

//private:
	cweeStr					name;			// name of the file in the pak
	cweeStr					fullPath;		// full file path including pak file name
	int						zipFilePos;		// zip file info position in pak
	int						fileSize;		// size of the file
	void* z;				// unzip info
};

#endif /* !__FILE_H__ */
