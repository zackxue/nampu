
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "rtsp_transport.h"
#include "rtsp_log.h"


#define MAX_MANAGERS	(2)

#define MAX_COPY_LEN	(max_len - copy_len)
/*
 *	if_continue 与 copy_len 需定义并初始化方可使用
 */
#define my_snprintf(str, size, args...) do {		\
	if (if_continue) {	\
		uint32 want_copy_len = snprintf(&str[copy_len], size, ##args);	\
		if (want_copy_len >= size) {	\
			copy_len += size;	\
			if_continue = 0;	\
			log_0("memory is not enough\n");	\
		} else	\
			copy_len += want_copy_len;	\
	}	\
} while(0)


typedef enum
{
  RTSP_TRANSPORT_DELIVERY = 1 << 0,     /* multicast | unicast */
  RTSP_TRANSPORT_DESTINATION = 1 << 1,
  RTSP_TRANSPORT_SOURCE = 1 << 2,
  RTSP_TRANSPORT_INTERLEAVED = 1 << 3,
  RTSP_TRANSPORT_APPEND = 1 << 4,
  RTSP_TRANSPORT_TTL = 1 << 5,
  RTSP_TRANSPORT_LAYERS = 1 << 6,
  RTSP_TRANSPORT_PORT = 1 << 7,
  RTSP_TRANSPORT_CLIENT_PORT = 1 << 8,
  RTSP_TRANSPORT_SERVER_PORT = 1 << 9,
  RTSP_TRANSPORT_SSRC = 1 << 10,
  RTSP_TRANSPORT_MODE = 1 << 11,
} rtsp_transport_parameter;


typedef struct
{
	char *name;
	const RTSP_TRANS_MODE mode;
	const char *gst_mime;
	const char *manager[MAX_MANAGERS];
} rtsp_trans_map;

static const rtsp_trans_map transports[] = {
	{"rtp", RTSP_TRANS_RTP, "application/x-rtp", {"gstrtpbin", "rtpdec"}},
	{NULL, RTSP_TRANS_UNKNOWN, NULL, {NULL, NULL}}
};


typedef struct
{
	const char *name;
	const RTSP_PROFILE profile;
} rtsp_profile_map;

static const rtsp_profile_map profiles[] = {
	{"avp", RTSP_PROFILE_AVP},
	{"savp", RTSP_PROFILE_SAVP},
	{NULL, RTSP_PROFILE_UNKNOWN}
};


typedef struct
{
	const char *name;
	const RTSP_LOWER_TRANS ltrans;
} rtsp_ltrans_map;

static const rtsp_ltrans_map ltrans[] = {
	{"udp", RTSP_LOWER_TRANS_UDP},
	{"mcast", RTSP_LOWER_TRANS_UDP_MCAST},
	{"tcp", RTSP_LOWER_TRANS_TCP},
	{NULL, RTSP_LOWER_TRANS_UNKNOWN}
};


#define RTSP_TRANSPORT_PARAMETER_IS_UNIQUE(param) \
do {                                    \
  if ((transport_params & (param)) != 0)          \
    goto invalid_transport;                       \
  transport_params |= (param);                    \
} while(0)


RTSP_RESULT rtsp_transport_new(rtsp_transport **transport)
{
	rtsp_transport *trans;

	return_val_if_fail(transport != NULL, RTSP_EINVAL);

	trans = (rtsp_transport *)my_alloc(sizeof(rtsp_transport));
	memset(trans, 0, sizeof(rtsp_transport));

	*transport = trans;

	return rtsp_transport_init(trans);
}


RTSP_RESULT rtsp_transport_init(rtsp_transport *transport)
{
	return_val_if_fail(transport != NULL, RTSP_EINVAL);

	if (transport->destination)
		my_free(transport->destination, transport->destination_size);
	if (transport->source)
		my_free(transport->source, transport->source_size);

	memset(transport, 0, sizeof(rtsp_transport));

	transport->trans = RTSP_TRANS_RTP;
	transport->profile = RTSP_PROFILE_AVP;
	transport->lower_transport = RTSP_LOWER_TRANS_UDP_MCAST;
	transport->mode_play = 1;
	transport->mode_record = 0;
	transport->interleaved.min = -1;
	transport->interleaved.max = -1;
	transport->port.min = -1;
	transport->port.max = -1;
	transport->client_port.min = -1;
	transport->client_port.max = -1;
	transport->server_port.min = -1;
	transport->server_port.max = -1;

	return RTSP_OK;
}


static void parse_mode(rtsp_transport *transport, const char *str)
{
	transport->mode_play = (strstr (str, "play") != NULL);
	transport->mode_record = (strstr (str, "record") != NULL);
}


static mbool check_range(const char *str, char **tmp, int *range)
{
	long range_val;

	range_val = strtol(str, tmp, 10);
	if (range_val >= MIN_INT32 && range_val <= MAX_INT32) {
		*range = range_val;
		return 1;
	} else {
		return 0;
	}
}

static mbool parse_range(const char *str, rtsp_range *range)
{
	char *minus;
	char *tmp;

	if (isspace(*str) || *str == '+' || *str == '-')
		goto invalid_range;

	minus = strstr(str, "-");
	if (minus) {
		if (isspace(minus[1]) || minus[1] == '+' || minus[1] == '-')
			goto invalid_range;

		if (!check_range(str, &tmp, &range->min) || str == tmp || tmp != minus)
			goto invalid_range;

		if (!check_range(minus + 1, &tmp, &range->max) || (*tmp && *tmp != ';'))
			goto invalid_range;
	} else {
		if (!check_range(str, &tmp, &range->min) || str == tmp || 
			(*tmp && *tmp != ';'))
			goto invalid_range;

		range->max = -1;
	}

	return 1;

invalid_range:
	{
		range->min = -1;
		range->max = -1;
		return 0;
	}
}


static void range_as_text(const rtsp_range *range, char *str, size_t str_size)
{
	if (str == NULL)
		return ;
	if (str_size < 24)
		goto invalid;
	if (range->min < 0)
		goto invalid;
	else if(range->max < 0)
		snprintf(str, str_size, "%d", range->min);
	else
		snprintf(str, str_size, "%d-%d", range->min, range->max);
	return ;

invalid:
	str[0] = '\0';
	return ;
}

static const char *rtsp_transport_mode_as_text(const rtsp_transport *transport)
{
	int i;

	for (i = 0; transports[i].name; i++)
		if (transports[i].mode == transport->trans)
			return transports[i].name;

	return NULL;
}

static const char *rtsp_transport_profile_as_text(const rtsp_transport *transport)
{
	int i;

	for (i = 0; profiles[i].name; i++)
		if (profiles[i].profile == transport->profile)
			return profiles[i].name;

	return NULL;
}

static const char *rtsp_transport_ltrans_as_text(const rtsp_transport *transport)
{
	int i;

	/* need to special case RTSP_LOWER_TRANS_UDP_MCAST */
	if (transport->lower_transport == RTSP_LOWER_TRANS_UDP_MCAST)
		return "udp";

	for (i = 0; ltrans[i].name; i++)
		if (ltrans[i].ltrans == transport->lower_transport)
			return ltrans[i].name;

	return NULL;
}


#define IS_VALID_PORT_RANGE(range) \
	(range.min >= 0 && range.min < 65536 && range.max < 65536)

#define IS_VALID_INTERLEAVE_RANGE(range) \
	(range.min >= 0 && range.min < 256 && range.max < 256)

#define GOTO_INVALID_TRANSPORT	{log_0("invalid\n"); goto invalid_transport;}

RTSP_RESULT rtsp_transport_parse(const char *str, rtsp_transport *transport)
{
	char **split, *down, **transp = NULL;
	uint32 transport_params = 0;
	int i, count;

	return_val_if_fail(transport != NULL, RTSP_EINVAL);
	return_val_if_fail(str != NULL, RTSP_EINVAL);

	rtsp_transport_init(transport);

	/* case insensitive */
	down = my_strdown(str, -1);

	split = my_strsplit(down, ";", -1);
	my_free(down, STRLEN1(down));

	/* First field contains the transport/profile/lower_transport */
	if (split[0] == NULL)
		GOTO_INVALID_TRANSPORT;

	transp = my_strsplit(split[0], "/", -1);

	if (transp[0] == NULL || transp[1] == NULL)
		GOTO_INVALID_TRANSPORT;

	for (i = 0; transports[i].name; i++)
		if (strcmp(transp[0], transports[i].name) == 0)
			break;
	transport->trans = transports[i].mode;

	for (i = 0; profiles[i].name; i++)
		if (strcmp(transp[1], profiles[i].name) == 0)
			break;
	transport->profile = profiles[i].profile;
	count = 2;

	if (transp[count] != NULL) {
		for (i = 0; ltrans[i].name; i++)
			if (strcmp(transp[count], ltrans[i].name) == 0)
				break;
		transport->lower_transport = ltrans[i].ltrans;
	} else {
		/* specifying the lower transport is optional */
		if (transport->trans == RTSP_TRANS_RTP &&
			transport->profile == RTSP_PROFILE_AVP)
			transport->lower_transport = RTSP_LOWER_TRANS_UDP_MCAST;
		else
			transport->lower_transport = RTSP_LOWER_TRANS_UNKNOWN;
	}

	my_strfreev(transp);
	transp = NULL;

	if (transport->trans == RTSP_TRANS_UNKNOWN ||
		transport->profile == RTSP_PROFILE_UNKNOWN ||
		transport->lower_transport == RTSP_LOWER_TRANS_UNKNOWN)
	goto unsupported_transport;

	i = 1;
	while (split[i]) {
		if (strcmp (split[i], "multicast") == 0) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_DELIVERY);
			if (transport->lower_transport == RTSP_LOWER_TRANS_TCP)
				GOTO_INVALID_TRANSPORT;
			transport->lower_transport = RTSP_LOWER_TRANS_UDP_MCAST;
		} else if (strcmp (split[i], "unicast") == 0) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_DELIVERY);
			if (transport->lower_transport == RTSP_LOWER_TRANS_UDP_MCAST)
				transport->lower_transport = RTSP_LOWER_TRANS_UDP;
		} else if (my_str_has_prefix (split[i], "destination=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_DESTINATION);
			transport->destination = my_strdup (split[i] + 12);
			transport->destination_size = STRLEN1(split[i] + 12);
		} else if (my_str_has_prefix (split[i], "source=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_SOURCE);
			transport->source = my_strdup (split[i] + 7);
			transport->source_size = STRLEN1(split[i] + 7);
		} else if (my_str_has_prefix (split[i], "layers=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_LAYERS);
			transport->layers = strtoul (split[i] + 7, NULL, 10);
		} else if (my_str_has_prefix (split[i], "mode=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_MODE);
			parse_mode (transport, split[i] + 5);
			if (!transport->mode_play && !transport->mode_record)
				GOTO_INVALID_TRANSPORT;
		} else if (strcmp (split[i], "append") == 0) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_APPEND);
			transport->append = 1;
		} else if (my_str_has_prefix (split[i], "interleaved=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_INTERLEAVED);
			parse_range (split[i] + 12, &transport->interleaved);
			if (!IS_VALID_INTERLEAVE_RANGE (transport->interleaved))
				GOTO_INVALID_TRANSPORT;
		} else if (my_str_has_prefix (split[i], "ttl=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_TTL);
			transport->ttl = strtoul (split[i] + 4, NULL, 10);
			if (transport->ttl >= 256)
				GOTO_INVALID_TRANSPORT;
		} else if (my_str_has_prefix (split[i], "port=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_PORT);
			if (parse_range (split[i] + 5, &transport->port)) {
				if (!IS_VALID_PORT_RANGE (transport->port))
					GOTO_INVALID_TRANSPORT;
			}
		} else if (my_str_has_prefix (split[i], "client_port=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_CLIENT_PORT);
			if (parse_range (split[i] + 12, &transport->client_port)) {
				if (!IS_VALID_PORT_RANGE (transport->client_port))
					GOTO_INVALID_TRANSPORT;
			}
		} else if (my_str_has_prefix (split[i], "server_port=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_SERVER_PORT);
			if (parse_range (split[i] + 12, &transport->server_port)) {
				if (!IS_VALID_PORT_RANGE (transport->server_port))
					GOTO_INVALID_TRANSPORT;
			}
		} else if (my_str_has_prefix (split[i], "ssrc=")) {
			RTSP_TRANSPORT_PARAMETER_IS_UNIQUE (RTSP_TRANSPORT_SSRC);
			transport->ssrc = strtoul (split[i] + 5, NULL, 16);
		} else {
			/* unknown field... */
			log_0 ("unknown transport field \"%s\"", split[i]);
		}
		i++;
	}
	my_strfreev(split);

	return RTSP_OK;

unsupported_transport:
	{
		my_strfreev(split);
		return RTSP_ERROR;
	}

invalid_transport:
	{
		log_0("goto invalid_transport\n");
		my_strfreev(transp);
		my_strfreev(split);
		return RTSP_EINVAL;
	}
}


int rtsp_transport_as_text(rtsp_transport *transport, char *str, uint32 str_size)
{
	/* 定义、初始化if_continue, copy_len 和 max_len */
	char if_continue = 1;
	int copy_len = 0;
	int max_len = str_size;

	const char *tmp;
	char tmp_str[25] = {0};

	return_val_if_fail(transport != NULL, -1);

	/* add the transport specifier */
	if ((tmp = rtsp_transport_mode_as_text(transport)) == NULL)
		GOTO_INVALID_TRANSPORT;
	my_snprintf(str, MAX_COPY_LEN, "%s", tmp);

	if ((tmp = rtsp_transport_profile_as_text(transport)) == NULL)
		GOTO_INVALID_TRANSPORT;
	my_snprintf(str, MAX_COPY_LEN, "/%s", tmp);

	if (transport->trans != RTSP_TRANS_RTP ||
		transport->profile != RTSP_PROFILE_AVP ||
		transport->lower_transport == RTSP_LOWER_TRANS_TCP) {
		if ((tmp = rtsp_transport_ltrans_as_text(transport)) == NULL)
			GOTO_INVALID_TRANSPORT;
		my_snprintf(str, MAX_COPY_LEN, "/%s", tmp);
	}

	 /* add the unicast/multicast parameter */
	 if (transport->lower_transport == RTSP_LOWER_TRANS_UDP_MCAST) {
	 	my_snprintf(str, MAX_COPY_LEN, ";multicast");
	 }
	 else
	 	my_snprintf(str, MAX_COPY_LEN, ";unicast");

	 /* add the destination parameter */
	 if (transport->destination != NULL)
		my_snprintf(str, MAX_COPY_LEN, ";destination=%s", transport->destination);

	 /* add the source parameter */
	if (transport->source != NULL) 
		my_snprintf(str, MAX_COPY_LEN, ";source=%s", transport->source);

	/* add the interleaved parameter */
	if (transport->lower_transport == RTSP_LOWER_TRANS_TCP && 
		transport->interleaved.min >= 0) {
		if (transport->interleaved.min < 256 && transport->interleaved.max < 256) {
			range_as_text(&transport->interleaved, tmp_str, sizeof(tmp_str));
			my_snprintf(str, MAX_COPY_LEN, ";interleaved=%s", tmp_str);
		} else
			GOTO_INVALID_TRANSPORT;
	}

	/* add the append parameter */
	if (transport->mode_record && transport->append)
		my_snprintf(str, MAX_COPY_LEN, ";append");

	/* add the ttl parameter */
	if (transport->lower_transport == RTSP_LOWER_TRANS_UDP_MCAST && 
		transport->ttl != 0) {
		if (transport->ttl < 256) {
			my_snprintf(str, MAX_COPY_LEN, ";ttl=%u", transport->ttl);
		}
		else
			GOTO_INVALID_TRANSPORT;
	}

	/* add the layers parameter */
	if (transport->layers != 0)
		my_snprintf(str, MAX_COPY_LEN, ";layers=%u", transport->layers);

	/* add the port parameter */
	if (transport->lower_transport != RTSP_LOWER_TRANS_TCP) {
		if (transport->trans == RTSP_TRANS_RTP && transport->port.min >= 0) {
			if (transport->port.min < 65536 && transport->port.max < 65536) {
				range_as_text(&transport->port, tmp_str, sizeof(tmp_str));
				my_snprintf(str, MAX_COPY_LEN, ";port=%s", tmp_str);
			} else
			GOTO_INVALID_TRANSPORT;
		}

		/* add the client_port parameter */
		if (transport->trans == RTSP_TRANS_RTP
			&& transport->client_port.min >= 0) {
			if (transport->client_port.min < 65536
				&& transport->client_port.max < 65536) {
				range_as_text(&transport->client_port, tmp_str, sizeof(tmp_str));
				my_snprintf(str, MAX_COPY_LEN, ";client_port=%s", tmp_str);
			} else
			GOTO_INVALID_TRANSPORT;
		}

		/* add the server_port parameter */
		if (transport->trans == RTSP_TRANS_RTP
		&& transport->server_port.min >= 0) {
			if (transport->server_port.min < 65536
				&& transport->server_port.max < 65536) {
				range_as_text(&transport->server_port, tmp_str, sizeof(tmp_str));
				my_snprintf(str, MAX_COPY_LEN, ";server_port=%s", tmp_str);
			} else
			GOTO_INVALID_TRANSPORT;
		}
	}

	/* add the ssrc parameter */
	if (transport->lower_transport != RTSP_LOWER_TRANS_UDP_MCAST &&
		transport->ssrc != 0) {
		my_snprintf(str, MAX_COPY_LEN, ";ssrc=%08X", transport->ssrc);
	}

	/* add the mode parameter */
	if (transport->mode_play && transport->mode_record) {
		my_snprintf(str, MAX_COPY_LEN, ";mode=\"PLAY,RECORD\"");
	}
	else if (transport->mode_record) {
		my_snprintf(str, MAX_COPY_LEN, ";mode=\"RECORD\"");
	}
	else if (transport->mode_play)
		my_snprintf(str, MAX_COPY_LEN, ";mode=\"PLAY\"");

	if (copy_len < str_size)
	{
		str[copy_len] = '\0';
		return copy_len;
	}
	else
	{
		str[str_size - 1] = '\0';
		log_0("memory is not enough\n");
		return str_size;
	}

invalid_transport:
	{
		log_0("goto invalid_transport\n");
		return 0;
	}
}

RTSP_RESULT rtsp_transport_get_mime(RTSP_TRANS_MODE trans, const char **mime)
{
	int i;

	return_val_if_fail(mime != NULL, RTSP_EINVAL);

	for (i = 0; transports[i].name; i++)
		if (transports[i].mode == trans)
			break;
	*mime = transports[i].gst_mime;

	return RTSP_OK;
}


RTSP_RESULT rtsp_transport_get_manager(RTSP_TRANS_MODE trans, 
	const char **manager, uint32 option)
{
	int i;

	return_val_if_fail(manager != NULL, RTSP_EINVAL);

	for (i = 0; transports[i].name; i++)
		if (transports[i].mode == trans)
			break;

	if (option < MAX_MANAGERS)
		*manager = transports[i].manager[option];
	else
		*manager = NULL;

	return RTSP_OK;
}


RTSP_RESULT rtsp_transport_free(rtsp_transport *transport)
{
	return_val_if_fail(transport != NULL, RTSP_EINVAL);

	rtsp_transport_init(transport);
	my_free(transport, sizeof(rtsp_transport));

	return RTSP_OK;
}

