/* Rule diagnostics belong to development builds. Release calls must not pack
 * arguments or update global tracing state on the speech path. */
#ifndef RELOQ61_RULE_TRACE_H
#define RELOQ61_RULE_TRACE_H
#if defined(EVV_RULE_DIAGNOSTICS) || !defined(EVV_NO_BYTECODE)
#define EVV_RULE_TRACE_ENABLED 1
extern int delta_rule_trace;
#else
#define EVV_RULE_TRACE_ENABLED 0
#define delta_rule_trace 0
#endif
#endif
