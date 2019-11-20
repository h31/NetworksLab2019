"""
Error Codes
 Value Meaning
 0 Not defined, see error message (if any).
 1 File not found.
 2 Access violation.
 3 Disk full or allocation exceeded.
 4 Illegal TFTP operation.
 5 Unknown transfer ID.
 6 File already exists.
"""

import struct

errNofile = struct.pack(b'!2H15sB', 5, 1, b'File not found.', 0)
errFileopen = struct.pack(b'!2H18sB', 5, 2, b'Can not open file.', 0)
errFilewrite = struct.pack(b'!2H19sB', 5, 2, b'Can not write file.', 0)
errBlockNo = struct.pack(b'!2H20sB', 5, 5, b'Unknown transfer ID.', 0)
errFileExists = struct.pack(b'!2H20sB', 5, 6, b'File already exists.', 0)
errUnknown = struct.pack(b'!2H23sB', 5, 4, b'Illegal TFTP operation.', 0)
