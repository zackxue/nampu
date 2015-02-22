/********************************************************************
 * jpf_stream_xml.c  - deal xml of stream, parse and create xml
 * Function£ºparse or create xml relate to stream.
 * Author:yangy
 * Description:users can add parse or create message of stream,define
 *             struct about stream information
 * History:
 * 2011.06.01 - Yang Ying, initiate to create;
 * 2011.06.07 - Yang ying modify
 ********************************************************************/

#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_cmd_type.h"
#include "nmp_pu_xml.h"
