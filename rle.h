#ifndef __RLE_H__
#define __RLE_H__

void RLE_Encoding(HANDLE fp, BYTE *buf, int size);
BOOL RLE_Decoding(HANDLE fp, BYTE *buf, int size);

#endif /* __RLE_H__ */
