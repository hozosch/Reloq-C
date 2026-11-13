#ifndef RELOQ_DICTIONARY_NOTIFICATION_H
#define RELOQ_DICTIONARY_NOTIFICATION_H

#include <windows.h>

/* Display a temporary Shell notification from a background worker. */
BOOL sapi_dictionary_notify(const WCHAR *title,const WCHAR *text);

#endif
