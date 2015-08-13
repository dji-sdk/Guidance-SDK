/*
** global function
*/ 
extern unsigned short Get_CRC16_Check_Sum(unsigned char *pchMessage,unsigned int dwLength,unsigned short wCRC); 
extern unsigned int Verify_CRC16_Check_Sum(unsigned char* pchMessage, unsigned int dwLength); 
extern void Append_CRC16_Check_Sum(unsigned char* pchMessage, unsigned int dwLength); 

/*
** global const variable 
*/ 
extern const unsigned short wCRC_Table[256];
extern const unsigned short CRC_INIT; 