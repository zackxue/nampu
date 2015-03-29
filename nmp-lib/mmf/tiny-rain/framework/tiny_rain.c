/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#include "tiny_rain.h"

extern void init_log_facility( void );
extern int32_t init_timer_facility( void );
extern void init_media_facility( void );
extern void init_memblock_facility( void );

void tiny_rain_init( void )
{
    static int32_t init = 0;

    if (!init)  
    {   /* not thread-safe */
        init = 1;
        init_log_facility();
        init_timer_facility();
        init_media_facility();
        init_memblock_facility();      
    }
}


//:~ End
