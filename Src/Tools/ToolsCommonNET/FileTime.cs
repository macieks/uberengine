using System;
using System.Runtime.InteropServices;

namespace ToolsCommonNET
{
    public class FileTime
    {
        /// <summary>
        /// File attributes are metadata values stored by the file system on disk and are used by the system and are available to developers via various file I/O APIs.
        /// </summary>
        public enum FileAttributes : uint
        {
            /// <summary>
            /// A file that is read-only. Applications can read the file, but cannot write to it or delete it. This attribute is not honored on directories. For more information, see "You cannot view or change the Read-only or the System attributes of folders in Windows Server 2003, in Windows XP, or in Windows Vista".
            /// </summary>
            Readonly = 0x00000001,

            /// <summary>
            /// The file or directory is hidden. It is not included in an ordinary directory listing.
            /// </summary>
            Hidden = 0x00000002,

            /// <summary>
            /// A file or directory that the operating system uses a part of, or uses exclusively.
            /// </summary>
            System = 0x00000004,

            /// <summary>
            /// The handle that identifies a directory.
            /// </summary>
            Directory = 0x00000010,

            /// <summary>
            /// A file or directory that is an archive file or directory. Applications typically use this attribute to mark files for backup or removal.
            /// </summary>
            Archive = 0x00000020,

            /// <summary>
            /// This value is reserved for system use.
            /// </summary>
            Device = 0x00000040,

            /// <summary>
            /// A file that does not have other attributes set. This attribute is valid only when used alone.
            /// </summary>
            Normal = 0x00000080,

            /// <summary>
            /// A file that is being used for temporary storage. File systems avoid writing data back to mass storage if sufficient cache memory is available, because typically, an application deletes a temporary file after the handle is closed. In that scenario, the system can entirely avoid writing the data. Otherwise, the data is written after the handle is closed.
            /// </summary>
            Temporary = 0x00000100,

            /// <summary>
            /// A file that is a sparse file.
            /// </summary>
            SparseFile = 0x00000200,

            /// <summary>
            /// A file or directory that has an associated reparse point, or a file that is a symbolic link.
            /// </summary>
            ReparsePoint = 0x00000400,

            /// <summary>
            /// A file or directory that is compressed. For a file, all of the data in the file is compressed. For a directory, compression is the default for newly created files and subdirectories.
            /// </summary>
            Compressed = 0x00000800,

            /// <summary>
            /// The data of a file is not available immediately. This attribute indicates that the file data is physically moved to offline storage. This attribute is used by Remote Storage, which is the hierarchical storage management software. Applications should not arbitrarily change this attribute.
            /// </summary>
            Offline = 0x00001000,

            /// <summary>
            /// The file or directory is not to be indexed by the content indexing service.
            /// </summary>
            NotContentIndexed = 0x00002000,

            /// <summary>
            /// A file or directory that is encrypted. For a file, all data streams in the file are encrypted. For a directory, encryption is the default for newly created files and subdirectories.
            /// </summary>
            Encrypted = 0x00004000,

            /// <summary>
            /// This value is reserved for system use.
            /// </summary>
            Virtual = 0x00010000
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct MY_FILETIME
        {
            public UInt32 dwLowDateTime;
            public UInt32 dwHighDateTime;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct WIN32_FILE_ATTRIBUTE_DATA
        {
            public FileAttributes dwFileAttributes;
            public MY_FILETIME ftCreationTime;
            public MY_FILETIME ftLastAccessTime;
            public MY_FILETIME ftLastWriteTime;
            public uint nFileSizeHigh;
            public uint nFileSizeLow;
        }

        public enum GET_FILEEX_INFO_LEVELS
        {
            GetFileExInfoStandard,
            GetFileExMaxInfoLevel
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct SYSTEMTIME
        {
            public UInt16 wYear;
            public UInt16 wMonth;
            public UInt16 wDayOfWeek;
            public UInt16 wDay;
            public UInt16 wHour;
            public UInt16 wMinute;
            public UInt16 wSecond;
            public UInt16 wMilliseconds;
        }

        [DllImport("kernel32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool GetFileAttributesEx(IntPtr lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, IntPtr lpFileInformation);

        [DllImport("kernel32.dll")]
        [return: MarshalAs(UnmanagedType.Bool)]
        static extern bool FileTimeToSystemTime(IntPtr fileTime, IntPtr sysTime);

        public static DateTime GetLastWriteTime(string path)
        {
            // Trying to match C++ (there's difference between C# and C++ file write times due to daylight saving change)

            // Get file attributes

            IntPtr attrsPtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(WIN32_FILE_ATTRIBUTE_DATA)));
            if (GetFileAttributesEx(Marshal.StringToHGlobalAnsi(path), GET_FILEEX_INFO_LEVELS.GetFileExInfoStandard, attrsPtr) == false)
            {
                Marshal.FreeHGlobal(attrsPtr);
                return DateTime.Now;
            }
            WIN32_FILE_ATTRIBUTE_DATA attrs = (WIN32_FILE_ATTRIBUTE_DATA)Marshal.PtrToStructure(attrsPtr, typeof(WIN32_FILE_ATTRIBUTE_DATA));

            // Extract file write time

            IntPtr fileTimePtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(MY_FILETIME)));
            Marshal.StructureToPtr(attrs.ftLastWriteTime, fileTimePtr, false);
            Marshal.FreeHGlobal(attrsPtr);

            // Convert file time to system time

            IntPtr systemTimePtr = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(SYSTEMTIME)));
            if (FileTimeToSystemTime(fileTimePtr, systemTimePtr) == false)
            {
                Marshal.FreeHGlobal(fileTimePtr);
                Marshal.FreeHGlobal(systemTimePtr);
                return DateTime.Now;
            }
            Marshal.FreeHGlobal(fileTimePtr);
            SYSTEMTIME systemTime = (SYSTEMTIME)Marshal.PtrToStructure(systemTimePtr, typeof(SYSTEMTIME));
            Marshal.FreeHGlobal(systemTimePtr);

            // Convert to .net DateTime

            DateTime dt = new DateTime(systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
            return dt;
        }
    }
}
