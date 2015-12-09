#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sisci_api.h>
#include "reporting.h"


#define BUFLEN 1024



/* Verbosity level 
 *
 * 0 = errors only,
 * 1 = warnings and errors
 * 2 = warnings, errors and informatives
 * 3 = everything above + debug
 */
unsigned verbosity = 0;



/* SISCI error codes 
 *
 * Use this to lookup the correct index in the error string table.
 */
static unsigned error_codes[] = {
    SCI_ERR_OK,
    SCI_ERR_BUSY,
    SCI_ERR_FLAG_NOT_IMPLEMENTED,
    SCI_ERR_ILLEGAL_FLAG,
    SCI_ERR_NOSPC,
    SCI_ERR_API_NOSPC,
    SCI_ERR_HW_NOSPC,
    SCI_ERR_NOT_IMPLEMENTED,
    SCI_ERR_ILLEGAL_ADAPTERNO,
    SCI_ERR_NO_SUCH_ADAPTERNO,
    SCI_ERR_TIMEOUT,
    SCI_ERR_OUT_OF_RANGE,
    SCI_ERR_NO_SUCH_SEGMENT,
    SCI_ERR_ILLEGAL_NODEID,
    SCI_ERR_CONNECTION_REFUSED,
    SCI_ERR_SEGMENT_NOT_CONNECTED,
    SCI_ERR_SIZE_ALIGNMENT,
    SCI_ERR_OFFSET_ALIGNMENT,
    SCI_ERR_ILLEGAL_PARAMETER,
    SCI_ERR_MAX_ENTRIES,
    SCI_ERR_SEGMENT_NOT_PREPARED,
    SCI_ERR_ILLEGAL_ADDRESS,
    SCI_ERR_ILLEGAL_OPERATION,
    SCI_ERR_ILLEGAL_QUERY,
    SCI_ERR_SEGMENTID_USED,
    SCI_ERR_SYSTEM,
    SCI_ERR_CANCELLED,
    SCI_ERR_NOT_CONNECTED,
    SCI_ERR_NOT_AVAILABLE,
    SCI_ERR_INCONSISTENT_VERSIONS,
    SCI_ERR_COND_INT_RACE_PROBLEM,
    SCI_ERR_OVERFLOW,
    SCI_ERR_NOT_INITIALIZED,
    SCI_ERR_ACCESS,
    SCI_ERR_NOT_SUPPORTED,
    SCI_ERR_DEPRECATED,
    SCI_ERR_NO_SUCH_NODEID,
    SCI_ERR_NODE_NOT_RESPONDING,
    SCI_ERR_NO_REMOTE_LINK_ACCESS,
    SCI_ERR_NO_LINK_ACCESS,
    SCI_ERR_TRANSFER_FAILED,
    SCI_ERR_EWOULD_BLOCK,
    SCI_ERR_SEMAPHORE_COUNT_EXCEEDED,
    SCI_ERR_IRQL_ILLEGAL,
    SCI_ERR_REMOTE_BUSY,
    SCI_ERR_LOCAL_BUSY,
    SCI_ERR_ALL_BUSY
};



/* Corresponding error strings */
static const char* error_strings[] = {
    "OK",
    "Resource busy",
    "Flag option is not implemented",
    "Illegal flag option",
    "Out of local resources",
    "Out of local API resources",
    "Out of hardware resources",
    "Not implemented",
    "Illegal adapter number",
    "Adapter not found",
    "Operation timed out",
    "Out of range",
    "Segment ID not found",
    "Illegal node ID",
    "Connection to remote node is refused",
    "No connection to segment",
    "Size is not aligned",
    "Offset is not aligned",
    "Illegal function parameter",
    "Maximum possible physical mapping is exceeded",
    "Segment is not prepared",
    "Illegal address",
    "Illegal operation",
    "Illegal query operation",
    "Segment ID already used",
    "Could not get requested resource from the system",
    "Operation cancelled",
    "Host is not connected to remote host",
    "Operation not available",
    "Inconsistent driver version",
    "Out of local resources",
    "Host not initialized",
    "No local or remote access for requested operation",
    "Request not supported",
    "Function deprecated",
    "Node ID not found",
    "Node does not respond",
    "Remote link is not operational",
    "Local link is not operational",
    "Transfer failed",
    "Illegal interrupt line",
    "Remote host is busy",
    "Local host is busy",
    "System is busy"
};



/* Lookup error string from SISCI error code */
const char* SCIGetErrorString(sci_error_t code)
{
    const size_t len = sizeof(error_codes) / sizeof(error_codes[0]);

    for (size_t idx = 0; idx < len; ++idx)
    {
        if (error_codes[idx] == code)
        {
            return error_strings[idx];
        }
    }

    return "Unknown error";
}



void log_info(const char* frmt, ...)
{
    if (verbosity >= 2)
    {
        va_list args;
        char buff[BUFLEN];
        size_t size;

        va_start(args, frmt);
        size = vsnprintf(buff, sizeof(buff), frmt, args);
        va_end(args);

        fwrite("INFO   : ", 9, 1, stderr);
        fwrite(buff, size, 1, stderr);
        fwrite("\n", 1, 1, stderr);
        fflush(stderr);
    }
}



void log_warn(const char* frmt, ...)
{
    if (verbosity >= 1)
    {
        va_list args;
        char buff[BUFLEN];
        size_t size;

        va_start(args, frmt);
        size = vsnprintf(buff, sizeof(buff), frmt, args);
        va_end(args);

        fwrite("WARNING: ", 9, 1, stderr);
        fwrite(buff, size, 1, stderr);
        fwrite("\n", 1, 1, stderr);
        fflush(stderr);
    }
}



void log_error(const char* frmt, ...)
{
    va_list args;
    char buff[BUFLEN];
    size_t size;

    va_start(args, frmt);
    size = vsnprintf(buff, sizeof(buff), frmt, args);
    va_end(args);

    fwrite("ERROR  : ", 9, 1, stderr);
    fwrite(buff, size, 1, stderr);
    fwrite("\n", 1, 1, stderr);
    fflush(stderr);
}



void log_debug(const char* frmt, ...)
{
    if (verbosity >= 3)
    {
        va_list args;
        char buff[BUFLEN];
        size_t size;

        va_start(args, frmt);
        size = vsnprintf(buff, sizeof(buff), frmt, args);
        va_end(args);

        fwrite("DEBUG  : ", 9, 1, stderr);
        fwrite(buff, size, 1, stderr);
        fwrite("\n", 1, 1, stderr);
        fflush(stderr);
    }

}



//void log_phaddr(sci_local_segment_t segment)
//{
//    if (verbosity >= 3)
//    {
//        sci_error_t err;
//        sci_query_local_segment_t query;
//
//        query.subcommand = SCI_Q_LOCAL_SEGMENT_PHYS_ADDR;
//        query.segment = segment;
//
//        SCIQuery(SCI_Q_LOCAL_SEGMENT, &query, 0, &err);
//        if (err != SCI_ERR_OK)
//        {
//            log_error("Failed to query local segment: %s", SCIGetErrorString(err));
//            return;
//        }
//
//        log_debug("Local segment has phys addr: %lu", query.data.ioaddr);
//    }
//}



void log_ioaddr(sci_local_segment_t segment)
{
    // TODO: Move this to main.cu and make a function there to get GPU name + bus addr

    if (verbosity >= 3)
    {
        sci_error_t err;
        sci_query_local_segment_t query;

        query.subcommand = SCI_Q_LOCAL_SEGMENT_IOADDR;
        query.segment = segment;

        SCIQuery(SCI_Q_LOCAL_SEGMENT, &query, 0, &err);
        if (err != SCI_ERR_OK)
        {
            log_error("Failed to query local segment: %s", SCIGetErrorString(err));
            return;
        }

        log_debug("Local segment has IO addr: 0x%08lx", query.data.ioaddr);
    }
}