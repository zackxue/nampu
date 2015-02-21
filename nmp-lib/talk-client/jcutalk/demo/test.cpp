
//#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//#include "Alaw_encoder.h"
#include "WavHeader.h "
#include "g711.h"

#include "..\\jcutalk.h"
#pragma comment(lib, "..\\Release\\jcutalk.lib")

#define ASSERT0(r) do{int _r; printf("%d => r:%d\n", __LINE__, _r = r); if(_r<0) abort();}while(0)
#define ASSERTp(p) do{void *_p; printf("%d => p:%p\n", __LINE__, _p = p); if(_p==NULL) abort();}while(0)

#define PRINT(fmt, ...) \
	do \
	{ \
		char buf[1024] = {0}; \
		sprintf_s(buf, fmt, ## __VA_ARGS__); \
		OutputDebugStringA(buf); \
	} while (0);


typedef struct talk_ctx_s {
    int start;
    int close;
}talk_ctx_t;

///////////////////////////////
#define G711A_FILE "lsy_andy.alaw"
#define G711U_FILE "lsy_andy.ulaw"
#define AUDIO_FILE "sample.pcm"
static FILE *__get_auido_file()
{
	int err;
	static FILE *g_audio_file = NULL;
	if (g_audio_file == NULL)
		g_audio_file = fopen(G711U_FILE, "rb");

	assert(g_audio_file);
	return g_audio_file;
}

#define TEST_FILE "test.pcm"
static FILE *__get_write_auido_file()
{
	int err;
	static FILE *g_audio_file = NULL;
	if (g_audio_file == NULL)
		g_audio_file = fopen(TEST_FILE, "wb");

	assert(g_audio_file);
	return g_audio_file;
}

static void __write_wav_header(FILE *file)
{
	printf("@@@@@@@@@@@@@@22 sizeof(struct tagHXD_WAVFLIEHEAD):%d\n", sizeof(struct tagHXD_WAVFLIEHEAD));
	assert(file);
	copy_wav_header(file);
}

static int __read_audio_data(char *buf, int size)
{
	assert(buf && size > 0);

	for (;;)
	{
		if ((fread(buf, 1, size, __get_auido_file()) != size))
		{
			return fseek(__get_auido_file(), 0, SEEK_SET);
		}

		break;
	}

	return 0;
}

static int __write_audio_data(char *buf, int size)
{
	assert(buf && size > 0);

	int ret;
	int write_size = 0;
	while (write_size < size)
	{
		ret = fwrite(buf + write_size, 1, size - write_size, __get_write_auido_file());
		if (ret < 0)
		{
			PRINT("write file error err:%d\n", GetLastError());
			return -1;
		}
		write_size += ret;
	}

	return write_size;
}

#define ADD_FRAME(frm) \
	do \
	{ \
		char buf[1024] = {0}; \
		int read_size = 0, size = 320; \
		memcpy(buf, frm, sizeof(HI_TALK_FRMAE_HDR)); \
		read_size = __read_audio_data(buf + sizeof(HI_TALK_FRMAE_HDR), size); \
		frm = (HI_TALK_FRMAE_HDR*)buf; \
	} while (0);

static short __linear16FromuLaw(unsigned char uLawByte) {
	static int const exp_lut[8] = {0,132,396,924,1980,4092,8316,16764};
	uLawByte = ~uLawByte;

	bool sign = (uLawByte & 0x80) != 0;
	unsigned char exponent = (uLawByte>>4) & 0x07;
	unsigned char mantissa = uLawByte & 0x0F;

	short result = exp_lut[exponent] + (mantissa << (exponent+3));
	if (sign) result = -result;
	return result;
}

static short __linear16FromaLaw(unsigned char uLawByte) 
{
	unsigned int shift;
	unsigned int sign;
	unsigned int linear;

	uLawByte ^= 0x55;  // A-law has alternate bits inverted for transmission   

	sign = uLawByte&0x80;   

	linear = uLawByte&0x1f;   
	linear <<= 4;   
	linear += 8;  // Add a 'half' bit (0x08) to place PCM value in middle of range   

	uLawByte &= 0x7f;   
	if(uLawByte>=0x20)   
	{   
		linear |= 0x100;  // Put in MSB   
		shift = (uLawByte>>4)-1;   
		linear <= shift;   
	}   

	return (!sign) ? (-linear) : linear; 
}

int __recv(talk_hdl *hdl, talk_frame_hdr_t *frm, void *ctx)
{
	char *pUlaw = NULL;
	int nUlawLen = 0;
	short szPcm[1024 * 2] = {0};
	int nPcmLen = 0;
    int ret;

	int lastTickCount = 0;
	//printf("@@@@@@@@@@@ TickCount:%d\n", lastTickCount = ::GetTickCount());

    printf("hdl:%p, frm => u32Magic:0x%x, u32FrameNo:%d, u32Pts:%d, u32Len:%d\n" \
            , hdl, ntohl(frm->magic), ntohl(frm->frame_num), frm->pts, ntohl(frm->frame_length));
	talk_ctx_t *c = (talk_ctx_t*)ctx;
	
#if 1
	ret = __write_audio_data((char *)frm + sizeof(talk_frame_hdr_t), ntohl(frm->frame_length));
	if(ret < 0)
	{
		printf("__write_audio_data error:%d\n", ret);
		c->close = 1;
		ASSERT0(jcu_talk_cli_close(hdl));
	}
#else
	// g711a转为pcm
	nUlawLen = ntohl(frm->u32Len);
	pUlaw = (char *)frm + sizeof(HI_TALK_FRMAE_HDR);
	for (unsigned i = 0; i < nUlawLen; ++i) 
	{
		//szPcm[i] = __linear16FromuLaw(pUlaw[i]);
		szPcm[i] = Snack_Alaw2Lin(pUlaw[i]);
	}

	// Complete delivery to the client:
	nPcmLen = nUlawLen*2;

	ret = __write_audio_data((char *)szPcm, nPcmLen);
	if(ret < 0)
	{
		printf("__write_audio_data error:%d\n", ret);
		c->close = 1;
		ASSERT0(hi_talk_cli_close(hdl));
	}
#endif
#if 0
	frm->u32Len = htonl(320);
	if ((__read_audio_data((char *)frm + sizeof(HI_TALK_FRMAE_HDR), 320)))
	{
		return 0;
	}
#endif
 //   if(c->start)
 //   {	
	//	//ADD_FRAME(frame);
 //       ret = hi_talk_cli_send(hdl, frm);
 //       printf("hi_talk_cli_send ret:%d\n", ret);
 //       if(ret < 0)
 //       {
 //           c->close = 1;
 //           ASSERT0(hi_talk_cli_close(hdl));
 //       }
 //   }

	//printf("interval:%d\n", GetTickCount() - lastTickCount);
	return 0;
}

int __start(talk_hdl *hdl, void *ctx)
{
    printf("hdl:%p, start.\n", hdl);
    talk_ctx_t *c = (talk_ctx_t*)ctx;
    c->start = 1;
	// 写入wav头
	__write_wav_header(__get_write_auido_file());
	set_user_data(hdl, (void *)&c->start);
	extern int __start_send_data(void *arg);
	printf("__start_send_data ret:%d\n", __start_send_data(hdl));
	return 0;
}
int __stop(talk_hdl *hdl, void *ctx)
{
    printf("hdl:%p, stop.\n", hdl);
    talk_ctx_t *c = (talk_ctx_t*)ctx;
    c->start = 0;
	return 0;
}

DWORD WINAPI __send_data_func(void *arg)
{
	char ulawBuf[1024] = {0};
	int ulawLen = 0;
	int pcmLen = 0;
#define MAX_AUDIO_FRAME_SIZE 160
	int ret; 
	talk_hdl *hdl = (talk_hdl *)arg;
	int *start = (int *)get_user_data(hdl);

	talk_frame_hdr_t *frm = (talk_frame_hdr_t *)calloc(1, sizeof(talk_frame_hdr_t) + 1024);
	while ((*start))
	{
		frm->magic = htonl(TALK_MAGIC);
		frm->pts = 0;
		frm->frame_num = 0;
		frm->frame_length = htonl(MAX_AUDIO_FRAME_SIZE);
		if ((__read_audio_data((char *)frm + sizeof(talk_frame_hdr_t), MAX_AUDIO_FRAME_SIZE)))
		{
			return 0;
		}
#if 0

#if 0	
		char *pAlaw = (char *)frm + sizeof(HI_TALK_FRMAE_HDR);
	/*	for (unsigned i = 0; i < MAX_AUDIO_FRAME_SIZE; ++i)
		{
			ulawBuf[i] = alaw2ulaw(pAlaw[i]);
		}*/

		short pcmBuf[1024] = {0};
		// g711a转为pcm
		//nUlawLen = ntohl(frm->u32Len);
		//char *pAlaw = (char *)frm + sizeof(HI_TALK_FRMAE_HDR);
		for (unsigned i = 0; i < MAX_AUDIO_FRAME_SIZE; ++i) 
		{
			//pcmBuf[i] = __linear16FromuLaw(pAlaw[i]);
			pcmBuf[i] = Snack_Alaw2Lin(pAlaw[i]);
		}

		pcmLen = 2 * MAX_AUDIO_FRAME_SIZE;
		memcpy((char *)frm + sizeof(HI_TALK_FRMAE_HDR), pcmBuf, pcmLen);
		frm->u32Len = htonl(pcmLen);

#else
		short pcmBuf[1024] = {0};
		g711u_Decode((unsigned char *)frm + sizeof(HI_TALK_FRMAE_HDR), (char *)pcmBuf, MAX_AUDIO_FRAME_SIZE, &pcmLen);
		memcpy((char *)frm + sizeof(HI_TALK_FRMAE_HDR), pcmBuf, pcmLen);
		frm->u32Len = htonl(pcmLen);
#endif		
	
#endif
		ret = jcu_talk_cli_send(hdl, frm);
		if(ret < 0)
		{
			printf("[err]hi_talk_cli_send ret:%d\n", ret);
			break;
		}

		Sleep(20);
	}
	
	printf("@@@@@@@@@@@@@ talk_cli_unref\n");
	talk_cli_unref(hdl);
	return 0;
}

// 创建发送数据的线程
int __start_send_data(void *arg)
{
	HANDLE hThread;
	DWORD threadId;
	talk_hdl *hdl = (talk_hdl *)arg;
	
	talk_cli_ref(hdl);
	hThread = CreateThread(NULL, 0, __send_data_func, arg, 0, &threadId);
	if (!hThread)
	{
		printf("failed to CreateThread, err = %d\n", GetLastError());
		talk_cli_unref(hdl);
		return -1;
	}
	else
	{
		printf("succeed to CreateThread, thread id = 0x%x\n", &threadId);
	}

	CloseHandle(hThread);
	return 0;
}

#ifndef WIN32
int main(int argc, char *argv[])
{
    talk_parm_t parm;
    parm.recv = __recv;
    parm.start= __start;
    parm.stop = __stop;

    printf("use args: ip, port\n");

    assert(argc == 3);

    ASSERT0(hi_talk_cli_init(&parm));

    for(;;)
    {
        printf("\n========== for =========\n");
        
        talk_ctx_t *ctx = (talk_ctx_t*)calloc(1, sizeof(talk_ctx_t));

        hi_talk_cli_t *cli = hi_talk_cli_open(argv[1], atoi(argv[2]), ctx);
        ASSERTp(cli);

        sleep(30);
        if(!ctx->close)
        {
            ASSERT0(hi_talk_cli_close(cli));
        }

        free(ctx);
        
        printf("\n========== end =========\n");
    }
    return 0;
}
#else
	 
void __set_media_info(media_info_t *mi)
{
	memset(mi, 0, sizeof(*mi));
	memcpy(mi, "DAH-DVR-20000000", strlen("DAH-DVR-20000000"));
	mi->channel = 0;
	mi->attr.samples_per_sec = SAMPLE_8K;
	mi->attr.encode_type   = ENC_G711U;
	mi->attr.audio_channel= 1;
	mi->attr.audio_bits    = 16;
}

typedef struct __talk_client_set
{
	int seq;
	talk_ctx_t *ctx;
	talk_hdl *cli_hdl;
}talk_client_set;

#define MAX_TALK_CLIENT_SIZE 20
static talk_client_set g_talk_cli_set[MAX_TALK_CLIENT_SIZE] = {0};

int test(char *ip, char *port)
{
	int *start = NULL;
	talk_hdl *cli;
	talk_ctx_t *ctx;
	media_info_t mi;
    jcu_talk_parm_t parm;
    parm.recv = __recv;
    parm.start= __start;
    parm.stop = __stop;

	WORD wVersionRequested = MAKEWORD( 2, 2 );
	WSADATA wsaData;

	int err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
		assert(FALSE);
	if (LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
	{			
		WSACleanup();
		assert(FALSE);
		return -1;
	}

	__set_media_info(&mi);
    ASSERT0(jcu_talk_cli_init(&parm));

    for(;;)
    {
        printf("\n========== for =========\n");
        
		for (int i = 0; i< MAX_TALK_CLIENT_SIZE; i++)
		{
			ctx = (talk_ctx_t*)calloc(1, sizeof(talk_ctx_t));
			{
				cli = jcu_talk_cli_open(ip, atoi(port), ctx, &mi);
			}
			ASSERTp(cli);

			g_talk_cli_set[i].seq = i;
			g_talk_cli_set[i].ctx = ctx;
			g_talk_cli_set[i].cli_hdl = cli;
			printf("[%d]jcu_talk_cli_open cli:%p\n", i, cli);
		}

		//getchar();
		Sleep(5*1000);

		for (int i = 0; i< MAX_TALK_CLIENT_SIZE; i++)
		{
			ctx = g_talk_cli_set[i].ctx;
			cli = g_talk_cli_set[i].cli_hdl;
			if(!ctx->close)
			{
				start  = (int *)get_user_data(cli);
				if (start)
				{
					*start = 0;
				}
				ASSERT0(jcu_talk_cli_close(cli));
			}
			
			//free(ctx);

			printf("[%d]jcu_talk_cli_close cli:%p\n", i, cli);
		}
        
        printf("\n========== end =========\n");
    }

    return 0;
}
#endif

int main()
{
	test("192.168.1.12", "3323");
	
	//getchar();

	WSACleanup();
	return 0;
}