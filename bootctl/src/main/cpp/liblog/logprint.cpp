/*
**
** Copyright 2006-2014, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef __MINGW32__
#define HAVE_STRSEP
#endif

#include <log/logprint.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#ifndef __MINGW32__
#include <pwd.h>
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <wchar.h>

#include <cutils/list.h>

#include <log/log.h>
#include <log/log_read.h>
#include <private/android_logger.h>

#define MS_PER_NSEC 1000000
#define US_PER_NSEC 1000

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

typedef struct FilterInfo_t {
  char* mTag;
  android_LogPriority mPri;
  struct FilterInfo_t* p_next;
} FilterInfo;

struct AndroidLogFormat_t {
  android_LogPriority global_pri;
  FilterInfo* filters;
  AndroidLogPrintFormat format;
  bool colored_output;
  bool usec_time_output;
  bool nsec_time_output;
  bool printable_output;
  bool year_output;
  bool zone_output;
  bool epoch_output;
  bool monotonic_output;
  bool uid_output;
  bool descriptive_output;
};

/*
 * API issues prevent us from exposing "descriptive" in AndroidLogFormat_t
 * during android_log_processBinaryLogBuffer(), so we break layering.
 */
static bool descriptive_output = false;

/*
 * 8-bit color tags. See ECMA-48 Set Graphics Rendition in
 * [console_codes(4)](https://man7.org/linux/man-pages/man4/console_codes.4.html).
 *
 * The text manipulation character stream is defined as:
 *   ESC [ <parameter #> m
 *
 * We use "set <color> foreground" escape sequences instead of
 * "256/24-bit foreground color". This allows colors to render
 * according to user preferences in terminal emulator settings
 */
#define ANDROID_COLOR_BLUE 34
#define ANDROID_COLOR_DEFAULT 39
#define ANDROID_COLOR_GREEN 32
#define ANDROID_COLOR_RED 31
#define ANDROID_COLOR_YELLOW 33

static FilterInfo* filterinfo_new(const char* tag, android_LogPriority pri) {
  FilterInfo* p_ret;

  p_ret = (FilterInfo*)calloc(1, sizeof(FilterInfo));
  p_ret->mTag = strdup(tag);
  p_ret->mPri = pri;

  return p_ret;
}

/* balance to above, filterinfo_free left unimplemented */

/*
 * Note: also accepts 0-9 priorities
 * returns ANDROID_LOG_UNKNOWN if the character is unrecognized
 */
static android_LogPriority filterCharToPri(char c) {
  android_LogPriority pri;

  c = tolower(c);

  if (c >= '0' && c <= '9') {
    if (c >= ('0' + ANDROID_LOG_SILENT)) {
      pri = ANDROID_LOG_VERBOSE;
    } else {
      pri = (android_LogPriority)(c - '0');
    }
  } else if (c == 'v') {
    pri = ANDROID_LOG_VERBOSE;
  } else if (c == 'd') {
    pri = ANDROID_LOG_DEBUG;
  } else if (c == 'i') {
    pri = ANDROID_LOG_INFO;
  } else if (c == 'w') {
    pri = ANDROID_LOG_WARN;
  } else if (c == 'e') {
    pri = ANDROID_LOG_ERROR;
  } else if (c == 'f') {
    pri = ANDROID_LOG_FATAL;
  } else if (c == 's') {
    pri = ANDROID_LOG_SILENT;
  } else if (c == '*') {
    pri = ANDROID_LOG_DEFAULT;
  } else {
    pri = ANDROID_LOG_UNKNOWN;
  }

  return pri;
}

static char filterPriToChar(android_LogPriority pri) {
  switch (pri) {
    /* clang-format off */
    case ANDROID_LOG_VERBOSE: return 'V';
    case ANDROID_LOG_DEBUG:   return 'D';
    case ANDROID_LOG_INFO:    return 'I';
    case ANDROID_LOG_WARN:    return 'W';
    case ANDROID_LOG_ERROR:   return 'E';
    case ANDROID_LOG_FATAL:   return 'F';
    case ANDROID_LOG_SILENT:  return 'S';

    case ANDROID_LOG_DEFAULT:
    case ANDROID_LOG_UNKNOWN:
    default:                  return '?';
      /* clang-format on */
  }
}

static int colorFromPri(android_LogPriority pri) {
  switch (pri) {
    /* clang-format off */
    case ANDROID_LOG_VERBOSE: return ANDROID_COLOR_DEFAULT;
    case ANDROID_LOG_DEBUG:   return ANDROID_COLOR_BLUE;
    case ANDROID_LOG_INFO:    return ANDROID_COLOR_GREEN;
    case ANDROID_LOG_WARN:    return ANDROID_COLOR_YELLOW;
    case ANDROID_LOG_ERROR:   return ANDROID_COLOR_RED;
    case ANDROID_LOG_FATAL:   return ANDROID_COLOR_RED;
    case ANDROID_LOG_SILENT:  return ANDROID_COLOR_DEFAULT;

    case ANDROID_LOG_DEFAULT:
    case ANDROID_LOG_UNKNOWN:
    default:                  return ANDROID_COLOR_DEFAULT;
      /* clang-format on */
  }
}

static android_LogPriority filterPriForTag(AndroidLogFormat* p_format, const char* tag) {
  FilterInfo* p_curFilter;

  for (p_curFilter = p_format->filters; p_curFilter != NULL; p_curFilter = p_curFilter->p_next) {
    if (0 == strcmp(tag, p_curFilter->mTag)) {
      if (p_curFilter->mPri == ANDROID_LOG_DEFAULT) {
        return p_format->global_pri;
      } else {
        return p_curFilter->mPri;
      }
    }
  }

  return p_format->global_pri;
}

/**
 * returns 1 if this log line should be printed based on its priority
 * and tag, and 0 if it should not
 */
int android_log_shouldPrintLine(AndroidLogFormat* p_format, const char* tag,
                                android_LogPriority pri) {
  return pri >= filterPriForTag(p_format, tag);
}

AndroidLogFormat* android_log_format_new() {
  AndroidLogFormat* p_ret;

  p_ret = static_cast<AndroidLogFormat*>(calloc(1, sizeof(AndroidLogFormat)));

  p_ret->global_pri = ANDROID_LOG_VERBOSE;
  p_ret->format = FORMAT_BRIEF;
  p_ret->colored_output = false;
  p_ret->usec_time_output = false;
  p_ret->nsec_time_output = false;
  p_ret->printable_output = false;
  p_ret->year_output = false;
  p_ret->zone_output = false;
  p_ret->epoch_output = false;
  p_ret->monotonic_output = false;
  p_ret->uid_output = false;
  p_ret->descriptive_output = false;
  descriptive_output = false;

  return p_ret;
}

static list_declare(convertHead);

void android_log_format_free(AndroidLogFormat* p_format) {
  FilterInfo *p_info, *p_info_old;

  p_info = p_format->filters;

  while (p_info != NULL) {
    p_info_old = p_info;
    p_info = p_info->p_next;

    free(p_info_old);
  }

  free(p_format);

  /* Free conversion resource, can always be reconstructed */
  while (!list_empty(&convertHead)) {
    struct listnode* node = list_head(&convertHead);
    list_remove(node);
    LOG_ALWAYS_FATAL_IF(node == list_head(&convertHead), "corrupted list");
    free(node);
  }
}

int android_log_setPrintFormat(AndroidLogFormat* p_format, AndroidLogPrintFormat format) {
  switch (format) {
    case FORMAT_MODIFIER_COLOR:
      p_format->colored_output = true;
      return 0;
    case FORMAT_MODIFIER_TIME_USEC:
      p_format->usec_time_output = true;
      return 0;
    case FORMAT_MODIFIER_TIME_NSEC:
      p_format->nsec_time_output = true;
      return 0;
    case FORMAT_MODIFIER_PRINTABLE:
      p_format->printable_output = true;
      return 0;
    case FORMAT_MODIFIER_YEAR:
      p_format->year_output = true;
      return 0;
    case FORMAT_MODIFIER_ZONE:
      p_format->zone_output = !p_format->zone_output;
      return 0;
    case FORMAT_MODIFIER_EPOCH:
      p_format->epoch_output = true;
      return 0;
    case FORMAT_MODIFIER_MONOTONIC:
      p_format->monotonic_output = true;
      return 0;
    case FORMAT_MODIFIER_UID:
      p_format->uid_output = true;
      return 0;
    case FORMAT_MODIFIER_DESCRIPT:
      p_format->descriptive_output = true;
      descriptive_output = true;
      return 0;
    default:
      break;
  }
  p_format->format = format;
  return 1;
}

#ifndef __MINGW32__
static const char tz[] = "TZ";
static const char utc[] = "UTC";
#endif

/**
 * Returns FORMAT_OFF on invalid string
 */
AndroidLogPrintFormat android_log_formatFromString(const char* formatString) {
  static AndroidLogPrintFormat format;

  /* clang-format off */
  if (!strcmp(formatString, "brief")) format = FORMAT_BRIEF;
  else if (!strcmp(formatString, "process")) format = FORMAT_PROCESS;
  else if (!strcmp(formatString, "tag")) format = FORMAT_TAG;
  else if (!strcmp(formatString, "thread")) format = FORMAT_THREAD;
  else if (!strcmp(formatString, "raw")) format = FORMAT_RAW;
  else if (!strcmp(formatString, "time")) format = FORMAT_TIME;
  else if (!strcmp(formatString, "threadtime")) format = FORMAT_THREADTIME;
  else if (!strcmp(formatString, "long")) format = FORMAT_LONG;
  else if (!strcmp(formatString, "color")) format = FORMAT_MODIFIER_COLOR;
  else if (!strcmp(formatString, "colour")) format = FORMAT_MODIFIER_COLOR;
  else if (!strcmp(formatString, "usec")) format = FORMAT_MODIFIER_TIME_USEC;
  else if (!strcmp(formatString, "nsec")) format = FORMAT_MODIFIER_TIME_NSEC;
  else if (!strcmp(formatString, "printable")) format = FORMAT_MODIFIER_PRINTABLE;
  else if (!strcmp(formatString, "year")) format = FORMAT_MODIFIER_YEAR;
  else if (!strcmp(formatString, "zone")) format = FORMAT_MODIFIER_ZONE;
  else if (!strcmp(formatString, "epoch")) format = FORMAT_MODIFIER_EPOCH;
  else if (!strcmp(formatString, "monotonic")) format = FORMAT_MODIFIER_MONOTONIC;
  else if (!strcmp(formatString, "uid")) format = FORMAT_MODIFIER_UID;
  else if (!strcmp(formatString, "descriptive")) format = FORMAT_MODIFIER_DESCRIPT;
    /* clang-format on */

#ifndef __MINGW32__
  else {
    extern char* tzname[2];
    static const char gmt[] = "GMT";
    char* cp = getenv(tz);
    if (cp) {
      cp = strdup(cp);
    }
    setenv(tz, formatString, 1);
    /*
     * Run tzset here to determine if the timezone is legitimate. If the
     * zone is GMT, check if that is what was asked for, if not then
     * did not match any on the system; report an error to caller.
     */
    tzset();
    if (!tzname[0] ||
        ((!strcmp(tzname[0], utc) || !strcmp(tzname[0], gmt))                  /* error? */
         && strcasecmp(formatString, utc) && strcasecmp(formatString, gmt))) { /* ok */
      if (cp) {
        setenv(tz, cp, 1);
      } else {
        unsetenv(tz);
      }
      tzset();
      format = FORMAT_OFF;
    } else {
      format = FORMAT_MODIFIER_ZONE;
    }
    free(cp);
  }
#endif

  return format;
}

/**
 * filterExpression: a single filter expression
 * eg "AT:d"
 *
 * returns 0 on success and -1 on invalid expression
 *
 * Assumes single threaded execution
 */

int android_log_addFilterRule(AndroidLogFormat* p_format, const char* filterExpression) {
  size_t tagNameLength;
  android_LogPriority pri = ANDROID_LOG_DEFAULT;

  tagNameLength = strcspn(filterExpression, ":");

  if (tagNameLength == 0) {
    goto error;
  }

  if (filterExpression[tagNameLength] == ':') {
    pri = filterCharToPri(filterExpression[tagNameLength + 1]);

    if (pri == ANDROID_LOG_UNKNOWN) {
      goto error;
    }
  }

  if (0 == strncmp("*", filterExpression, tagNameLength)) {
    /*
     * This filter expression refers to the global filter
     * The default level for this is DEBUG if the priority
     * is unspecified
     */
    if (pri == ANDROID_LOG_DEFAULT) {
      pri = ANDROID_LOG_DEBUG;
    }

    p_format->global_pri = pri;
  } else {
    /*
     * for filter expressions that don't refer to the global
     * filter, the default is verbose if the priority is unspecified
     */
    if (pri == ANDROID_LOG_DEFAULT) {
      pri = ANDROID_LOG_VERBOSE;
    }

    char* tagName;

/*
 * Presently HAVE_STRNDUP is never defined, so the second case is always taken
 * Darwin doesn't have strndup, everything else does
 */
#ifdef HAVE_STRNDUP
    tagName = strndup(filterExpression, tagNameLength);
#else
    /* a few extra bytes copied... */
    tagName = strdup(filterExpression);
    tagName[tagNameLength] = '\0';
#endif /*HAVE_STRNDUP*/

    FilterInfo* p_fi = filterinfo_new(tagName, pri);
    free(tagName);

    p_fi->p_next = p_format->filters;
    p_format->filters = p_fi;
  }

  return 0;
error:
  return -1;
}

#ifndef HAVE_STRSEP
/* KISS replacement helper for below */
static char* strsep(char** stringp, const char* delim) {
  char* token;
  char* ret = *stringp;

  if (!ret || !*ret) {
    return NULL;
  }
  token = strpbrk(ret, delim);
  if (token) {
    *token = '\0';
    ++token;
  } else {
    token = ret + strlen(ret);
  }
  *stringp = token;
  return ret;
}
#endif

/**
 * filterString: a comma/whitespace-separated set of filter expressions
 *
 * eg "AT:d *:i"
 *
 * returns 0 on success and -1 on invalid expression
 *
 * Assumes single threaded execution
 *
 */
int android_log_addFilterString(AndroidLogFormat* p_format, const char* filterString) {
  char* filterStringCopy = strdup(filterString);
  char* p_cur = filterStringCopy;
  char* p_ret;
  int err;

  /* Yes, I'm using strsep */
  while (NULL != (p_ret = strsep(&p_cur, " \t,"))) {
    /* ignore whitespace-only entries */
    if (p_ret[0] != '\0') {
      err = android_log_addFilterRule(p_format, p_ret);

      if (err < 0) {
        goto error;
      }
    }
  }

  free(filterStringCopy);
  return 0;
error:
  free(filterStringCopy);
  return -1;
}

/**
 * Splits a wire-format buffer into an AndroidLogEntry
 * entry allocated by caller. Pointers will point directly into buf
 *
 * Returns 0 on success and -1 on invalid wire format (entry will be
 * in unspecified state)
 */
int android_log_processLogBuffer(struct logger_entry* buf, AndroidLogEntry* entry) {
  entry->message = NULL;
  entry->messageLen = 0;

  entry->tv_sec = buf->sec;
  entry->tv_nsec = buf->nsec;
  entry->uid = -1;
  entry->pid = buf->pid;
  entry->tid = buf->tid;

  /*
   * format: <priority:1><tag:N>\0<message:N>\0
   *
   * tag str
   *   starts at buf + buf->hdr_size + 1
   * msg
   *   starts at buf + buf->hdr_size + 1 + len(tag) + 1
   *
   * The message may have been truncated.  When that happens, we must null-terminate the message
   * ourselves.
   */
  if (buf->len < 3) {
    /*
     * An well-formed entry must consist of at least a priority
     * and two null characters
     */
    fprintf(stderr, "+++ LOG: entry too small\n");
    return -1;
  }

  int msgStart = -1;
  int msgEnd = -1;

  int i;
  if (buf->hdr_size < sizeof(logger_entry)) {
    fprintf(stderr, "+++ LOG: hdr_size must be at least as big as struct logger_entry\n");
    return -1;
  }
  char* msg = reinterpret_cast<char*>(buf) + buf->hdr_size;
  entry->uid = buf->uid;

  for (i = 1; i < buf->len; i++) {
    if (msg[i] == '\0') {
      if (msgStart == -1) {
        msgStart = i + 1;
      } else {
        msgEnd = i;
        break;
      }
    }
  }

  if (msgStart == -1) {
    /* +++ LOG: malformed log message, DYB */
    for (i = 1; i < buf->len; i++) {
      /* odd characters in tag? */
      if ((msg[i] <= ' ') || (msg[i] == ':') || (msg[i] >= 0x7f)) {
        msg[i] = '\0';
        msgStart = i + 1;
        break;
      }
    }
    if (msgStart == -1) {
      msgStart = buf->len - 1; /* All tag, no message, print truncates */
    }
  }
  if (msgEnd == -1) {
    /* incoming message not null-terminated; force it */
    msgEnd = buf->len - 1; /* may result in msgEnd < msgStart */
    msg[msgEnd] = '\0';
  }

  entry->priority = static_cast<android_LogPriority>(msg[0]);
  entry->tag = msg + 1;
  entry->tagLen = msgStart - 1;
  entry->message = msg + msgStart;
  entry->messageLen = (msgEnd < msgStart) ? 0 : (msgEnd - msgStart);

  return 0;
}

static bool findChar(const char** cp, size_t* len, int c) {
  while ((*len) && isspace(*(*cp))) {
    ++(*cp);
    --(*len);
  }
  if (c == INT_MAX) return *len;
  if ((*len) && (*(*cp) == c)) {
    ++(*cp);
    --(*len);
    return true;
  }
  return false;
}

/*
 * Recursively convert binary log data to printable form.
 *
 * This needs to be recursive because you can have lists of lists.
 *
 * If we run out of room, we stop processing immediately.  It's important
 * for us to check for space on every output element to avoid producing
 * garbled output.
 *
 * Returns 0 on success, 1 on buffer full, -1 on failure.
 */
enum objectType {
  TYPE_OBJECTS = '1',
  TYPE_BYTES = '2',
  TYPE_MILLISECONDS = '3',
  TYPE_ALLOCATIONS = '4',
  TYPE_ID = '5',
  TYPE_PERCENT = '6',
  TYPE_MONOTONIC = 's'
};

static int android_log_printBinaryEvent(const unsigned char** pEventData, size_t* pEventDataLen,
                                        char** pOutBuf, size_t* pOutBufLen, const char** fmtStr,
                                        size_t* fmtLen) {
  const unsigned char* eventData = *pEventData;
  size_t eventDataLen = *pEventDataLen;
  char* outBuf = *pOutBuf;
  char* outBufSave = outBuf;
  size_t outBufLen = *pOutBufLen;
  size_t outBufLenSave = outBufLen;
  unsigned char type;
  size_t outCount = 0;
  int result = 0;
  const char* cp;
  size_t len;
  int64_t lval;

  if (eventDataLen < 1) return -1;

  type = *eventData;

  cp = NULL;
  len = 0;
  if (fmtStr && *fmtStr && fmtLen && *fmtLen && **fmtStr) {
    cp = *fmtStr;
    len = *fmtLen;
  }
  /*
   * event.logtag format specification:
   *
   * Optionally, after the tag names can be put a description for the value(s)
   * of the tag. Description are in the format
   *    (<name>|data type[|data unit])
   * Multiple values are separated by commas.
   *
   * The data type is a number from the following values:
   * 1: int
   * 2: long
   * 3: string
   * 4: list
   * 5: float
   *
   * The data unit is a number taken from the following list:
   * 1: Number of objects
   * 2: Number of bytes
   * 3: Number of milliseconds
   * 4: Number of allocations
   * 5: Id
   * 6: Percent
   * s: Number of seconds (monotonic time)
   * Default value for data of type int/long is 2 (bytes).
   */
  if (!cp || !findChar(&cp, &len, '(')) {
    len = 0;
  } else {
    char* outBufLastSpace = NULL;

    findChar(&cp, &len, INT_MAX);
    while (len && *cp && (*cp != '|') && (*cp != ')')) {
      if (outBufLen <= 0) {
        /* halt output */
        goto no_room;
      }
      outBufLastSpace = isspace(*cp) ? outBuf : NULL;
      *outBuf = *cp;
      ++outBuf;
      ++cp;
      --outBufLen;
      --len;
    }
    if (outBufLastSpace) {
      outBufLen += outBuf - outBufLastSpace;
      outBuf = outBufLastSpace;
    }
    if (outBufLen <= 0) {
      /* halt output */
      goto no_room;
    }
    if (outBufSave != outBuf) {
      *outBuf = '=';
      ++outBuf;
      --outBufLen;
    }

    if (findChar(&cp, &len, '|') && findChar(&cp, &len, INT_MAX)) {
      static const unsigned char typeTable[] = {EVENT_TYPE_INT, EVENT_TYPE_LONG, EVENT_TYPE_STRING,
                                                EVENT_TYPE_LIST, EVENT_TYPE_FLOAT};

      if ((*cp >= '1') && (*cp < (char)('1' + (sizeof(typeTable) / sizeof(typeTable[0])))) &&
          (type != typeTable[(size_t)(*cp - '1')]))
        len = 0;

      if (len) {
        ++cp;
        --len;
      } else {
        /* reset the format */
        outBuf = outBufSave;
        outBufLen = outBufLenSave;
      }
    }
  }
  outCount = 0;
  lval = 0;
  switch (type) {
    case EVENT_TYPE_INT:
      /* 32-bit signed int */
      {
        if (eventDataLen < sizeof(android_event_int_t)) return -1;
        auto* event_int = reinterpret_cast<const android_event_int_t*>(eventData);
        lval = event_int->data;
        eventData += sizeof(android_event_int_t);
        eventDataLen -= sizeof(android_event_int_t);
      }
      goto pr_lval;
    case EVENT_TYPE_LONG:
      /* 64-bit signed long */
      if (eventDataLen < sizeof(android_event_long_t)) {
        return -1;
      }
      {
        auto* event_long = reinterpret_cast<const android_event_long_t*>(eventData);
        lval = event_long->data;
      }
      eventData += sizeof(android_event_long_t);
      eventDataLen -= sizeof(android_event_long_t);
    pr_lval:
      outCount = snprintf(outBuf, outBufLen, "%" PRId64, lval);
      if (outCount < outBufLen) {
        outBuf += outCount;
        outBufLen -= outCount;
      } else {
        /* halt output */
        goto no_room;
      }
      break;
    case EVENT_TYPE_FLOAT:
      /* float */
      {
        if (eventDataLen < sizeof(android_event_float_t)) return -1;
        auto* event_float = reinterpret_cast<const android_event_float_t*>(eventData);
        float fval = event_float->data;
        eventData += sizeof(android_event_int_t);
        eventDataLen -= sizeof(android_event_int_t);

        outCount = snprintf(outBuf, outBufLen, "%f", fval);
        if (outCount < outBufLen) {
          outBuf += outCount;
          outBufLen -= outCount;
        } else {
          /* halt output */
          goto no_room;
        }
      }
      break;
    case EVENT_TYPE_STRING:
      /* UTF-8 chars, not NULL-terminated */
      {
        if (eventDataLen < sizeof(android_event_string_t)) return -1;
        auto* event_string = reinterpret_cast<const android_event_string_t*>(eventData);
        unsigned int strLen = event_string->length;
        eventData += sizeof(android_event_string_t);
        eventDataLen -= sizeof(android_event_string_t);

        if (eventDataLen < strLen) {
          result = -1; /* mark truncated */
          strLen = eventDataLen;
        }

        if (cp && (strLen == 0)) {
          /* reset the format if no content */
          outBuf = outBufSave;
          outBufLen = outBufLenSave;
        }
        if (strLen < outBufLen) {
          memcpy(outBuf, eventData, strLen);
          outBuf += strLen;
          outBufLen -= strLen;
        } else {
          if (outBufLen > 0) {
            /* copy what we can */
            memcpy(outBuf, eventData, outBufLen);
            outBuf += outBufLen;
            outBufLen -= outBufLen;
          }
          if (!result) result = 1; /* if not truncated, return no room */
        }
        eventData += strLen;
        eventDataLen -= strLen;
        if (result != 0) goto bail;
        break;
      }
    case EVENT_TYPE_LIST:
      /* N items, all different types */
      {
        if (eventDataLen < sizeof(android_event_list_t)) return -1;
        auto* event_list = reinterpret_cast<const android_event_list_t*>(eventData);

        int8_t count = event_list->element_count;
        eventData += sizeof(android_event_list_t);
        eventDataLen -= sizeof(android_event_list_t);

        if (outBufLen <= 0) goto no_room;

        *outBuf++ = '[';
        outBufLen--;

        for (int i = 0; i < count; i++) {
          result = android_log_printBinaryEvent(&eventData, &eventDataLen, &outBuf, &outBufLen,
                                                fmtStr, fmtLen);
          if (result != 0) goto bail;

          if (i < (count - 1)) {
            if (outBufLen <= 0) goto no_room;
            *outBuf++ = ',';
            outBufLen--;
          }
        }

        if (outBufLen <= 0) goto no_room;

        *outBuf++ = ']';
        outBufLen--;
      }
      break;
    default:
      fprintf(stderr, "Unknown binary event type %d\n", type);
      return -1;
  }
  if (cp && len) {
    if (findChar(&cp, &len, '|') && findChar(&cp, &len, INT_MAX)) {
      switch (*cp) {
        case TYPE_OBJECTS:
          outCount = 0;
          /* outCount = snprintf(outBuf, outBufLen, " objects"); */
          break;
        case TYPE_BYTES:
          if ((lval != 0) && ((lval % 1024) == 0)) {
            /* repaint with multiplier */
            static const char suffixTable[] = {'K', 'M', 'G', 'T'};
            size_t idx = 0;
            outBuf -= outCount;
            outBufLen += outCount;
            do {
              lval /= 1024;
              if ((lval % 1024) != 0) break;
            } while (++idx < ((sizeof(suffixTable) / sizeof(suffixTable[0])) - 1));
            outCount = snprintf(outBuf, outBufLen, "%" PRId64 "%cB", lval, suffixTable[idx]);
          } else {
            outCount = snprintf(outBuf, outBufLen, "B");
          }
          break;
        case TYPE_MILLISECONDS:
          if (((lval <= -1000) || (1000 <= lval)) && (outBufLen || (outBuf[-1] == '0'))) {
            /* repaint as (fractional) seconds, possibly saving space */
            if (outBufLen) outBuf[0] = outBuf[-1];
            outBuf[-1] = outBuf[-2];
            outBuf[-2] = outBuf[-3];
            outBuf[-3] = '.';
            while ((outBufLen == 0) || (*outBuf == '0')) {
              --outBuf;
              ++outBufLen;
            }
            if (*outBuf != '.') {
              ++outBuf;
              --outBufLen;
            }
            outCount = snprintf(outBuf, outBufLen, "s");
          } else {
            outCount = snprintf(outBuf, outBufLen, "ms");
          }
          break;
        case TYPE_MONOTONIC: {
          static const uint64_t minute = 60;
          static const uint64_t hour = 60 * minute;
          static const uint64_t day = 24 * hour;

          /* Repaint as unsigned seconds, minutes, hours ... */
          outBuf -= outCount;
          outBufLen += outCount;
          uint64_t val = lval;
          if (val >= day) {
            outCount = snprintf(outBuf, outBufLen, "%" PRIu64 "d ", val / day);
            if (outCount >= outBufLen) break;
            outBuf += outCount;
            outBufLen -= outCount;
            val = (val % day) + day;
          }
          if (val >= minute) {
            if (val >= hour) {
              outCount = snprintf(outBuf, outBufLen, "%" PRIu64 ":", (val / hour) % (day / hour));
              if (outCount >= outBufLen) break;
              outBuf += outCount;
              outBufLen -= outCount;
            }
            outCount =
                snprintf(outBuf, outBufLen, (val >= hour) ? "%02" PRIu64 ":" : "%" PRIu64 ":",
                         (val / minute) % (hour / minute));
            if (outCount >= outBufLen) break;
            outBuf += outCount;
            outBufLen -= outCount;
          }
          outCount = snprintf(outBuf, outBufLen, (val >= minute) ? "%02" PRIu64 : "%" PRIu64 "s",
                              val % minute);
        } break;
        case TYPE_ALLOCATIONS:
          outCount = 0;
          /* outCount = snprintf(outBuf, outBufLen, " allocations"); */
          break;
        case TYPE_ID:
          outCount = 0;
          break;
        case TYPE_PERCENT:
          outCount = snprintf(outBuf, outBufLen, "%%");
          break;
        default: /* ? */
          outCount = 0;
          break;
      }
      ++cp;
      --len;
      if (outCount < outBufLen) {
        outBuf += outCount;
        outBufLen -= outCount;
      } else if (outCount) {
        /* halt output */
        goto no_room;
      }
    }
    if (!findChar(&cp, &len, ')')) len = 0;
    if (!findChar(&cp, &len, ',')) len = 0;
  }

bail:
  *pEventData = eventData;
  *pEventDataLen = eventDataLen;
  *pOutBuf = outBuf;
  *pOutBufLen = outBufLen;
  if (cp) {
    *fmtStr = cp;
    *fmtLen = len;
  }
  return result;

no_room:
  result = 1;
  goto bail;
}

/**
 * Convert a binary log entry to ASCII form.
 *
 * For convenience we mimic the processLogBuffer API.  There is no
 * pre-defined output length for the binary data, since we're free to format
 * it however we choose, which means we can't really use a fixed-size buffer
 * here.
 */
int android_log_processBinaryLogBuffer(
    struct logger_entry* buf, AndroidLogEntry* entry,
    [[maybe_unused]] const EventTagMap* map, /* only on !__ANDROID__ */
    char* messageBuf, int messageBufLen) {
  size_t inCount;
  uint32_t tagIndex;
  const unsigned char* eventData;

  entry->message = NULL;
  entry->messageLen = 0;

  entry->tv_sec = buf->sec;
  entry->tv_nsec = buf->nsec;
  entry->priority = ANDROID_LOG_INFO;
  entry->uid = -1;
  entry->pid = buf->pid;
  entry->tid = buf->tid;

  if (buf->hdr_size < sizeof(logger_entry)) {
    fprintf(stderr, "+++ LOG: hdr_size must be at least as big as struct logger_entry\n");
    return -1;
  }
  eventData = reinterpret_cast<unsigned char*>(buf) + buf->hdr_size;
  if (buf->lid == LOG_ID_SECURITY) {
    entry->priority = ANDROID_LOG_WARN;
  }
  entry->uid = buf->uid;
  inCount = buf->len;
  if (inCount < sizeof(android_event_header_t)) return -1;
  auto* event_header = reinterpret_cast<const android_event_header_t*>(eventData);
  tagIndex = event_header->tag;
  eventData += sizeof(android_event_header_t);
  inCount -= sizeof(android_event_header_t);

  entry->tagLen = 0;
  entry->tag = NULL;
#ifdef __ANDROID__
  if (map != NULL) {
    entry->tag = android_lookupEventTag_len(map, &entry->tagLen, tagIndex);
  }
#endif

  /*
   * If we don't have a map, or didn't find the tag number in the map,
   * stuff a generated tag value into the start of the output buffer and
   * shift the buffer pointers down.
   */
  if (entry->tag == NULL) {
    size_t tagLen;

    tagLen = snprintf(messageBuf, messageBufLen, "[%" PRIu32 "]", tagIndex);
    if (tagLen >= (size_t)messageBufLen) {
      tagLen = messageBufLen - 1;
    }
    entry->tag = messageBuf;
    entry->tagLen = tagLen;
    messageBuf += tagLen + 1;
    messageBufLen -= tagLen + 1;
  }

  /*
   * Format the event log data into the buffer.
   */
  const char* fmtStr = NULL;
  size_t fmtLen = 0;
#ifdef __ANDROID__
  if (descriptive_output && map) {
    fmtStr = android_lookupEventFormat_len(map, &fmtLen, tagIndex);
  }
#endif

  char* outBuf = messageBuf;
  size_t outRemaining = messageBufLen - 1; /* leave one for nul byte */
  int result = 0;

  if ((inCount > 0) || fmtLen) {
    result = android_log_printBinaryEvent(&eventData, &inCount, &outBuf, &outRemaining, &fmtStr,
                                          &fmtLen);
  }
  if ((result == 1) && fmtStr) {
    /* We overflowed :-(, let's repaint the line w/o format dressings */
    eventData = reinterpret_cast<unsigned char*>(buf) + buf->hdr_size;
    eventData += 4;
    outBuf = messageBuf;
    outRemaining = messageBufLen - 1;
    result = android_log_printBinaryEvent(&eventData, &inCount, &outBuf, &outRemaining, NULL, NULL);
  }
  if (result < 0) {
    fprintf(stderr, "Binary log entry conversion failed\n");
  }
  if (result) {
    if (!outRemaining) {
      /* make space to leave an indicator */
      --outBuf;
      ++outRemaining;
    }
    *outBuf++ = (result < 0) ? '!' : '^'; /* Error or Truncation? */
    outRemaining--;
    /* pretend we ate all the data to prevent log stutter */
    inCount = 0;
    if (result > 0) result = 0;
  }

  /* eat the silly terminating '\n' */
  if (inCount == 1 && *eventData == '\n') {
    eventData++;
    inCount--;
  }

  if (inCount != 0) {
    fprintf(stderr, "Warning: leftover binary log data (%zu bytes)\n", inCount);
  }

  /*
   * Terminate the buffer.  The NUL byte does not count as part of
   * entry->messageLen.
   */
  *outBuf = '\0';
  entry->messageLen = outBuf - messageBuf;
  assert(entry->messageLen == (messageBufLen - 1) - outRemaining);

  entry->message = messageBuf;

  return result;
}

/*
 * Convert to printable from message to p buffer, return string length. If p is
 * NULL, do not copy, but still return the expected string length.
 */
size_t convertPrintable(char* p, const char* message, size_t messageLen) {
  char* begin = p;
  bool print = p != NULL;
  mbstate_t mb_state = {};

  while (messageLen) {
    char buf[6];
    ssize_t len = sizeof(buf) - 1;
    if ((size_t)len > messageLen) {
      len = messageLen;
    }
    len = mbrtowc(nullptr, message, len, &mb_state);

    if (len < 0) {
      snprintf(buf, sizeof(buf), "\\x%02X", static_cast<unsigned char>(*message));
      len = 1;
    } else {
      buf[0] = '\0';
      if (len == 1) {
        if (*message == '\a') {
          strcpy(buf, "\\a");
        } else if (*message == '\b') {
          strcpy(buf, "\\b");
        } else if (*message == '\t') {
          strcpy(buf, "\t"); /* Do not escape tabs */
        } else if (*message == '\v') {
          strcpy(buf, "\\v");
        } else if (*message == '\f') {
          strcpy(buf, "\\f");
        } else if (*message == '\r') {
          strcpy(buf, "\\r");
        } else if (*message == '\\') {
          strcpy(buf, "\\\\");
        } else if ((*message < ' ') || (*message & 0x80)) {
          snprintf(buf, sizeof(buf), "\\x%02X", static_cast<unsigned char>(*message));
        }
      }
      if (!buf[0]) {
        strncpy(buf, message, len);
        buf[len] = '\0';
      }
    }
    if (print) {
      strcpy(p, buf);
    }
    p += strlen(buf);
    message += len;
    messageLen -= len;
  }
  return p - begin;
}

#ifdef __ANDROID__
static char* readSeconds(char* e, struct timespec* t) {
  unsigned long multiplier;
  char* p;
  t->tv_sec = strtoul(e, &p, 10);
  if (*p != '.') {
    return NULL;
  }
  t->tv_nsec = 0;
  multiplier = NS_PER_SEC;
  while (isdigit(*++p) && (multiplier /= 10)) {
    t->tv_nsec += (*p - '0') * multiplier;
  }
  return p;
}

static struct timespec* sumTimespec(struct timespec* left, struct timespec* right) {
  left->tv_nsec += right->tv_nsec;
  left->tv_sec += right->tv_sec;
  if (left->tv_nsec >= (long)NS_PER_SEC) {
    left->tv_nsec -= NS_PER_SEC;
    left->tv_sec += 1;
  }
  return left;
}

static struct timespec* subTimespec(struct timespec* result, struct timespec* left,
                                    struct timespec* right) {
  result->tv_nsec = left->tv_nsec - right->tv_nsec;
  result->tv_sec = left->tv_sec - right->tv_sec;
  if (result->tv_nsec < 0) {
    result->tv_nsec += NS_PER_SEC;
    result->tv_sec -= 1;
  }
  return result;
}

static long long nsecTimespec(struct timespec* now) {
  return (long long)now->tv_sec * NS_PER_SEC + now->tv_nsec;
}

static void convertMonotonic(struct timespec* result, const AndroidLogEntry* entry) {
  struct listnode* node;
  struct conversionList {
    struct listnode node; /* first */
    struct timespec time;
    struct timespec convert;
  } * list, *next;
  struct timespec time, convert;

  /* If we do not have a conversion list, build one up */
  if (list_empty(&convertHead)) {
    bool suspended_pending = false;
    struct timespec suspended_monotonic = {0, 0};
    struct timespec suspended_diff = {0, 0};

    /*
     * Read dmesg for _some_ synchronization markers and insert
     * Anything in the Android Logger before the dmesg logging span will
     * be highly suspect regarding the monotonic time calculations.
     */
    FILE* p = popen("/system/bin/dmesg", "re");
    if (p) {
      char* line = NULL;
      size_t len = 0;
      while (getline(&line, &len, p) > 0) {
        static const char suspend[] = "PM: suspend entry ";
        static const char resume[] = "PM: suspend exit ";
        static const char healthd[] = "healthd";
        static const char battery[] = ": battery ";
        static const char suspended[] = "Suspended for ";
        struct timespec monotonic;
        struct tm tm;
        char *cp, *e = line;
        bool add_entry = true;

        if (*e == '<') {
          while (*e && (*e != '>')) {
            ++e;
          }
          if (*e != '>') {
            continue;
          }
        }
        if (*e != '[') {
          continue;
        }
        while (*++e == ' ') {
          ;
        }
        e = readSeconds(e, &monotonic);
        if (!e || (*e != ']')) {
          continue;
        }

        if ((e = strstr(e, suspend))) {
          e += sizeof(suspend) - 1;
        } else if ((e = strstr(line, resume))) {
          e += sizeof(resume) - 1;
        } else if (((e = strstr(line, healthd))) &&
                   ((e = strstr(e + sizeof(healthd) - 1, battery)))) {
          /* NB: healthd is roughly 150us late, worth the price to
           * deal with ntp-induced or hardware clock drift. */
          e += sizeof(battery) - 1;
        } else if ((e = strstr(line, suspended))) {
          e += sizeof(suspended) - 1;
          e = readSeconds(e, &time);
          if (!e) {
            continue;
          }
          add_entry = false;
          suspended_pending = true;
          suspended_monotonic = monotonic;
          suspended_diff = time;
        } else {
          continue;
        }
        if (add_entry) {
          /* look for "????-??-?? ??:??:??.????????? UTC" */
          cp = strstr(e, " UTC");
          if (!cp || ((cp - e) < 29) || (cp[-10] != '.')) {
            continue;
          }
          e = cp - 29;
          cp = readSeconds(cp - 10, &time);
          if (!cp) {
            continue;
          }
          cp = strptime(e, "%Y-%m-%d %H:%M:%S.", &tm);
          if (!cp) {
            continue;
          }
          cp = getenv(tz);
          if (cp) {
            cp = strdup(cp);
          }
          setenv(tz, utc, 1);
          time.tv_sec = mktime(&tm);
          if (cp) {
            setenv(tz, cp, 1);
            free(cp);
          } else {
            unsetenv(tz);
          }
          list = static_cast<conversionList*>(calloc(1, sizeof(conversionList)));
          list_init(&list->node);
          list->time = time;
          subTimespec(&list->convert, &time, &monotonic);
          list_add_tail(&convertHead, &list->node);
        }
        if (suspended_pending && !list_empty(&convertHead)) {
          list = node_to_item(list_tail(&convertHead), struct conversionList, node);
          if (subTimespec(&time, subTimespec(&time, &list->time, &list->convert),
                          &suspended_monotonic)
                  ->tv_sec > 0) {
            /* resume, what is convert factor before? */
            subTimespec(&convert, &list->convert, &suspended_diff);
          } else {
            /* suspend */
            convert = list->convert;
          }
          time = suspended_monotonic;
          sumTimespec(&time, &convert);
          /* breakpoint just before sleep */
          list = static_cast<conversionList*>(calloc(1, sizeof(conversionList)));
          list_init(&list->node);
          list->time = time;
          list->convert = convert;
          list_add_tail(&convertHead, &list->node);
          /* breakpoint just after sleep */
          list = static_cast<conversionList*>(calloc(1, sizeof(conversionList)));
          list_init(&list->node);
          list->time = time;
          sumTimespec(&list->time, &suspended_diff);
          list->convert = convert;
          sumTimespec(&list->convert, &suspended_diff);
          list_add_tail(&convertHead, &list->node);
          suspended_pending = false;
        }
      }
      pclose(p);
    }
    /* last entry is our current time conversion */
    list = static_cast<conversionList*>(calloc(1, sizeof(conversionList)));
    list_init(&list->node);
    clock_gettime(CLOCK_REALTIME, &list->time);
    clock_gettime(CLOCK_MONOTONIC, &convert);
    clock_gettime(CLOCK_MONOTONIC, &time);
    /* Correct for instant clock_gettime latency (syscall or ~30ns) */
    subTimespec(&time, &convert, subTimespec(&time, &time, &convert));
    /* Calculate conversion factor */
    subTimespec(&list->convert, &list->time, &time);
    list_add_tail(&convertHead, &list->node);
    if (suspended_pending) {
      /* manufacture a suspend @ point before */
      subTimespec(&convert, &list->convert, &suspended_diff);
      time = suspended_monotonic;
      sumTimespec(&time, &convert);
      /* breakpoint just after sleep */
      list = static_cast<conversionList*>(calloc(1, sizeof(conversionList)));
      list_init(&list->node);
      list->time = time;
      sumTimespec(&list->time, &suspended_diff);
      list->convert = convert;
      sumTimespec(&list->convert, &suspended_diff);
      list_add_head(&convertHead, &list->node);
      /* breakpoint just before sleep */
      list = static_cast<conversionList*>(calloc(1, sizeof(conversionList)));
      list_init(&list->node);
      list->time = time;
      list->convert = convert;
      list_add_head(&convertHead, &list->node);
    }
  }

  /* Find the breakpoint in the conversion list */
  list = node_to_item(list_head(&convertHead), struct conversionList, node);
  next = NULL;
  list_for_each(node, &convertHead) {
    next = node_to_item(node, struct conversionList, node);
    if (entry->tv_sec < next->time.tv_sec) {
      break;
    } else if (entry->tv_sec == next->time.tv_sec) {
      if (entry->tv_nsec < next->time.tv_nsec) {
        break;
      }
    }
    list = next;
  }

  /* blend time from one breakpoint to the next */
  convert = list->convert;
  if (next) {
    unsigned long long total, run;

    total = nsecTimespec(subTimespec(&time, &next->time, &list->time));
    time.tv_sec = entry->tv_sec;
    time.tv_nsec = entry->tv_nsec;
    run = nsecTimespec(subTimespec(&time, &time, &list->time));
    if (run < total) {
      long long crun;

      float f = nsecTimespec(subTimespec(&time, &next->convert, &convert));
      f *= run;
      f /= total;
      crun = f;
      convert.tv_sec += crun / (long long)NS_PER_SEC;
      if (crun < 0) {
        convert.tv_nsec -= (-crun) % NS_PER_SEC;
        if (convert.tv_nsec < 0) {
          convert.tv_nsec += NS_PER_SEC;
          convert.tv_sec -= 1;
        }
      } else {
        convert.tv_nsec += crun % NS_PER_SEC;
        if (convert.tv_nsec >= (long)NS_PER_SEC) {
          convert.tv_nsec -= NS_PER_SEC;
          convert.tv_sec += 1;
        }
      }
    }
  }

  /* Apply the correction factor */
  result->tv_sec = entry->tv_sec;
  result->tv_nsec = entry->tv_nsec;
  subTimespec(result, result, &convert);
}
#endif

/**
 * Formats a log message into a buffer
 *
 * Uses defaultBuffer if it can, otherwise malloc()'s a new buffer
 * If return value != defaultBuffer, caller must call free()
 * Returns NULL on malloc error
 */

char* android_log_formatLogLine(AndroidLogFormat* p_format, char* defaultBuffer,
                                size_t defaultBufferSize, const AndroidLogEntry* entry,
                                size_t* p_outLength) {
#if !defined(_WIN32)
  struct tm tmBuf;
#endif
  struct tm* ptm;
  /* good margin, 23+nul for msec, 26+nul for usec, 29+nul to nsec */
  char timeBuf[64];
  char prefixBuf[128], suffixBuf[128];
  char priChar;
  int prefixSuffixIsHeaderFooter = 0;
  char* ret;
  time_t now;
  unsigned long nsec;

  priChar = filterPriToChar(entry->priority);
  size_t prefixLen = 0, suffixLen = 0;
  size_t len;

  /*
   * Get the current date/time in pretty form
   *
   * It's often useful when examining a log with "less" to jump to
   * a specific point in the file by searching for the date/time stamp.
   * For this reason it's very annoying to have regexp meta characters
   * in the time stamp.  Don't use forward slashes, parenthesis,
   * brackets, asterisks, or other special chars here.
   *
   * The caller may have affected the timezone environment, this is
   * expected to be sensitive to that.
   */
  now = entry->tv_sec;
  nsec = entry->tv_nsec;
#if __ANDROID__
  if (p_format->monotonic_output) {
    struct timespec time;
    convertMonotonic(&time, entry);
    now = time.tv_sec;
    nsec = time.tv_nsec;
  }
#endif
  if (now < 0) {
    nsec = NS_PER_SEC - nsec;
  }
  if (p_format->epoch_output || p_format->monotonic_output) {
    ptm = NULL;
    snprintf(timeBuf, sizeof(timeBuf), p_format->monotonic_output ? "%6lld" : "%19lld",
             (long long)now);
  } else {
#if !defined(_WIN32)
    ptm = localtime_r(&now, &tmBuf);
#else
    ptm = localtime(&now);
#endif
    strftime(timeBuf, sizeof(timeBuf), &"%Y-%m-%d %H:%M:%S"[p_format->year_output ? 0 : 3], ptm);
  }
  len = strlen(timeBuf);
  if (p_format->nsec_time_output) {
    len += snprintf(timeBuf + len, sizeof(timeBuf) - len, ".%09ld", nsec);
  } else if (p_format->usec_time_output) {
    len += snprintf(timeBuf + len, sizeof(timeBuf) - len, ".%06ld", nsec / US_PER_NSEC);
  } else {
    len += snprintf(timeBuf + len, sizeof(timeBuf) - len, ".%03ld", nsec / MS_PER_NSEC);
  }
  if (p_format->zone_output && ptm) {
    strftime(timeBuf + len, sizeof(timeBuf) - len, " %z", ptm);
  }

  /*
   * Construct a buffer containing the log header and log message.
   */
  if (p_format->colored_output) {
    prefixLen =
        snprintf(prefixBuf, sizeof(prefixBuf), "\x1B[%dm", colorFromPri(entry->priority));
    prefixLen = MIN(prefixLen, sizeof(prefixBuf));

    const char suffixContents[] = "\x1B[0m";
    strcpy(suffixBuf, suffixContents);
    suffixLen = strlen(suffixContents);
  }

  char uid[16];
  uid[0] = '\0';
  if (p_format->uid_output) {
    if (entry->uid >= 0) {
/*
 * This code is Android specific, bionic guarantees that
 * calls to non-reentrant getpwuid() are thread safe.
 */
#ifdef __ANDROID__
      struct passwd* pwd = getpwuid(entry->uid);
      if (pwd && (strlen(pwd->pw_name) <= 5)) {
        snprintf(uid, sizeof(uid), "%5s:", pwd->pw_name);
      } else
#endif
      {
        /* Not worth parsing package list, names all longer than 5 */
        snprintf(uid, sizeof(uid), "%5d:", entry->uid);
      }
    } else {
      snprintf(uid, sizeof(uid), "      ");
    }
  }

  switch (p_format->format) {
    case FORMAT_TAG:
      len = snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen, "%c/%-8.*s: ", priChar,
                     (int)entry->tagLen, entry->tag);
      strcpy(suffixBuf + suffixLen, "\n");
      ++suffixLen;
      break;
    case FORMAT_PROCESS:
      len = snprintf(suffixBuf + suffixLen, sizeof(suffixBuf) - suffixLen, "  (%.*s)\n",
                     (int)entry->tagLen, entry->tag);
      suffixLen += MIN(len, sizeof(suffixBuf) - suffixLen);
      len = snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen, "%c(%s%5d) ", priChar,
                     uid, entry->pid);
      break;
    case FORMAT_THREAD:
      len = snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen, "%c(%s%5d:%5d) ",
                     priChar, uid, entry->pid, entry->tid);
      strcpy(suffixBuf + suffixLen, "\n");
      ++suffixLen;
      break;
    case FORMAT_RAW:
      prefixBuf[prefixLen] = 0;
      len = 0;
      strcpy(suffixBuf + suffixLen, "\n");
      ++suffixLen;
      break;
    case FORMAT_TIME:
      len = snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen,
                     "%s %c/%-8.*s(%s%5d): ", timeBuf, priChar, (int)entry->tagLen, entry->tag, uid,
                     entry->pid);
      strcpy(suffixBuf + suffixLen, "\n");
      ++suffixLen;
      break;
    case FORMAT_THREADTIME:
      ret = strchr(uid, ':');
      if (ret) {
        *ret = ' ';
      }
      len = snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen,
                     "%s %s%5d %5d %c %-8.*s: ", timeBuf, uid, entry->pid, entry->tid, priChar,
                     (int)entry->tagLen, entry->tag);
      strcpy(suffixBuf + suffixLen, "\n");
      ++suffixLen;
      break;
    case FORMAT_LONG:
      len = snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen,
                     "[ %s %s%5d:%5d %c/%-8.*s ]\n", timeBuf, uid, entry->pid, entry->tid, priChar,
                     (int)entry->tagLen, entry->tag);
      strcpy(suffixBuf + suffixLen, "\n\n");
      suffixLen += 2;
      prefixSuffixIsHeaderFooter = 1;
      break;
    case FORMAT_BRIEF:
    default:
      len =
          snprintf(prefixBuf + prefixLen, sizeof(prefixBuf) - prefixLen,
                   "%c/%-8.*s(%s%5d): ", priChar, (int)entry->tagLen, entry->tag, uid, entry->pid);
      strcpy(suffixBuf + suffixLen, "\n");
      ++suffixLen;
      break;
  }

  /* snprintf has a weird return value.   It returns what would have been
   * written given a large enough buffer.  In the case that the prefix is
   * longer then our buffer(128), it messes up the calculations below
   * possibly causing heap corruption.  To avoid this we double check and
   * set the length at the maximum (size minus null byte)
   */
  prefixLen += len;
  if (prefixLen >= sizeof(prefixBuf)) {
    prefixLen = sizeof(prefixBuf) - 1;
    prefixBuf[sizeof(prefixBuf) - 1] = '\0';
  }
  if (suffixLen >= sizeof(suffixBuf)) {
    suffixLen = sizeof(suffixBuf) - 1;
    suffixBuf[sizeof(suffixBuf) - 2] = '\n';
    suffixBuf[sizeof(suffixBuf) - 1] = '\0';
  }

  /* the following code is tragically unreadable */

  size_t numLines;
  char* p;
  size_t bufferSize;
  const char* pm;

  if (prefixSuffixIsHeaderFooter) {
    /* we're just wrapping message with a header/footer */
    numLines = 1;
  } else {
    pm = entry->message;
    numLines = 0;

    /*
     * The line-end finding here must match the line-end finding
     * in for ( ... numLines...) loop below
     */
    while (pm < (entry->message + entry->messageLen)) {
      if (*pm++ == '\n') numLines++;
    }
    /* plus one line for anything not newline-terminated at the end */
    if (pm > entry->message && *(pm - 1) != '\n') numLines++;
  }

  /*
   * this is an upper bound--newlines in message may be counted
   * extraneously
   */
  bufferSize = (numLines * (prefixLen + suffixLen)) + 1;
  if (p_format->printable_output) {
    /* Calculate extra length to convert non-printable to printable */
    bufferSize += convertPrintable(NULL, entry->message, entry->messageLen);
  } else {
    bufferSize += entry->messageLen;
  }

  if (defaultBufferSize >= bufferSize) {
    ret = defaultBuffer;
  } else {
    ret = (char*)malloc(bufferSize);

    if (ret == NULL) {
      return ret;
    }
  }

  ret[0] = '\0'; /* to start strcat off */

  p = ret;
  pm = entry->message;

  if (prefixSuffixIsHeaderFooter) {
    strcat(p, prefixBuf);
    p += prefixLen;
    if (p_format->printable_output) {
      p += convertPrintable(p, entry->message, entry->messageLen);
    } else {
      strncat(p, entry->message, entry->messageLen);
      p += entry->messageLen;
    }
    strcat(p, suffixBuf);
    p += suffixLen;
  } else {
    do {
      const char* lineStart;
      size_t lineLen;
      lineStart = pm;

      /* Find the next end-of-line in message */
      while (pm < (entry->message + entry->messageLen) && *pm != '\n') pm++;
      lineLen = pm - lineStart;

      strcat(p, prefixBuf);
      p += prefixLen;
      if (p_format->printable_output) {
        p += convertPrintable(p, lineStart, lineLen);
      } else {
        strncat(p, lineStart, lineLen);
        p += lineLen;
      }
      strcat(p, suffixBuf);
      p += suffixLen;

      if (*pm == '\n') pm++;
    } while (pm < (entry->message + entry->messageLen));
  }

  if (p_outLength != NULL) {
    *p_outLength = p - ret;
  }

  return ret;
}

/**
 * Either print or do not print log line, based on filter
 *
 * Returns count bytes written
 */

int android_log_printLogLine(AndroidLogFormat* p_format, int fd, const AndroidLogEntry* entry) {
  int ret;
  char defaultBuffer[512];
  char* outBuffer = NULL;
  size_t totalLen;

  outBuffer =
      android_log_formatLogLine(p_format, defaultBuffer, sizeof(defaultBuffer), entry, &totalLen);

  if (!outBuffer) return -1;

  do {
    ret = write(fd, outBuffer, totalLen);
  } while (ret < 0 && errno == EINTR);

  if (ret < 0) {
    fprintf(stderr, "+++ LOG: write failed (errno=%d)\n", errno);
    ret = 0;
    goto done;
  }

  if (((size_t)ret) < totalLen) {
    fprintf(stderr, "+++ LOG: write partial (%d of %d)\n", ret, (int)totalLen);
    goto done;
  }

done:
  if (outBuffer != defaultBuffer) {
    free(outBuffer);
  }

  return ret;
}
