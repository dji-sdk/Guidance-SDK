#include "protocal_uart_sdk.h"
#include <string.h>
#include "crc32.h"
#include "crc16.h"
#include <stdio.h>
#include <string.h>

#define  LOG_TAG "protocal"

void format_protocal_sdk_uart( unsigned char *src_buf, unsigned int original_len, 
							   unsigned char *dst_buf, unsigned int &new_len, 
							   unsigned short seq_num, unsigned char is_ack,
							   unsigned char enc_type
							  )
{
	protocal_sdk_uart_header *header = (protocal_sdk_uart_header*)dst_buf;
	header->m_header = 0xaa;
	new_len = original_len + sizeof(protocal_sdk_uart_header) + 2 + 4;
	header->m_length = new_len;
	header->m_version = 0;
	header->m_session_id = 0;
	header->m_A = is_ack;
	header->m_R = 0;
	header->m_padding = 0;
	header->m_enc_type = enc_type;
	header->m_seq_num = seq_num;
	Append_CRC16_Check_Sum( (unsigned char *)header, sizeof(protocal_sdk_uart_header) );
	memcpy( dst_buf + sizeof(protocal_sdk_uart_header), src_buf, original_len );

	word32      crc32;            // 32-bit CRC value
	crc32 = update_crc( -1, (byte*)dst_buf, header->m_length );
	printf("CRC = %08X \n", crc32);
}

bool is_header_valid( protocal_sdk_uart_header *header )
{
	return 0!=Verify_CRC16_Check_Sum( (unsigned char*)header, sizeof(*header) );
}

bool is_packet_valid( protocal_sdk_uart_header *header )
{
	char buffer[1024] = {0};
	if ( header->m_length > 1024 )
	{
		return false;
	}

	memcpy( buffer, header, header->m_length );
	word32      crc32;            // 32-bit CRC value
	crc32 = update_crc( -1, (byte*)buffer, header->m_length );
	char *p1 = buffer + header->m_length - 2, *p2 = (char*)header + header->m_length - 2;
	return ( p1[0] == p2[0] && p1[1] == p2[1] );	
}

unsigned char s_data[10240] = {0};
unsigned int s_len = 0;
unsigned int s_cur = 0;

int push( unsigned char *data, unsigned int len )
{
	memcpy( s_data + s_len, data, len );
	s_len += len;

	return 0;
}

int pop( unsigned char *data, unsigned int &len )
{
	static int pack_num = 0;

	for ( ; s_cur < s_len - sizeof(protocal_sdk_uart_header); ++s_cur )
	{
		if ( 0xaa != s_data[s_cur] )
		{
			continue;
		}

		protocal_sdk_uart_header *header = (protocal_sdk_uart_header*)(s_data + s_cur);
		if ( is_header_valid( header ) )
		{
			if ( s_cur + header->m_length > s_len )
			{
				char buffer[1024] = {0};
				memcpy( buffer, s_data + s_cur, s_len - s_cur );
				memcpy( s_data, buffer, s_len - s_cur );
				s_len = s_len - s_cur;
				s_cur = 0;
				break;
			}
			else if( is_packet_valid( header ) )
			{
				len = header->m_length - sizeof(protocal_sdk_uart_header) - 4;
				memcpy( data, s_data + s_cur + sizeof(protocal_sdk_uart_header), len );
				s_cur = s_cur + header->m_length;
				static unsigned short seq_num = 0;
				if( seq_num + 1 == header->m_seq_num )
				{
					seq_num = header->m_seq_num;
				}
				else if( 0 != seq_num )
				{
					seq_num = header->m_seq_num;
				}
				pack_num++;
				return 1;
			}
			else
			{
				printf( "packet err!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
			}
		}
		else
		{
			printf( "header err!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n" );
		}
	}

	return 0;
}
