#include <stdio.h>
#include <string.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <assert.h>
#include <error.h>
#include "nmp_xml_fun.h"
#include "nmp_xml_shared.h"
#include "nmp_cmd_type.h"
#include "nmp_debug.h"
#include "nmp_share_struct.h"

void
jpf_get_weekday(xmlDocPtr doc, char *weekpath,JpfWeekday *weekdays,int *weekday_num)
{
    xmlXPathObjectPtr app_result;
    xmlNodePtr cur, node, node1;

    char *day = NULL;
    char *enable = NULL;
    int i, timeseg_num = 0;

    app_result =  jpf_get_node(doc, (const xmlChar *)weekpath);
    if (app_result == NULL)
    {
        jpf_warning("get weekday xml node:%s error",weekpath);
        return;
    }

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    *weekday_num = nodeset->nodeNr;
    if (*weekday_num >= WEEKDAYS)
	*weekday_num = WEEKDAYS;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"weekDay")))
        {
            day = (char *)xmlGetProp(cur, (const xmlChar *)"Id");
            if (day)
            {
                weekdays[i].weekday = atoi(day);
                xmlFree(day);
            }
            node = cur->xmlChildrenNode;
            while (node != NULL)
            {
                if ((!xmlStrcmp(node->name, (const xmlChar *)"timeSegs")))
                {
                    node1 = node->xmlChildrenNode;
                    while (node1 != NULL)
                    {
                        if ((!xmlStrcmp(node1->name, (const xmlChar *)"timeSeg")))
                        {
                            if (timeseg_num >= TIME_SEG_NUM)
				    break;
                            enable = (char *)xmlGetProp(node1, (const xmlChar *)"enable");
                            if(enable)
                          	{
                                weekdays[i].time_segs[timeseg_num].seg_enable = atoi(enable);
                                xmlFree(enable);
                           	}
                            jpf_deal_text(
                                doc, node1,
                                weekdays[i].time_segs[timeseg_num].time_seg,
                                TIME_SEG_LEN
                            );

                            timeseg_num++;
                        }
                        node1 = node1->next;
                    }

                    weekdays[i].time_seg_num = timeseg_num;
                    timeseg_num = 0;
                }

                node = node->next;
           }
        }
        else
            xml_warning("Warning, not parse the node %s \n", cur->name);
    }

    xmlXPathFreeObject(app_result);
}


void
jpf_set_weekday(xmlNodePtr node, JpfWeekday *weekdays, int weekday_num)
{
xmlNodePtr content, node1, node2, node3;
    char str[INT_TO_CHAR_LEN] = {0};
	int i = 0, j, time_seg_num;

    while(i < weekday_num)
    {
        node1 = xmlNewNode(NULL, BAD_CAST "weekDay");
        snprintf(str, INT_TO_CHAR_LEN,  "%d", weekdays[i].weekday);
        xmlNewProp(node1, BAD_CAST "Id",BAD_CAST str);
        xmlAddChild(node, node1);
        node2 = xmlNewNode(NULL, BAD_CAST "timeSegs");
        xmlAddChild(node1, node2);
        time_seg_num = weekdays[i].time_seg_num;
        j = 0;
        while(j < time_seg_num)
        {
            node3 = xmlNewNode(NULL, BAD_CAST "timeSeg");
            content = xmlNewText(BAD_CAST weekdays[i].time_segs[j].time_seg);
            xmlAddChild(node2, node3);
            xmlAddChild(node3, content);
            snprintf(str, INT_TO_CHAR_LEN,  "%d", weekdays[i].time_segs[j].seg_enable);
            xmlNewProp(node3, BAD_CAST "enable",BAD_CAST str);
            j++;
        }
        i++;
    }
}


void jpf_get_time_policy(xmlDocPtr doc, xmlNodePtr root_node, JpfActionPolicy *time_policy)
{
    xmlNodePtr cur, node;
    char *day = NULL;
    int i = 0, timeseg_num = 0;

    cur = root_node->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"week")))
        {
            day = (char *)xmlGetProp(cur, (const xmlChar *)"id");
            if (day)
            {
                time_policy->weekdays[i].weekday = atoi(day);
                xmlFree(day);
            }
            node = cur->xmlChildrenNode;

            while (node != NULL)
            {
                if ((!xmlStrcmp(node->name, (const xmlChar *)"timeSeg")))
                {
                    if (timeseg_num >= TIME_SEG_NUM)
		           break;
                    jpf_deal_text(
                        doc, node,
                        time_policy->weekdays[i].time_segs[timeseg_num].time_seg,
                        TIME_SEG_LEN
                    );

                    timeseg_num++;
                }
                node = node->next;
            }

            time_policy->weekdays[i].time_seg_num = timeseg_num;
            timeseg_num = 0;
            i++;
         }
          else
            xml_warning("Warning, not parse the node %s \n", cur->name);

         cur = cur->next;
     }

     time_policy->weekday_num = i;
}



void
jpf_xml_parse_time_policy(char *time_policy, void *res_info)
{
	gint xml_len;
	xmlNodePtr root_node = NULL;
	xmlDocPtr doc_str;

	xml_len = strlen(time_policy);
	if (xml_len != 0)
	{
		doc_str =xmlParseMemory(time_policy,xml_len);
		if (doc_str == NULL )
		{
			printf("Document not parsed successfully. \n");
			return;
		}

		root_node = xmlDocGetRootElement(doc_str); //确定文档根元素
		if (root_node == NULL)
		{
			xml_error("empty document\n");
			xmlFreeDoc(doc_str);
			return;
		}

		jpf_get_time_policy(doc_str, root_node, (JpfActionPolicy *)res_info);

		xmlFreeDoc(doc_str);
	}
}



void
jpf_get_rectarea(xmlDocPtr doc, char *rectpath, JpfRectangle *detect_area, int *rect_num)
{
    xmlXPathObjectPtr app_result;
    xmlNodePtr cur, node;
    int i;

    app_result =  jpf_get_node(doc, (const xmlChar *)rectpath);
    if (app_result == NULL)
    {
        jpf_warning("get rectarea xml node:%s error",rectpath);
        return;
    }

    xmlNodeSetPtr nodeset = app_result->nodesetval;
    *rect_num = nodeset->nodeNr;
    for (i=0; i < nodeset->nodeNr; i++)
    {
        cur = nodeset->nodeTab[i];
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"rect")))
        {
            node = cur->xmlChildrenNode;
            while (node != NULL)
            {
                if ((!xmlStrcmp(node->name, (const xmlChar *)"left")))
                    jpf_deal_value(doc, node, &detect_area[i].left);
                else if ((!xmlStrcmp(node->name, (const xmlChar *)"top")))
                    jpf_deal_value(doc, node, &detect_area[i].top);
                else if ((!xmlStrcmp(node->name, (const xmlChar *)"right")))
                    jpf_deal_value(doc, node, &detect_area[i].right);
                else if ((!xmlStrcmp(node->name, (const xmlChar *)"bottom")))
                    jpf_deal_value(doc, node, &detect_area[i].bottom);

                node = node->next;
            }
        }
        else
            xml_warning("Warning, not parse the node %s \n", cur->name);

    }

    xmlXPathFreeObject(app_result);
}


void
jpf_set_rectarea(xmlNodePtr node, JpfRectangle *detect_area, int rect_num)
{
    xmlNodePtr node1;
    char str[INT_TO_CHAR_LEN] = {0};
    int i = 0;

    while(i < rect_num)
    {
        node1 = xmlNewNode(NULL, BAD_CAST "rect");
        xmlAddChild(node, node1);
        printf("-------rect_area->left=%d\n",detect_area[i].left);
        snprintf(str, INT_TO_CHAR_LEN,  "%d", detect_area[i].left);
        xmlNewChild(
            node1,
            NULL,
            BAD_CAST "left",
            BAD_CAST str
        );
        snprintf(str, INT_TO_CHAR_LEN,  "%d", detect_area[i].top);
        xmlNewChild(
            node1,
            NULL,
            BAD_CAST "top",
            BAD_CAST str
        );
        snprintf(str, INT_TO_CHAR_LEN,  "%d", detect_area[i].right);
        xmlNewChild(
            node1,
            NULL,
            BAD_CAST "right",
            BAD_CAST str
        );
        snprintf(str, INT_TO_CHAR_LEN,  "%d", detect_area[i].bottom);
        xmlNewChild(
            node1,
            NULL,
            BAD_CAST "bottom",
            BAD_CAST str
        );
        i++;
    }
}


void jpf_create_ptz_para(xmlNodePtr parent_node, JpfPtzPara para)
{
    char str[INT_TO_CHAR_LEN] = {0};
    xmlNodePtr node;

    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.serial_no);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "serialNo",
        BAD_CAST str);

    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.ptz_protocol);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "ptzProt",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.ptz_address);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "ptzAddr",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.baud_rate);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "baudRate",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.data_bit);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "dataBit",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.stop_bit);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "stopBit",
        BAD_CAST str);
    snprintf(str, INT_TO_CHAR_LEN,  "%d", para.verify);
    node = xmlNewChild(parent_node,
        NULL,
        BAD_CAST "verify",
        BAD_CAST str);
}


void jpf_parse_ptz_para(xmlDocPtr doc,xmlNodePtr cur, JpfPtzPara *para)
{
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"serialNo")))
        jpf_deal_value(doc, cur, &para->serial_no);
    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ptzProt")))
        jpf_deal_value(doc, cur, &para->ptz_protocol);
    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ptzAddr")))
        jpf_deal_value(doc, cur, &para->ptz_address);
    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"baudRate")))
        jpf_deal_value(doc, cur, &para->baud_rate);
    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"dataBit")))
        jpf_deal_value(doc, cur, &para->data_bit);
    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stopBit")))
        jpf_deal_value(doc, cur, &para->stop_bit);
    else if ((!xmlStrcmp(cur->name, (const xmlChar *)"verify")))
        jpf_deal_value(doc, cur, &para->verify);
    else
        xml_warning("Warning, not parse the node %s \n", cur->name);
}


