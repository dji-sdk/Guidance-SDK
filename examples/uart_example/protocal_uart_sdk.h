#ifndef __PROTOCAL_UART_SDK_H__
#define __PROTOCAL_UART_SDK_H__

#pragma pack(1)

typedef struct _protocal_sdk_uart_header
{
	unsigned char         m_header;
	unsigned short        m_version : 6;
	unsigned short        m_length : 10;
	unsigned char         m_R : 2;
	unsigned char         m_A : 1;
	unsigned char         m_session_id :5;
	unsigned char         m_enc_type : 3;
	unsigned char         m_padding : 5;
	unsigned char         m_reserved[3];
	unsigned short        m_seq_num;
	unsigned short        m_header_checksum;
}protocal_sdk_uart_header;

#pragma pack()

typedef struct _VO_OUTPUT
{
	short cnt;

	short vx;
	short vy;
	short vz;

	float x;
	float y;
	float z;

	float reserved[12];

	float height;
	float uncertainty_height;

	unsigned char reserve[4];
}VO_OUTPUT;

#pragma pack()

class soc2pc_vo_can_output
{
public:
	VO_OUTPUT  m_vo_output;
	unsigned int  m_frame_index;
	unsigned int  m_time_stamp;
	unsigned int  m_reserved[9];
};

bool is_header_valid( protocal_sdk_uart_header *header );

bool is_packet_valid( protocal_sdk_uart_header *header );

int pop( unsigned char *data, unsigned int &len );

int push( unsigned char *data, unsigned int len );

#endif
