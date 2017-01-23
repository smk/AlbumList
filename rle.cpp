#include "stdafx.h"

/* Error codes sent to the caller */
#define BAD_FILE_NAME 1
#define BAD_ARGUMENT  2

/*	Being that fgetc=EOF only after an access
	then 'byte_stored_status' is 'TRUE' if a byte has been stored by 'fgetc'
	or 'FALSE' if there's no valid byte not already read and not handled in 'val_byte_stored'
 */

static int		val_byte_stored;
static int		current_index	= 0;
static int		buffer_size		= 0;
static BYTE	   *buffer			= NULL;
static HANDLE	dest_file		= NULL;
static HANDLE	source_file		= NULL;
static DWORD	bytes_written	= 0;

/* Pseudo procedures */
#define end_of_data()						(current_index == buffer_size)
#define read_byte()							(buffer[current_index++])
#define write_byte(byte)					(val_byte_stored = (byte), WriteFile(dest_file, &val_byte_stored, 1, &bytes_written, NULL))
#define write_array(array,byte_nb_to_write)	(WriteFile(dest_file, array, byte_nb_to_write, &bytes_written, NULL))

/* Returned parameters: None
   Action: Compresses with RLE type 1 method all bytes read by the function 'read_byte'
   Errors: An input/output error could disturb the running of the program
*/
void RLE_Encoding(HANDLE fp, BYTE *buf, int size)
{
	register unsigned char byte1,byte2, frame_size, array[129];

	dest_file = fp;
	buffer = buf;
	buffer_size = size;
	current_index = 0;

	if (!end_of_data())
	{
		byte1 = read_byte();    /* Is there at least a byte to analyze? */
		frame_size = 1;

		if (!end_of_data())
		/* Are there at least two bytes to analyze? */
		{
			byte2 = read_byte();
			frame_size = 2;

            do
			{
				if (byte1 == byte2)
                /* Is there a repetition? */
				{
					while ((!end_of_data()) && (byte1 == byte2) && (frame_size < 129))
                    {
						byte2 = read_byte();
						frame_size++;
                    }
                    if (byte1 == byte2)
                    /* Do we meet only a sequence of bytes? */
                    {
						write_byte(126+frame_size);
                        write_byte(byte1);
                        if (!end_of_data())
                        {
							byte1 = read_byte();
                            frame_size = 1;
                        }
                        else frame_size = 0;
                    }
                    else   /* No, then don't handle the last byte */
                    {
						write_byte(125+frame_size);
                        write_byte(byte1);
                        byte1 = byte2;
                        frame_size = 1;
					}
					if (!end_of_data())
					{
						byte2 = read_byte();
						frame_size = 2;
					}
				}
				else        /* Prepare the array of comparisons
                                where will be stored all the identical bytes */
				{
					*array = byte1;
					array[1] = byte2;
					while ((!end_of_data()) && (array[frame_size-2] != array[frame_size-1]) && (frame_size < 128))
					{
						array[frame_size] = read_byte();
						frame_size++;
					}
                    
					if (array[frame_size-2] == array[frame_size-1])
                    /* Do we meet a sequence of all different bytes followed by identical byte? */
					/* Yes, then don't count the two last bytes */
					{ 
						write_byte(frame_size-3);
						write_array(array,frame_size-2);
						byte1=array[frame_size-2];
						byte2 = byte1;
						frame_size = 2;
					}
					else 
					{
						write_byte(frame_size-1);
						write_array(array,frame_size);
						if (end_of_data())
							frame_size = 0;
						else 
						{
							byte1 = read_byte();
							if (end_of_data())
								frame_size = 1;
							else
							{
								byte2 = read_byte();
								frame_size = 2;
							}
						}
					}
				}
            }
            while ((!end_of_data()) || (frame_size >= 2));
		}

		if (frame_size == 1)
		{
			write_byte(0);
			write_byte(byte1);
		}
	}
}

static int byte_stored_status=FALSE;
static DWORD nBytesRead;

BOOL d_end_of_data()
{
	if (byte_stored_status) return FALSE;
		
	BYTE byte = 0;
	if (!ReadFile(source_file, &byte, 1, &nBytesRead, NULL) ||
		(nBytesRead == 0))
	{
		byte_stored_status = FALSE;
		return TRUE;
	}

	val_byte_stored = byte;
	byte_stored_status = TRUE;

	return FALSE;
}

BYTE d_read_byte()
{
	if (byte_stored_status)
	{
		byte_stored_status = FALSE;
		return (BYTE)val_byte_stored;
	}
	else
	{
		BYTE byte = 0;
		if (ReadFile(source_file, &byte, 1, &nBytesRead, NULL))
		{
			return byte;
		}
	}
	return 0;
}

void d_write_byte(BYTE byte)
{
	buffer[current_index++] = byte;
}

void d_write_block(BYTE byte, BYTE time_nb)
{ 
    memset(&buffer[current_index], byte, time_nb);
	current_index += time_nb;
}

BOOL RLE_Decoding(HANDLE fp, BYTE *buf, int size)
/* Returned parameters: None
   Action: Decompresses with RLE type 1 method all bytes read by the function read_byte
   Erreurs: An input/output error could disturb the running of the program
 */
{
	unsigned char header;
	register unsigned char i;

	source_file = fp;
	buffer = buf;
	current_index = 0;
	buffer_size = size;

	while (!d_end_of_data())
	{
		header = d_read_byte();
		switch (header & 128)
		{
		case 0:
			if (!d_end_of_data())
				for (i=0; i<=header; i++)
					d_write_byte(d_read_byte());
			/* else INVALID FILE */
			break;

		case 128:
			if (!d_end_of_data())
				d_write_block(d_read_byte(), (header & 127) + 2);
			/* else INVALID FILE */
		}
		if (current_index == buffer_size) break;
		if (current_index > buffer_size) return FALSE;
	}

	return TRUE;
}
