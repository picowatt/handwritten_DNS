#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define QR_QUERY (uint16_t)0
#define QR_RESPONSE (uint16_t)1
#define OPCODE_STANDARD_QUERY (uint16_t)0
#define OPCODE_INVERSE_QUERY (uint16_t)1
#define OPCODE_SERVER_STATUS_REQUEST (uint16_t)1
#define TC_NOT_TRUNCATED (uint16_t)0
#define RD_RECURSIVE (uint16_t)1

struct dns_header
{
    unsigned char id[2];
    unsigned char flags[2];
    unsigned char qdcount[2];
    unsigned char ancount[2];
    unsigned char nscount[2];
    unsigned char arcount[2];
};

enum dns_header_flag
{
    QR,
    Opcode,
    AA,
    TC,
    RD,
    RA,
    Z,
    RCODE
};

struct dns_question
{
    unsigned char *qname;
    int qname_len; 
    unsigned char qtype[3];
    unsigned char qclass[3];
};

#define QTYPE_A (uint16_t)1
#define QCLASS_IN (uint16_t)1

void dns_set_flag(uint16_t *flags, enum dns_header_flag flag, uint16_t value)
{
    switch (flag)
    {
    case QR:
        *flags |= value << 15;
        break;
    case Opcode:
        *flags |= value << 11;
        break;
    case TC:
        *flags |= value << 9;
        break;
    case RD:
        *flags |= value << 8;
        break;
    default:
        break;
    }
}

void set_field(unsigned char *arr, uint16_t num)
{
    arr[1] = num;
    arr[0] = num >> 8;

    return arr;
}

void set_arr(unsigned char *to, unsigned char from[2])
{
    *(to) = from[0];
    *(to + 1) = from[1];
}

int main()
{
    unsigned char out_buf[1024] = {0};
    unsigned char in_buf[1024] = {0};
    char str[] = "google.com";

    struct dns_header header;
    memset(&header, 0, sizeof(header));

    set_field(header.id, 0xAAAA);
    uint16_t flags = 0;
    dns_set_flag(&flags, RD, RD_RECURSIVE);
    set_field(header.flags, flags);
    set_field(header.qdcount, (uint16_t)1);

    unsigned char header_a[12] = {0};
    set_arr(header_a, header.id);
    set_arr(header_a + 2, header.flags);
    set_arr(header_a + 4, header.qdcount);
    set_arr(header_a + 6, header.ancount);
    set_arr(header_a + 8, header.nscount);
    set_arr(header_a + 10, header.arcount);

    unsigned char buf_s[256] = {0};
    unsigned char q_name_buf[256] = {0};
    struct dns_question message;
    memset(&message, 0, sizeof(message));
    set_field(message.qtype, QTYPE_A);
    set_field(message.qclass, QCLASS_IN);

    char *tok;
    char *rest = str;
    int written = 0;
    while ((tok = strtok_r(rest, ".", &rest)))
    {
        written += sprintf(buf_s, "%c%s", strlen(tok), tok);
        strcat(q_name_buf, buf_s);
    }
    message.qname = q_name_buf;
    message.qname_len = written;

    int size = 4 + 1 + message.qname_len;
    unsigned char *message_a = malloc(size * sizeof(unsigned char));
    memset(message_a, 0x00, size);
    memcpy(message_a, message.qname, message.qname_len);
    int offset = message.qname_len + 1;
    set_arr(message_a + offset, message.qtype);
    offset += 2;
    set_arr(message_a + offset, message.qclass);
    offset += 2;

    memcpy(out_buf, header_a, 12);
    memcpy(out_buf + 12, message_a, offset);

    for(int i = 0; i < offset + 12; i += 2 )
    {
        if((offset % 2 != 0) && i == (offset - 1))
        {
            printf("%02hhX\n", out_buf[i]);
            continue;
        }
        printf("%02hhX %02hhX\n", out_buf[i], out_buf[i + 1]);
    }

    free(message_a);
    return 0;
}