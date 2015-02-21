
#include <string.h>
#include "rtspmsg.h"




void
gst_util_dump_mem (const guchar * mem, guint size)
{
  guint i, j;
  GString *string = g_string_sized_new (50);
  GString *chars = g_string_sized_new (18);

  i = j = 0;
  while (i < size) {
    if (g_ascii_isprint (mem[i]))
      g_string_append_c (chars, mem[i]);
    else
      g_string_append_c (chars, '.');

    g_string_append_printf (string, "%02x ", mem[i]);

    j++;
    i++;

    if (j == 16 || i == size) {
      g_print ("%08x (%p): %-48.48s %-16.16s\n", i - j, mem + i - j,
          string->str, chars->str);
      g_string_set_size (string, 0);
      g_string_set_size (chars, 0);
      j = 0;
    }
  }
  g_string_free (string, TRUE);
  g_string_free (chars, TRUE);
}






typedef struct _RTSPKeyValue
{
  GstRTSPHeaderField field;
  gchar *value;
} RTSPKeyValue;

static void
key_value_foreach (GArray * array, GFunc func, gpointer user_data)
{
  guint i;

  g_return_if_fail (array != NULL);

  for (i = 0; i < array->len; i++) {
    (*func) (&g_array_index (array, RTSPKeyValue, i), user_data);
  }
}

/**
 * gst_rtsp_message_new:
 * @msg: a location for the new #GstRTSPMessage
 *
 * Create a new initialized #GstRTSPMessage. Free with gst_rtsp_message_free().
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_new (GstRTSPMessage ** msg)
{
  GstRTSPMessage *newmsg;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  newmsg = g_new0 (GstRTSPMessage, 1);

  *msg = newmsg;

  return gst_rtsp_message_init (newmsg);
}

/**
 * gst_rtsp_message_init:
 * @msg: a #GstRTSPMessage
 *
 * Initialize @msg. This function is mostly used when @msg is allocated on the
 * stack. The reverse operation of this is gst_rtsp_message_unset().
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_init (GstRTSPMessage * msg)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  gst_rtsp_message_unset (msg);

  msg->type = GST_RTSP_MESSAGE_INVALID;
  msg->hdr_fields = g_array_new (FALSE, FALSE, sizeof (RTSPKeyValue));

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_get_type:
 * @msg: a #GstRTSPMessage
 *
 * Get the message type of @msg.
 *
 * Returns: the message type.
 */
GstRTSPMsgType
gst_rtsp_message_get_type (GstRTSPMessage * msg)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_MESSAGE_INVALID);

  return msg->type;
}

/**
 * gst_rtsp_message_new_request:
 * @msg: a location for the new #GstRTSPMessage
 * @method: the request method to use
 * @uri: the uri of the request
 *
 * Create a new #GstRTSPMessage with @method and @uri and store the result
 * request message in @msg. Free with gst_rtsp_message_free().
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_new_request (GstRTSPMessage ** msg, GstRTSPMethod method,
    const gchar * uri)
{
  GstRTSPMessage *newmsg;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (uri != NULL, GST_RTSP_EINVAL);

  newmsg = g_new0 (GstRTSPMessage, 1);

  *msg = newmsg;

  return gst_rtsp_message_init_request (newmsg, method, uri);
}

/**
 * gst_rtsp_message_init_request:
 * @msg: a #GstRTSPMessage
 * @method: the request method to use
 * @uri: the uri of the request
 *
 * Initialize @msg as a request message with @method and @uri. To clear @msg
 * again, use gst_rtsp_message_unset().
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_init_request (GstRTSPMessage * msg, GstRTSPMethod method,
    const gchar * uri)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (uri != NULL, GST_RTSP_EINVAL);

  gst_rtsp_message_unset (msg);

  msg->type = GST_RTSP_MESSAGE_REQUEST;
  msg->type_data.request.method = method;
  msg->type_data.request.uri = g_strdup (uri);
  msg->type_data.request.version = GST_RTSP_VERSION_1_0;
  msg->hdr_fields = g_array_new (FALSE, FALSE, sizeof (RTSPKeyValue));

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_parse_request:
 * @msg: a #GstRTSPMessage
 * @method: location to hold the method
 * @uri: location to hold the uri
 * @version: location to hold the version
 *
 * Parse the request message @msg and store the values @method, @uri and
 * @version. The result locations can be #NULL if one is not interested in its
 * value.
 *
 * @uri remains valid for as long as @msg is valid and unchanged.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_parse_request (GstRTSPMessage * msg,
    GstRTSPMethod * method, const gchar ** uri, GstRTSPVersion * version)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (msg->type == GST_RTSP_MESSAGE_REQUEST ||
      msg->type == GST_RTSP_MESSAGE_HTTP_REQUEST, GST_RTSP_EINVAL);

  if (method)
    *method = msg->type_data.request.method;
  if (uri)
    *uri = msg->type_data.request.uri;
  if (version)
    *version = msg->type_data.request.version;

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_new_response:
 * @msg: a location for the new #GstRTSPMessage
 * @code: the status code
 * @reason: the status reason or #NULL
 * @request: the request that triggered the response or #NULL
 *
 * Create a new response #GstRTSPMessage with @code and @reason and store the
 * result message in @msg. Free with gst_rtsp_message_free().
 *
 * When @reason is #NULL, the default reason for @code will be used.
 *
 * When @request is not #NULL, the relevant headers will be copied to the new
 * response message.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_new_response (GstRTSPMessage ** msg, GstRTSPStatusCode code,
    const gchar * reason, const GstRTSPMessage * request)
{
  GstRTSPMessage *newmsg;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  newmsg = g_new0 (GstRTSPMessage, 1);

  *msg = newmsg;

  return gst_rtsp_message_init_response (newmsg, code, reason, request);
}

/**
 * gst_rtsp_message_init_response:
 * @msg: a #GstRTSPMessage
 * @code: the status code
 * @reason: the status reason or #NULL
 * @request: the request that triggered the response or #NULL
 *
 * Initialize @msg with @code and @reason.
 *
 * When @reason is #NULL, the default reason for @code will be used.
 *
 * When @request is not #NULL, the relevant headers will be copied to the new
 * response message.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_init_response (GstRTSPMessage * msg, GstRTSPStatusCode code,
    const gchar * reason, const GstRTSPMessage * request)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  gst_rtsp_message_unset (msg);

  if (reason == NULL)
    reason = gst_rtsp_status_as_text (code);

  msg->type = GST_RTSP_MESSAGE_RESPONSE;
  msg->type_data.response.code = code;
  msg->type_data.response.reason = g_strdup (reason);
  msg->type_data.response.version = GST_RTSP_VERSION_1_0;
  msg->hdr_fields = g_array_new (FALSE, FALSE, sizeof (RTSPKeyValue));

  if (request) {
    if (request->type == GST_RTSP_MESSAGE_HTTP_REQUEST) {
      msg->type = GST_RTSP_MESSAGE_HTTP_RESPONSE;
      if (request->type_data.request.version != GST_RTSP_VERSION_INVALID)
        msg->type_data.response.version = request->type_data.request.version;
      else
        msg->type_data.response.version = GST_RTSP_VERSION_1_1;
    } else {
      gchar *header;

      /* copy CSEQ */
      if (gst_rtsp_message_get_header (request, GST_RTSP_HDR_CSEQ, &header,
              0) == GST_RTSP_OK) {
        gst_rtsp_message_add_header (msg, GST_RTSP_HDR_CSEQ, header);
      }

      /* copy session id */
      if (gst_rtsp_message_get_header (request, GST_RTSP_HDR_SESSION, &header,
              0) == GST_RTSP_OK) {
        char *pos;

        header = g_strdup (header);
        if ((pos = strchr (header, ';'))) {
          *pos = '\0';
        }
        g_strchomp (header);
        gst_rtsp_message_take_header (msg, GST_RTSP_HDR_SESSION, header);
      }

      /* FIXME copy more headers? */
    }
  }

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_parse_response:
 * @msg: a #GstRTSPMessage
 * @code: location to hold the status code
 * @reason: location to hold the status reason
 * @version: location to hold the version
 *
 * Parse the response message @msg and store the values @code, @reason and
 * @version. The result locations can be #NULL if one is not interested in its
 * value.
 *
 * @reason remains valid for as long as @msg is valid and unchanged.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_parse_response (GstRTSPMessage * msg,
    GstRTSPStatusCode * code, const gchar ** reason, GstRTSPVersion * version)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (msg->type == GST_RTSP_MESSAGE_RESPONSE ||
      msg->type == GST_RTSP_MESSAGE_HTTP_RESPONSE, GST_RTSP_EINVAL);

  if (code)
    *code = msg->type_data.response.code;
  if (reason)
    *reason = msg->type_data.response.reason;
  if (version)
    *version = msg->type_data.response.version;

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_new_data:
 * @msg: a location for the new #GstRTSPMessage
 * @channel: the channel
 *
 * Create a new data #GstRTSPMessage with @channel and store the
 * result message in @msg. Free with gst_rtsp_message_free().
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_new_data (GstRTSPMessage ** msg, guint8 channel)
{
  GstRTSPMessage *newmsg;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  newmsg = g_new0 (GstRTSPMessage, 1);

  *msg = newmsg;

  return gst_rtsp_message_init_data (newmsg, channel);
}

/**
 * gst_rtsp_message_init_data:
 * @msg: a #GstRTSPMessage
 * @channel: a channel
 *
 * Initialize a new data #GstRTSPMessage for @channel.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_init_data (GstRTSPMessage * msg, guint8 channel)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  gst_rtsp_message_unset (msg);

  msg->type = GST_RTSP_MESSAGE_DATA;
  msg->type_data.data.channel = channel;

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_parse_data:
 * @msg: a #GstRTSPMessage
 * @channel: location to hold the channel
 *
 * Parse the data message @msg and store the channel in @channel.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_parse_data (GstRTSPMessage * msg, guint8 * channel)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (msg->type == GST_RTSP_MESSAGE_DATA, GST_RTSP_EINVAL);

  if (channel)
    *channel = msg->type_data.data.channel;

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_unset:
 * @msg: a #GstRTSPMessage
 *
 * Unset the contents of @msg so that it becomes an uninitialized
 * #GstRTSPMessage again. This function is mostly used in combination with 
 * gst_rtsp_message_init_request(), gst_rtsp_message_init_response() and
 * gst_rtsp_message_init_data() on stack allocated #GstRTSPMessage structures.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_unset (GstRTSPMessage * msg)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  switch (msg->type) {
    case GST_RTSP_MESSAGE_INVALID:
      break;
    case GST_RTSP_MESSAGE_REQUEST:
    case GST_RTSP_MESSAGE_HTTP_REQUEST:
      g_free (msg->type_data.request.uri);
      break;
    case GST_RTSP_MESSAGE_RESPONSE:
    case GST_RTSP_MESSAGE_HTTP_RESPONSE:
      g_free (msg->type_data.response.reason);
      break;
    case GST_RTSP_MESSAGE_DATA:
      break;
    default:
      g_return_val_if_reached (GST_RTSP_EINVAL);
  }

  if (msg->hdr_fields != NULL) {
    guint i;

    for (i = 0; i < msg->hdr_fields->len; i++) {
      RTSPKeyValue *keyval = &g_array_index (msg->hdr_fields, RTSPKeyValue, i);

      g_free (keyval->value);
    }
    g_array_free (msg->hdr_fields, TRUE);
  }
  g_free (msg->body);

  memset (msg, 0, sizeof (GstRTSPMessage));

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_free:
 * @msg: a #GstRTSPMessage
 *
 * Free the memory used by @msg.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_free (GstRTSPMessage * msg)
{
  GstRTSPResult res;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  res = gst_rtsp_message_unset (msg);
  if (res == GST_RTSP_OK)
    g_free (msg);

  return res;
}

/**
 * gst_rtsp_message_take_header:
 * @msg: a #GstRTSPMessage
 * @field: a #GstRTSPHeaderField
 * @value: the value of the header
 *
 * Add a header with key @field and @value to @msg. This function takes
 * ownership of @value.
 *
 * Returns: a #GstRTSPResult.
 *
 * Since: 0.10.23
 */
GstRTSPResult
gst_rtsp_message_take_header (GstRTSPMessage * msg, GstRTSPHeaderField field,
    gchar * value)
{
  RTSPKeyValue key_value;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (value != NULL, GST_RTSP_EINVAL);

  key_value.field = field;
  key_value.value = value;

  g_array_append_val (msg->hdr_fields, key_value);

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_add_header:
 * @msg: a #GstRTSPMessage
 * @field: a #GstRTSPHeaderField
 * @value: the value of the header
 *
 * Add a header with key @field and @value to @msg. This function takes a copy
 * of @value.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_add_header (GstRTSPMessage * msg, GstRTSPHeaderField field,
    const gchar * value)
{
  return gst_rtsp_message_take_header (msg, field, g_strdup (value));
}

/**
 * gst_rtsp_message_remove_header:
 * @msg: a #GstRTSPMessage
 * @field: a #GstRTSPHeaderField
 * @indx: the index of the header
 *
 * Remove the @indx header with key @field from @msg. If @indx equals -1, all
 * headers will be removed.
 *
 * Returns: a #GstRTSPResult.
 */
GstRTSPResult
gst_rtsp_message_remove_header (GstRTSPMessage * msg, GstRTSPHeaderField field,
    gint indx)
{
  GstRTSPResult res = GST_RTSP_ENOTIMPL;
  guint i = 0;
  gint cnt = 0;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  while (i < msg->hdr_fields->len) {
    RTSPKeyValue *key_value = &g_array_index (msg->hdr_fields, RTSPKeyValue, i);

    if (key_value->field == field && (indx == -1 || cnt++ == indx)) {
      g_free (key_value->value);
      g_array_remove_index (msg->hdr_fields, i);
      res = GST_RTSP_OK;
      if (indx != -1)
        break;
    } else {
      i++;
    }
  }
  return res;
}

/**
 * gst_rtsp_message_get_header:
 * @msg: a #GstRTSPMessage
 * @field: a #GstRTSPHeaderField
 * @value: pointer to hold the result
 * @indx: the index of the header
 *
 * Get the @indx header value with key @field from @msg. The result in @value
 * stays valid as long as it remains present in @msg.
 *
 * Returns: #GST_RTSP_OK when @field was found, #GST_RTSP_ENOTIMPL if the key
 * was not found.
 */
GstRTSPResult
gst_rtsp_message_get_header (const GstRTSPMessage * msg,
    GstRTSPHeaderField field, gchar ** value, gint indx)
{
  guint i;
  gint cnt = 0;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  /* no header initialized, there are no headers */
  if (msg->hdr_fields == NULL)
    return GST_RTSP_ENOTIMPL;

  for (i = 0; i < msg->hdr_fields->len; i++) {
    RTSPKeyValue *key_value = &g_array_index (msg->hdr_fields, RTSPKeyValue, i);

    if (key_value->field == field && cnt++ == indx) {
      if (value)
        *value = key_value->value;
      return GST_RTSP_OK;
    }
  }

  return GST_RTSP_ENOTIMPL;
}

/**
 * gst_rtsp_message_append_headers:
 * @msg: a #GstRTSPMessage
 * @str: a string
 *
 * Append the currently configured headers in @msg to the #GString @str suitable
 * for transmission.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_append_headers (const GstRTSPMessage * msg, GString * str)
{
  guint i;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (str != NULL, GST_RTSP_EINVAL);

  for (i = 0; i < msg->hdr_fields->len; i++) {
    RTSPKeyValue *key_value;
    const gchar *keystr;

    key_value = &g_array_index (msg->hdr_fields, RTSPKeyValue, i);
    keystr = gst_rtsp_header_as_text (key_value->field);

    g_string_append_printf (str, "%s: %s\r\n", keystr, key_value->value);
  }
  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_set_body:
 * @msg: a #GstRTSPMessage
 * @data: the data
 * @size: the size of @data
 *
 * Set the body of @msg to a copy of @data.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_set_body (GstRTSPMessage * msg, const guint8 * data,
    guint size)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  return gst_rtsp_message_take_body (msg, g_memdup (data, size), size);
}

/**
 * gst_rtsp_message_take_body:
 * @msg: a #GstRTSPMessage
 * @data: the data
 * @size: the size of @data
 *
 * Set the body of @msg to @data and @size. This method takes ownership of
 * @data.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_take_body (GstRTSPMessage * msg, guint8 * data, guint size)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (data != NULL || size == 0, GST_RTSP_EINVAL);

  if (msg->body)
    g_free (msg->body);

  msg->body = data;
  msg->body_size = size;

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_get_body:
 * @msg: a #GstRTSPMessage
 * @data: location for the data
 * @size: location for the size of @data
 *
 * Get the body of @msg. @data remains valid for as long as @msg is valid and
 * unchanged.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_get_body (const GstRTSPMessage * msg, guint8 ** data,
    guint * size)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (data != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (size != NULL, GST_RTSP_EINVAL);

  *data = msg->body;
  *size = msg->body_size;

  return GST_RTSP_OK;
}

/**
 * gst_rtsp_message_steal_body:
 * @msg: a #GstRTSPMessage
 * @data: location for the data
 * @size: location for the size of @data
 *
 * Take the body of @msg and store it in @data and @size. After this method,
 * the body and size of @msg will be set to #NULL and 0 respectively.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_steal_body (GstRTSPMessage * msg, guint8 ** data, guint * size)
{
  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (data != NULL, GST_RTSP_EINVAL);
  g_return_val_if_fail (size != NULL, GST_RTSP_EINVAL);

  *data = msg->body;
  *size = msg->body_size;

  msg->body = NULL;
  msg->body_size = 0;

  return GST_RTSP_OK;
}

static void
dump_key_value (gpointer data, gpointer user_data G_GNUC_UNUSED)
{
  RTSPKeyValue *key_value = (RTSPKeyValue *) data;

  g_print ("   key: '%s', value: '%s'\n",
      gst_rtsp_header_as_text (key_value->field), key_value->value);
}

/**
 * gst_rtsp_message_dump:
 * @msg: a #GstRTSPMessage
 *
 * Dump the contents of @msg to stdout.
 *
 * Returns: #GST_RTSP_OK.
 */
GstRTSPResult
gst_rtsp_message_dump (GstRTSPMessage * msg)
{
  guint8 *data;
  guint size;

  g_return_val_if_fail (msg != NULL, GST_RTSP_EINVAL);

  switch (msg->type) {
    case GST_RTSP_MESSAGE_REQUEST:
      g_print ("RTSP request message %p\n", msg);
      g_print (" request line:\n");
      g_print ("   method: '%s'\n",
          gst_rtsp_method_as_text (msg->type_data.request.method));
      g_print ("   uri:    '%s'\n", msg->type_data.request.uri);
      g_print ("   version: '%s'\n",
          gst_rtsp_version_as_text (msg->type_data.request.version));
      g_print (" headers:\n");
      key_value_foreach (msg->hdr_fields, dump_key_value, NULL);
      g_print (" body:\n");
      gst_rtsp_message_get_body (msg, &data, &size);
      gst_util_dump_mem (data, size);
      break;
    case GST_RTSP_MESSAGE_RESPONSE:
      g_print ("RTSP response message %p\n", msg);
      g_print (" status line:\n");
      g_print ("   code:   '%d'\n", msg->type_data.response.code);
      g_print ("   reason: '%s'\n", msg->type_data.response.reason);
      g_print ("   version: '%s'\n",
          gst_rtsp_version_as_text (msg->type_data.response.version));
      g_print (" headers:\n");
      key_value_foreach (msg->hdr_fields, dump_key_value, NULL);
      gst_rtsp_message_get_body (msg, &data, &size);
      g_print (" body: length %d\n", size);
      gst_util_dump_mem (data, size);
      break;
    case GST_RTSP_MESSAGE_HTTP_REQUEST:
      g_print ("HTTP request message %p\n", msg);
      g_print (" request line:\n");
      g_print ("   method:  '%s'\n",
          gst_rtsp_method_as_text (msg->type_data.request.method));
      g_print ("   uri:     '%s'\n", msg->type_data.request.uri);
      g_print ("   version: '%s'\n",
          gst_rtsp_version_as_text (msg->type_data.request.version));
      g_print (" headers:\n");
      key_value_foreach (msg->hdr_fields, dump_key_value, NULL);
      g_print (" body:\n");
      gst_rtsp_message_get_body (msg, &data, &size);
      gst_util_dump_mem (data, size);
      break;
    case GST_RTSP_MESSAGE_HTTP_RESPONSE:
      g_print ("HTTP response message %p\n", msg);
      g_print (" status line:\n");
      g_print ("   code:    '%d'\n", msg->type_data.response.code);
      g_print ("   reason:  '%s'\n", msg->type_data.response.reason);
      g_print ("   version: '%s'\n",
          gst_rtsp_version_as_text (msg->type_data.response.version));
      g_print (" headers:\n");
      key_value_foreach (msg->hdr_fields, dump_key_value, NULL);
      gst_rtsp_message_get_body (msg, &data, &size);
      g_print (" body: length %d\n", size);
      gst_util_dump_mem (data, size);
      break;
    case GST_RTSP_MESSAGE_DATA:
      g_print ("RTSP data message %p\n", msg);
      g_print (" channel: '%d'\n", msg->type_data.data.channel);
      g_print (" size:    '%d'\n", msg->body_size);
      gst_rtsp_message_get_body (msg, &data, &size);
      gst_util_dump_mem (data, size);
      break;
    default:
      g_print ("unsupported message type %d\n", msg->type);
      return GST_RTSP_EINVAL;
  }
  return GST_RTSP_OK;
}

static void
gen_date_string (gchar * date_string, guint len)
{
  static const char wkdays[7][4] =
      { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
  static const char months[12][4] =
      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
    "Nov", "Dec"
  };
  struct tm tm;
  time_t t;

  time (&t);

#ifdef HAVE_GMTIME_R
  gmtime_r (&t, &tm);
#else
  tm = *gmtime (&t);
#endif

  g_snprintf (date_string, len, "%s, %02d %s %04d %02d:%02d:%02d GMT",
      wkdays[tm.tm_wday], tm.tm_mday, months[tm.tm_mon], tm.tm_year + 1900,
      tm.tm_hour, tm.tm_min, tm.tm_sec);
}


GString *
gst_rtsp_message_to_string( GstRTSPMessage  * message)
{
  GString *str = NULL;

  str = g_string_new ("");

  switch (message->type) {
    case GST_RTSP_MESSAGE_REQUEST:
      /* create request string, add CSeq */
      g_string_append_printf (str, "%s %s RTSP/1.0\r\n",
          gst_rtsp_method_as_text (message->type_data.request.method),
          message->type_data.request.uri);
      break;
    case GST_RTSP_MESSAGE_RESPONSE:
      /* create response string */
      g_string_append_printf (str, "RTSP/1.0 %d %s\r\n",
          message->type_data.response.code, message->type_data.response.reason);
      break;
    case GST_RTSP_MESSAGE_HTTP_REQUEST:
      /* create request string */
      g_string_append_printf (str, "%s %s HTTP/%s\r\n",
          gst_rtsp_method_as_text (message->type_data.request.method),
          message->type_data.request.uri,
          gst_rtsp_version_as_text (message->type_data.request.version));
      break;
    case GST_RTSP_MESSAGE_HTTP_RESPONSE:
      /* create response string */
      g_string_append_printf (str, "HTTP/%s %d %s\r\n",
          gst_rtsp_version_as_text (message->type_data.request.version),
          message->type_data.response.code, message->type_data.response.reason);
      break;
    case GST_RTSP_MESSAGE_DATA:
    {
      guint8 data_header[4];

      /* prepare data header */
      data_header[0] = '$';
      data_header[1] = message->type_data.data.channel;
      data_header[2] = (message->body_size >> 8) & 0xff;
      data_header[3] = message->body_size & 0xff;

      /* create string with header and data */
      str = g_string_append_len (str, (gchar *) data_header, 4);
      str =
          g_string_append_len (str, (gchar *) message->body,
          message->body_size);
      break;
    }
    default:
      g_string_free (str, TRUE);
      g_return_val_if_reached (NULL);
      break;
  }

  /* append headers and body */
  if (message->type != GST_RTSP_MESSAGE_DATA) {
    gchar date_string[100];

    gen_date_string (date_string, sizeof (date_string));

    /* add date header */
    gst_rtsp_message_remove_header (message, GST_RTSP_HDR_DATE, -1);
    gst_rtsp_message_add_header (message, GST_RTSP_HDR_DATE, date_string);

    /* append headers */
    gst_rtsp_message_append_headers (message, str);

    /* append Content-Length and body if needed */
    if (message->body != NULL && message->body_size > 0) {
      gchar *len;

      len = g_strdup_printf ("%d", message->body_size);
      g_string_append_printf (str, "%s: %s\r\n",
          gst_rtsp_header_as_text (GST_RTSP_HDR_CONTENT_LENGTH), len);
      g_free (len);
      /* header ends here */
      g_string_append (str, "\r\n");
      str =
          g_string_append_len (str, (gchar *) message->body,
          message->body_size);
    } else {
      /* just end headers */
      g_string_append (str, "\r\n");
    }
  }

  return str;
}


GstRTSPMessage *
gst_rtsp_message_dup_request(GstRTSPMessage *request)
{
	GstRTSPMessage *msg;
	GstRTSPMethod method;
	const gchar *uri;
	gchar *value;
	GstRTSPVersion version;
	GstRTSPHeaderField field;
	guint8 *body_data;
	guint body_size;

	if (gst_rtsp_message_get_type(request) != 
		GST_RTSP_MESSAGE_REQUEST)
		return NULL;

	if (gst_rtsp_message_parse_request(request,
		&method, &uri, &version) != GST_RTSP_OK)
		return NULL;

	if (gst_rtsp_message_new_request(&msg,
		method, uri) != GST_RTSP_OK)
		return NULL;

	for (field = GST_RTSP_HDR_INVALID + 1;
		field < GST_RTSP_HDR_LAST;
		++field)
	{
		if (gst_rtsp_message_get_header(request,
			field, &value, 0) == GST_RTSP_OK)
		{
			gst_rtsp_message_add_header(msg, field, value);
		}
	}

	if (gst_rtsp_message_get_body(request, 
		&body_data, &body_size) == GST_RTSP_OK)
	{
		gst_rtsp_message_set_body(msg, body_data, body_size);
	}

	return msg;
}

//:~ End
