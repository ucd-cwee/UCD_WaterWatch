/*
WaterWatch, Energy Demand Management System for Water Systems. For further information on WaterWatch, please see https://cwee.ucdavis.edu/waterwatch.

Copyright (c) 2019 - 2023 by Robert Good, Erin Musabandesu, and David Linz. (Email: rtgood@ucdavis.edu). All rights reserved.

Created: RTG	/	2023
History: RTG	/	2023		1. Initial Release.

Copyright / Usage Details: You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise)
when your product is released in binary form. You are allowed to modify the source code in any way you want
except you cannot modify the copyright details at the top of each module. If you want to distribute source
code with your application, then you are only allowed to distribute versions released by the author. This is
to maintain a single distribution point for the source code.
*/

#pragma once
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define includeZeroMQ

#include "../WaterWatchCpp/Precompiled.h"
#include "../WaterWatchCpp/enum.h"
#include "../WaterWatchCpp/Strings.h"
#include "../WaterWatchCpp/vec.h"
#include "../WaterWatchCpp/SharedPtr.h"
#include "../WaterWatchCpp/List.h"
#include "../WaterWatchCpp/cweeTime.h"
#include "../WaterWatchCpp/UnorderedList.h"
#include "../WaterWatchCpp/DispatchTimer.h"
#include "../WaterWatchCpp/cweeJob.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// Easy CURL operations
class cweeCURL {
public:
	enum class CURLoption {
		/* This is the FILE * or void * the regular output should be written to. */
		CURLOPT_WRITEDATA = 1,

		/* The full URL to get/put */
		CURLOPT_URL = 2,

		/* Port number to connect to,  if other than default. */
		CURLOPT_PORT = 3,

		/* Name of proxy to use. */
		CURLOPT_PROXY = 4,

		/* "user:password, options" to use when fetching. */
		CURLOPT_USERPWD = 5,

		/* "user:password" to use with proxy. */
		CURLOPT_PROXYUSERPWD = 6,

		/* Range to get,  specified as an ASCII string. */
		CURLOPT_RANGE = 7,

		/* not used */

		/* Specified file stream to upload from (use as input): */
		CURLOPT_READDATA = 9,

		/* Buffer to receive error messages in,  must be at least CURL_ERROR_SIZE
		* bytes big. */
		CURLOPT_ERRORBUFFER = 10,

		/* Function that will be called to store the output (instead of fwrite). The
		* parameters will use fwrite() syntax,  make sure to follow them. */
		CURLOPT_WRITEFUNCTION = 11,

		/* Function that will be called to read the input (instead of fread). The
		* parameters will use fread() syntax,  make sure to follow them. */
		CURLOPT_READFUNCTION = 12,

		/* Time-out the read operation after this amount of seconds */
		CURLOPT_TIMEOUT = 13,

		/* If the CURLOPT_INFILE is used,  this can be used to inform libcurl about
		* how large the file being sent really is. That allows better error
		* checking and better verifies that the upload was successful. -1 means
		* unknown size.
		*
		* For large file support,  there is also a _LARGE version of the key
		* which takes an off_t type,  allowing platforms with larger off_t
		* sizes to handle larger files.  See below for INFILESIZE_LARGE.
		*/
		CURLOPT_INFILESIZE = 14,

		/* POST static input fields. */
		CURLOPT_POSTFIELDS = 15,

		/* Set the referrer page (needed by some CGIs) */
		CURLOPT_REFERER = 16,

		/* Set the FTP PORT string (interface name,  named or numerical IP address)
		Use i.e '-' to use default address. */
		CURLOPT_FTPPORT = 17,

		/* Set the User-Agent string (examined by some CGIs) */
		CURLOPT_USERAGENT = 18,

		/* If the download receives less than "low speed limit" bytes/second
		* during "low speed time" seconds,  the operations is aborted.
		* You could i.e if you have a pretty high speed connection,  abort if
		* it is less than 2000 bytes/sec during 20 seconds.
		*/

		/* Set the "low speed limit" */
		CURLOPT_LOW_SPEED_LIMIT = 19,

		/* Set the "low speed time" */
		CURLOPT_LOW_SPEED_TIME = 20,

		/* Set the continuation offset.
		*
		* Note there is also a _LARGE version of this key which uses
		* off_t types,  allowing for large file offsets on platforms which
		* use larger-than-32-bit off_t's.  Look below for RESUME_FROM_LARGE.
		*/
		CURLOPT_RESUME_FROM = 21,

		/* Set cookie in request: */
		CURLOPT_COOKIE = 22,

		/* This points to a linked list of headers,  struct curl_slist kind. This
		list is also used for RTSP (in spite of its name) */
		CURLOPT_HTTPHEADER = 23,

		/* This points to a linked list of post entries,  struct curl_httppost */
		CURLOPT_HTTPPOST = 24,

		/* name of the file keeping your private SSL-certificate */
		CURLOPT_SSLCERT = 25,

		/* password for the SSL or SSH private key */
		CURLOPT_KEYPASSWD = 26,

		/* send TYPE parameter? */
		CURLOPT_CRLF = 27,

		/* send linked-list of QUOTE commands */
		CURLOPT_QUOTE = 28,

		/* send FILE * or void * to store headers to,  if you use a callback it
		is simply passed to the callback unmodified */
		CURLOPT_HEADERDATA = 29,

		/* point to a file to read the initial cookies from,  also enables
		"cookie awareness" */
		CURLOPT_COOKIEFILE = 31,

		/* What version to specifically try to use.
		See CURL_SSLVERSION defines below. */
		CURLOPT_SSLVERSION = 32,

		/* What kind of HTTP time condition to use,  see defines */
		CURLOPT_TIMECONDITION = 33,

		/* Time to use with the above condition. Specified in number of seconds
		since 1 Jan 1970 */
		CURLOPT_TIMEVALUE = 34,

		/* 35 = OBSOLETE */

		/* Custom request,  for customizing the get command like
		HTTP: DELETE,  TRACE and others
		FTP: to use a different list command
		*/
		CURLOPT_CUSTOMREQUEST = 36,

		/* FILE handle to use instead of stderr */
		CURLOPT_STDERR = 37,

		/* 38 is not used */

		/* send linked-list of post-transfer QUOTE commands */
		CURLOPT_POSTQUOTE = 39,

		// CURLOPT_OBSOLETE = 40, /* = do not use! */

		CURLOPT_VERBOSE = 41,     /* talk a lot */
		CURLOPT_HEADER = 42,      /* throw the header out too */
		CURLOPT_NOPROGRESS = 43,  /* shut off the progress meter */
		CURLOPT_NOBODY = 44,      /* use HEAD to get http document */
		CURLOPT_FAILONERROR = 45, /* no output on http error codes >= 400 */
		CURLOPT_UPLOAD = 46,      /* this is an upload */
		CURLOPT_POST = 47,        /* HTTP POST method */
		CURLOPT_DIRLISTONLY = 48,  /* bare names when listing directories */

		CURLOPT_APPEND = 50,      /* Append instead of overwrite on upload! */

		/* Specify whether to read the user+password from the .netrc or the URL.
		* This must be one of the CURL_NETRC_* enums below. */
		CURLOPT_NETRC = 51,

		CURLOPT_FOLLOWLOCATION = 52, /* use Location: Luke! */

		CURLOPT_TRANSFERTEXT = 53, /* transfer data in text/ASCII format */
		CURLOPT_PUT = 54,         /* HTTP PUT */

		/* 55 = OBSOLETE */

		/* DEPRECATED
		* Function that will be called instead of the internal progress display
		* function. This function should be defined as the curl_progress_callback
		* prototype defines. */
		CURLOPT_PROGRESSFUNCTION = 56,

		/* Data passed to the CURLOPT_PROGRESSFUNCTION and CURLOPT_XFERINFOFUNCTION
		callbacks */
		CURLOPT_PROGRESSDATA = 57,
#define CURLOPT_XFERINFODATA CURLOPT_PROGRESSDATA

		/* We want the referrer field set automatically when following locations */
		CURLOPT_AUTOREFERER = 58,

		/* Port of the proxy,  can be set in the proxy string as well with:
		"[host]:[port]" */
		CURLOPT_PROXYPORT = 59,

		/* size of the POST input data,  if strlen() is not good to use */
		CURLOPT_POSTFIELDSIZE = 60,

		/* tunnel non-http operations through a HTTP proxy */
		CURLOPT_HTTPPROXYTUNNEL = 61,

		/* Set the interface string to use as outgoing network interface */
		CURLOPT_INTERFACE = 62,

		/* Set the krb4/5 security level,  this also enables krb4/5 awareness.  This
		* is a string = 'confidential' or 'private'.  If the string
		* is set but doesn't match one of these,  'private' will be used.  */
		CURLOPT_KRBLEVEL = 63,

		/* Set if we should verify the peer in ssl handshake,  set 1 to verify. */
		CURLOPT_SSL_VERIFYPEER = 64,

		/* The CApath or CAfile used to validate the peer certificate
		this option is used only if SSL_VERIFYPEER is true */
		CURLOPT_CAINFO = 65,

		/* 66 = OBSOLETE */
		/* 67 = OBSOLETE */

		/* Maximum number of http redirects to follow */
		CURLOPT_MAXREDIRS = 68,

		/* Pass a long set to 1 to get the date of the requested document (if
		possible)! Pass a zero to shut it off. */
		CURLOPT_FILETIME = 69,

		/* This points to a linked list of telnet options */
		CURLOPT_TELNETOPTIONS = 70,

		/* Max amount of cached alive connections */
		CURLOPT_MAXCONNECTS = 71,

		// CURLOPT_OBSOLETE = 72, /* = do not use!*/

		/* 73 = OBSOLETE */

		/* Set to explicitly use a new connection for the upcoming transfer.
		Do not use this unless you're absolutely sure of this,  as it makes the
		operation slower and is less friendly for the network. */
		CURLOPT_FRESH_CONNECT = 74,

		/* Set to explicitly forbid the upcoming transfer's connection to be re-used
		when done. Do not use this unless you're absolutely sure of this,  as it
		makes the operation slower and is less friendly for the network. */
		CURLOPT_FORBID_REUSE = 75,

		/* Set to a file name that contains random data for libcurl to use to
		seed the random engine when doing SSL connects. */
		CURLOPT_RANDOM_FILE = 76,

		/* Set to the Entropy Gathering Daemon socket pathname */
		CURLOPT_EGDSOCKET = 77,

		/* Time-out connect operations after this amount of seconds,  if connects are
		OK within this time,  then fine... This only aborts the connect phase. */
		CURLOPT_CONNECTTIMEOUT = 78,

		/* Function that will be called to store headers (instead of fwrite). The
		* parameters will use fwrite() syntax,  make sure to follow them. */
		CURLOPT_HEADERFUNCTION = 79,

		/* Set this to force the HTTP request to get back to GET. Only really usable
		if POST,  PUT or a custom request have been used first.
		*/
		CURLOPT_HTTPGET = 80,

		/* Set if we should verify the Common name from the peer certificate in ssl
		* handshake = 2 to ensure that it matches the
		* provided hostname. */
		CURLOPT_SSL_VERIFYHOST = 81,

		/* Specify which file name to write all known cookies in after completed
		operation. Set file name to "-" (dash) to make it go to stdout. */
		CURLOPT_COOKIEJAR = 82,

		/* Specify which SSL ciphers to use */
		CURLOPT_SSL_CIPHER_LIST = 83,

		/* Specify which HTTP version to use! This must be set to one of the
		CURL_HTTP_VERSION* enums set below. */
		CURLOPT_HTTP_VERSION = 84,

		/* Specifically switch on or off the FTP engine's use of the EPSV command. By
		default,  that one will always be attempted before the more traditional
		PASV command. */
		CURLOPT_FTP_USE_EPSV = 85,

		/* type of the file keeping your SSL-certificate ("DER" = "ENG") */
		CURLOPT_SSLCERTTYPE = 86,

		/* name of the file keeping your private SSL-key */
		CURLOPT_SSLKEY = 87,

		/* type of the file keeping your private SSL-key ("DER" = "ENG") */
		CURLOPT_SSLKEYTYPE = 88,

		/* crypto engine for the SSL-sub system */
		CURLOPT_SSLENGINE = 89,

		/* set the crypto engine for the SSL-sub system as default
		the param has no meaning...
		*/
		CURLOPT_SSLENGINE_DEFAULT = 90,

		/* Non-zero value means to use the global dns cache */
		CURLOPT_DNS_USE_GLOBAL_CACHE = 91, /* = do not use! */

		/* DNS cache timeout */
		CURLOPT_DNS_CACHE_TIMEOUT = 92,

		/* send linked-list of pre-transfer QUOTE commands */
		CURLOPT_PREQUOTE = 93,

		/* set the debug function */
		CURLOPT_DEBUGFUNCTION = 94,

		/* set the data for the debug function */
		CURLOPT_DEBUGDATA = 95,

		/* mark this as start of a cookie session */
		CURLOPT_COOKIESESSION = 96,

		/* The CApath directory used to validate the peer certificate
		this option is used only if SSL_VERIFYPEER is true */
		CURLOPT_CAPATH = 97,

		/* Instruct libcurl to use a smaller receive buffer */
		CURLOPT_BUFFERSIZE = 98,

		/* Instruct libcurl to not use any signal/alarm handlers,  even when using
		timeouts. This option is useful for multi-threaded applications.
		See libcurl-the-guide for more background information. */
		CURLOPT_NOSIGNAL = 99,

		/* Provide a CURLShare for mutexing non-ts data */
		CURLOPT_SHARE = 100,

		/* indicates type of proxy. accepted values are CURLPROXY_HTTP (default,
		CURLPROXY_HTTPS = CURLPROXY_SOCKS4A and
		CURLPROXY_SOCKS5. */
		CURLOPT_PROXYTYPE = 101,

		/* Set the Accept-Encoding string. Use this to tell a server you would like
		the response to be compressed. Before 7.21.6,  this was known as
		CURLOPT_ENCODING */
		CURLOPT_ACCEPT_ENCODING = 102,

		/* Set pointer to private data */
		CURLOPT_PRIVATE = 103,

		/* Set aliases for HTTP 200 in the HTTP Response header */
		CURLOPT_HTTP200ALIASES = 104,

		/* Continue to send authentication (user+password) when following locations,
		even when hostname changed. This can potentially send off the name
		and password to whatever host the server decides. */
		CURLOPT_UNRESTRICTED_AUTH = 105,

		/* Specifically switch on or off the FTP engine's use of the EPRT command (
		it also disables the LPRT attempt). By default,  those ones will always be
		attempted before the good old traditional PORT command. */
		CURLOPT_FTP_USE_EPRT = 106,

		/* Set this to a bitmask value to enable the particular authentications
		methods you like. Use this in combination with CURLOPT_USERPWD.
		Note that setting multiple bits may cause extra network round-trips. */
		CURLOPT_HTTPAUTH = 107,

		/* Set the ssl context callback function,  currently only for OpenSSL or
		WolfSSL ssl_ctx,  or mbedTLS mbedtls_ssl_config in the second argument.
		The function must match the curl_ssl_ctx_callback prototype. */
		CURLOPT_SSL_CTX_FUNCTION = 108,

		/* Set the userdata for the ssl context callback function's third
		argument */
		CURLOPT_SSL_CTX_DATA = 109,

		/* FTP Option that causes missing dirs to be created on the remote server.
		In 7.19.4 we introduced the convenience enums for this option using the
		CURLFTP_CREATE_DIR prefix.
		*/
		CURLOPT_FTP_CREATE_MISSING_DIRS = 110,

		/* Set this to a bitmask value to enable the particular authentications
		methods you like. Use this in combination with CURLOPT_PROXYUSERPWD.
		Note that setting multiple bits may cause extra network round-trips. */
		CURLOPT_PROXYAUTH = 111,

		/* FTP option that changes the timeout = associated with
		getting a response.  This is different from transfer timeout time and
		essentially places a demand on the FTP server to acknowledge commands
		in a timely manner. */
		CURLOPT_FTP_RESPONSE_TIMEOUT = 112,
#define CURLOPT_SERVER_RESPONSE_TIMEOUT CURLOPT_FTP_RESPONSE_TIMEOUT

		/* Set this option to one of the CURL_IPRESOLVE_* defines (see below) to
		tell libcurl to resolve names to those IP versions only. This only has
		affect on systems with support for more than one,  i.e IPv4 _and_ IPv6. */
		CURLOPT_IPRESOLVE = 113,

		/* Set this option to limit the size of a file that will be downloaded from
		an HTTP or FTP server.

		Note there is also _LARGE version which adds large file support for
		platforms which have larger off_t sizes.  See MAXFILESIZE_LARGE below. */
		CURLOPT_MAXFILESIZE = 114,

		/* See the comment for INFILESIZE above = specifies
		* the size of the file being uploaded.  -1 means unknown.
		*/
		CURLOPT_INFILESIZE_LARGE = 115,

		/* Sets the continuation offset.  There is also a LONG version of this,
		* look above for RESUME_FROM.
		*/
		CURLOPT_RESUME_FROM_LARGE = 116,

		/* Sets the maximum size of data that will be downloaded from
		* an HTTP or FTP server.  See MAXFILESIZE above for the LONG version.
		*/
		CURLOPT_MAXFILESIZE_LARGE = 117,

		/* Set this option to the file name of your .netrc file you want libcurl
		to parse (using the CURLOPT_NETRC option). If not set,  libcurl will do
		a poor attempt to find the user's home directory and check for a .netrc
		file in there. */
		CURLOPT_NETRC_FILE = 118,

		/* Enable SSL/TLS for FTP,  pick one of:
		CURLUSESSL_TRY     - try using SSL,  proceed anyway otherwise
		CURLUSESSL_CONTROL - SSL for the control connection or fail
		CURLUSESSL_ALL     - SSL for all communication or fail
		*/
		CURLOPT_USE_SSL = 119,

		/* The _LARGE version of the standard POSTFIELDSIZE option */
		CURLOPT_POSTFIELDSIZE_LARGE = 120,

		/* Enable/disable the TCP Nagle algorithm */
		CURLOPT_TCP_NODELAY = 121,

		/* 122 OBSOLETE,  used in 7.12.3. Gone in 7.13.0 */
		/* 123 OBSOLETE. Gone in 7.16.0 */
		/* 124 OBSOLETE,  used in 7.12.3. Gone in 7.13.0 */
		/* 125 OBSOLETE,  used in 7.12.3. Gone in 7.13.0 */
		/* 126 OBSOLETE,  used in 7.12.3. Gone in 7.13.0 */
		/* 127 OBSOLETE. Gone in 7.16.0 */
		/* 128 OBSOLETE. Gone in 7.16.0 */

		/* When FTP over SSL/TLS is selected (with CURLOPT_USE_SSL),  this option
		can be used to change libcurl's default action which is to first try
		"AUTH SSL" and then "AUTH TLS" in this order,  and proceed when a OK
		response has been received.

		Available parameters are:
		CURLFTPAUTH_DEFAULT - let libcurl decide
		CURLFTPAUTH_SSL     - try "AUTH SSL" first,  then TLS
		CURLFTPAUTH_TLS     - try "AUTH TLS" first,  then SSL
		*/
		CURLOPT_FTPSSLAUTH = 129,

		CURLOPT_IOCTLFUNCTION = 130,
		CURLOPT_IOCTLDATA = 131,

		/* 132 OBSOLETE. Gone in 7.16.0 */
		/* 133 OBSOLETE. Gone in 7.16.0 */

		/* zero terminated string for pass on to the FTP server when asked for
		"account" info */
		CURLOPT_FTP_ACCOUNT = 134,

		/* feed cookie into cookie engine */
		CURLOPT_COOKIELIST = 135,

		/* ignore Content-Length */
		CURLOPT_IGNORE_CONTENT_LENGTH = 136,

		/* Set to non-zero to skip the IP address received in a 227 PASV FTP server
		response. Typically used for FTP-SSL purposes but is not restricted to
		that. libcurl will then instead use the same IP address it used for the
		control connection. */
		CURLOPT_FTP_SKIP_PASV_IP = 137,

		/* Select "file method" to use when doing FTP,  see the curl_ftpmethod
		above. */
		CURLOPT_FTP_FILEMETHOD = 138,

		/* Local port number to bind the socket to */
		CURLOPT_LOCALPORT = 139,

		/* Number of ports to try,  including the first one set with LOCALPORT.
		Thus,  setting it to 1 will make no additional attempts but the first.
		*/
		CURLOPT_LOCALPORTRANGE = 140,

		/* no transfer,  set up connection and let application use the socket by
		extracting it with CURLINFO_LASTSOCKET */
		CURLOPT_CONNECT_ONLY = 141,

		/* Function that will be called to convert from the
		network encoding (instead of using the iconv calls in libcurl) */
		CURLOPT_CONV_FROM_NETWORK_FUNCTION = 142,

		/* Function that will be called to convert to the
		network encoding (instead of using the iconv calls in libcurl) */
		CURLOPT_CONV_TO_NETWORK_FUNCTION = 143,

		/* Function that will be called to convert from UTF8
		(instead of using the iconv calls in libcurl)
		Note that this is used only for SSL certificate processing */
		CURLOPT_CONV_FROM_UTF8_FUNCTION = 144,

		/* if the connection proceeds too quickly then need to slow it down */
		/* limit-rate: maximum number of bytes per second to send or receive */
		CURLOPT_MAX_SEND_SPEED_LARGE = 145,
		CURLOPT_MAX_RECV_SPEED_LARGE = 146,

		/* Pointer to command string to send if USER/PASS fails. */
		CURLOPT_FTP_ALTERNATIVE_TO_USER = 147,

		/* callback function for setting socket options */
		CURLOPT_SOCKOPTFUNCTION = 148,
		CURLOPT_SOCKOPTDATA = 149,

		/* set to 0 to disable session ID re-use for this transfer,  default is
		enabled (== 1) */
		CURLOPT_SSL_SESSIONID_CACHE = 150,

		/* allowed SSH authentication methods */
		CURLOPT_SSH_AUTH_TYPES = 151,

		/* Used by scp/sftp to do public/private key authentication */
		CURLOPT_SSH_PUBLIC_KEYFILE = 152,
		CURLOPT_SSH_PRIVATE_KEYFILE = 153,

		/* Send CCC (Clear Command Channel) after authentication */
		CURLOPT_FTP_SSL_CCC = 154,

		/* Same as TIMEOUT and CONNECTTIMEOUT,  but with ms resolution */
		CURLOPT_TIMEOUT_MS = 155,
		CURLOPT_CONNECTTIMEOUT_MS = 156,

		/* set to zero to disable the libcurl's decoding and thus pass the raw body
		data to the application even when it is encoded/compressed */
		CURLOPT_HTTP_TRANSFER_DECODING = 157,
		CURLOPT_HTTP_CONTENT_DECODING = 158,

		/* Permission used when creating new files and directories on the remote
		server for protocols that support it,  SFTP/SCP/FILE */
		CURLOPT_NEW_FILE_PERMS = 159,
		CURLOPT_NEW_DIRECTORY_PERMS = 160,

		/* Set the behaviour of POST when redirecting. Values must be set to one
		of CURL_REDIR* defines below. This used to be called CURLOPT_POST301 */
		CURLOPT_POSTREDIR = 161,

		/* used by scp/sftp to verify the host's public key */
		CURLOPT_SSH_HOST_PUBLIC_KEY_MD5 = 162,

		/* Callback function for opening socket (instead of socket(2)). Optionally,
		callback is able change the address or refuse to connect returning
		CURL_SOCKET_BAD.  The callback should have type
		curl_opensocket_callback */
		CURLOPT_OPENSOCKETFUNCTION = 163,
		CURLOPT_OPENSOCKETDATA = 164,

		/* POST volatile input fields. */
		CURLOPT_COPYPOSTFIELDS = 165,

		/* set transfer mode (, type=<a|i>) when doing FTP via an HTTP proxy */
		CURLOPT_PROXY_TRANSFER_MODE = 166,

		/* Callback function for seeking in the input stream */
		CURLOPT_SEEKFUNCTION = 167,
		CURLOPT_SEEKDATA = 168,

		/* CRL file */
		CURLOPT_CRLFILE = 169,

		/* Issuer certificate */
		CURLOPT_ISSUERCERT = 170,

		/* (IPv6) Address scope */
		CURLOPT_ADDRESS_SCOPE = 171,

		/* Collect certificate chain info and allow it to get retrievable with
		CURLINFO_CERTINFO after the transfer is complete. */
		CURLOPT_CERTINFO = 172,

		/* "name" and "pwd" to use when fetching. */
		CURLOPT_USERNAME = 173,
		CURLOPT_PASSWORD = 174,

		/* "name" and "pwd" to use with Proxy when fetching. */
		CURLOPT_PROXYUSERNAME = 175,
		CURLOPT_PROXYPASSWORD = 176,

		/* Comma separated list of hostnames defining no-proxy zones. These should
		match both hostnames directly,  and hostnames within a domain. For
		example = but NOT
		notlocal.com or www.notlocal.com. For compatibility with other
		implementations of this,  .local.com will be considered to be the same as
		local.com. A single * is the only valid wildcard,  and effectively
		disables the use of proxy. */
		CURLOPT_NOPROXY = 177,

		/* block size for TFTP transfers */
		CURLOPT_TFTP_BLKSIZE = 178,

		/* Socks Service */
		CURLOPT_SOCKS5_GSSAPI_SERVICE = 179, /* = do not use!*/

		/* Socks Service */
		CURLOPT_SOCKS5_GSSAPI_NEC = 180,

		/* set the bitmask for the protocols that are allowed to be used for the
		transfer,  which thus helps the app which takes URLs from users or other
		external inputs and want to restrict what protocol(s) to deal
		with. Defaults to CURLPROTO_ALL. */
		CURLOPT_PROTOCOLS = 181,

		/* set the bitmask for the protocols that libcurl is allowed to follow to,
		as a subset of the CURLOPT_PROTOCOLS ones. That means the protocol needs
		to be set in both bitmasks to be allowed to get redirected to. */
		CURLOPT_REDIR_PROTOCOLS = 182,

		/* set the SSH knownhost file name to use */
		CURLOPT_SSH_KNOWNHOSTS = 183,

		/* set the SSH host key callback,  must point to a curl_sshkeycallback
		function */
		CURLOPT_SSH_KEYFUNCTION = 184,

		/* set the SSH host key callback custom pointer */
		CURLOPT_SSH_KEYDATA = 185,

		/* set the SMTP mail originator */
		CURLOPT_MAIL_FROM = 186,

		/* set the list of SMTP mail receiver(s) */
		CURLOPT_MAIL_RCPT = 187,

		/* FTP: send PRET before PASV */
		CURLOPT_FTP_USE_PRET = 188,

		/* RTSP request method (OPTIONS = etc...) */
		CURLOPT_RTSP_REQUEST = 189,

		/* The RTSP session identifier */
		CURLOPT_RTSP_SESSION_ID = 190,

		/* The RTSP stream URI */
		CURLOPT_RTSP_STREAM_URI = 191,

		/* The Transport: header to use in RTSP requests */
		CURLOPT_RTSP_TRANSPORT = 192,

		/* Manually initialize the client RTSP CSeq for this handle */
		CURLOPT_RTSP_CLIENT_CSEQ = 193,

		/* Manually initialize the server RTSP CSeq for this handle */
		CURLOPT_RTSP_SERVER_CSEQ = 194,

		/* The stream to pass to INTERLEAVEFUNCTION. */
		CURLOPT_INTERLEAVEDATA = 195,

		/* Let the application define a custom write method for RTP data */
		CURLOPT_INTERLEAVEFUNCTION = 196,

		/* Turn on wildcard matching */
		CURLOPT_WILDCARDMATCH = 197,

		/* Directory matching callback called before downloading of an
		individual file (chunk) started */
		CURLOPT_CHUNK_BGN_FUNCTION = 198,

		/* Directory matching callback called after the file (chunk)
		was downloaded,  or skipped */
		CURLOPT_CHUNK_END_FUNCTION = 199,

		/* Change match (fnmatch-like) callback for wildcard matching */
		CURLOPT_FNMATCH_FUNCTION = 200,

		/* Let the application define custom chunk data pointer */
		CURLOPT_CHUNK_DATA = 201,

		/* FNMATCH_FUNCTION user pointer */
		CURLOPT_FNMATCH_DATA = 202,

		/* send linked-list of name:port:address sets */
		CURLOPT_RESOLVE = 203,

		/* Set a username for authenticated TLS */
		CURLOPT_TLSAUTH_USERNAME = 204,

		/* Set a password for authenticated TLS */
		CURLOPT_TLSAUTH_PASSWORD = 205,

		/* Set authentication type for authenticated TLS */
		CURLOPT_TLSAUTH_TYPE = 206,

		/* Set to 1 to enable the "TE:" header in HTTP requests to ask for
		compressed transfer-encoded responses. Set to 0 to disable the use of TE:
		in outgoing requests. The current default is 0,  but it might change in a
		future libcurl release.

		libcurl will ask for the compressed methods it knows of,  and if that
		isn't any,  it will not ask for transfer-encoding at all even if this
		option is set to 1.

		*/
		CURLOPT_TRANSFER_ENCODING = 207,

		/* Callback function for closing socket (instead of close(2)). The callback
		should have type curl_closesocket_callback */
		CURLOPT_CLOSESOCKETFUNCTION = 208,
		CURLOPT_CLOSESOCKETDATA = 209,

		/* allow GSSAPI credential delegation */
		CURLOPT_GSSAPI_DELEGATION = 210,

		/* Set the name servers to use for DNS resolution */
		CURLOPT_DNS_SERVERS = 211,

		/* Time-out accept operations (currently for FTP only) after this amount
		of milliseconds. */
		CURLOPT_ACCEPTTIMEOUT_MS = 212,

		/* Set TCP keepalive */
		CURLOPT_TCP_KEEPALIVE = 213,

		/* non-universal keepalive knobs (Linux = more) */
		CURLOPT_TCP_KEEPIDLE = 214,
		CURLOPT_TCP_KEEPINTVL = 215,

		/* Enable/disable specific SSL features with a bitmask,  see CURLSSLOPT_* */
		CURLOPT_SSL_OPTIONS = 216,

		/* Set the SMTP auth originator */
		CURLOPT_MAIL_AUTH = 217,

		/* Enable/disable SASL initial response */
		CURLOPT_SASL_IR = 218,

		/* Function that will be called instead of the internal progress display
		* function. This function should be defined as the curl_xferinfo_callback
		* prototype defines. (Deprecates CURLOPT_PROGRESSFUNCTION) */
		CURLOPT_XFERINFOFUNCTION = 219,

		/* The XOAUTH2 bearer token */
		CURLOPT_XOAUTH2_BEARER = 220,

		/* Set the interface string to use as outgoing network
		* interface for DNS requests.
		* Only supported by the c-ares DNS backend */
		CURLOPT_DNS_INTERFACE = 221,

		/* Set the local IPv4 address to use for outgoing DNS requests.
		* Only supported by the c-ares DNS backend */
		CURLOPT_DNS_LOCAL_IP4 = 222,

		/* Set the local IPv6 address to use for outgoing DNS requests.
		* Only supported by the c-ares DNS backend */
		CURLOPT_DNS_LOCAL_IP6 = 223,

		/* Set authentication options directly */
		CURLOPT_LOGIN_OPTIONS = 224,

		/* Enable/disable TLS NPN extension (http2 over ssl might fail without) */
		CURLOPT_SSL_ENABLE_NPN = 225,

		/* Enable/disable TLS ALPN extension (http2 over ssl might fail without) */
		CURLOPT_SSL_ENABLE_ALPN = 226,

		/* Time to wait for a response to a HTTP request containing an
		* Expect: 100-continue header before sending the data anyway. */
		CURLOPT_EXPECT_100_TIMEOUT_MS = 227,

		/* This points to a linked list of headers used for proxy requests only,
		struct curl_slist kind */
		CURLOPT_PROXYHEADER = 228,

		/* Pass in a bitmask of "header options" */
		CURLOPT_HEADEROPT = 229,

		/* The public key in DER form used to validate the peer public key
		this option is used only if SSL_VERIFYPEER is true */
		CURLOPT_PINNEDPUBLICKEY = 230,

		/* Path to Unix domain socket */
		CURLOPT_UNIX_SOCKET_PATH = 231,

		/* Set if we should verify the certificate status. */
		CURLOPT_SSL_VERIFYSTATUS = 232,

		/* Set if we should enable TLS false start. */
		CURLOPT_SSL_FALSESTART = 233,

		/* Do not squash dot-dot sequences */
		CURLOPT_PATH_AS_IS = 234,

		/* Proxy Service Name */
		CURLOPT_PROXY_SERVICE_NAME = 235,

		/* Service Name */
		CURLOPT_SERVICE_NAME = 236,

		/* Wait/don't wait for pipe/mutex to clarify */
		CURLOPT_PIPEWAIT = 237,

		/* Set the protocol used when curl is given a URL without a protocol */
		CURLOPT_DEFAULT_PROTOCOL = 238,

		/* Set stream weight,  1 - 256 (default is 16) */
		CURLOPT_STREAM_WEIGHT = 239,

		/* Set stream dependency on another CURL handle */
		CURLOPT_STREAM_DEPENDS = 240,

		/* Set E-xclusive stream dependency on another CURL handle */
		CURLOPT_STREAM_DEPENDS_E = 241,

		/* Do not send any tftp option requests to the server */
		CURLOPT_TFTP_NO_OPTIONS = 242,

		/* Linked-list of host:port:connect-to-host:connect-to-port,
		overrides the URL's host:port (only for the network layer) */
		CURLOPT_CONNECT_TO = 243,

		/* Set TCP Fast Open */
		CURLOPT_TCP_FASTOPEN = 244,

		/* Continue to send data if the server responds early with an
		* HTTP status code >= 300 */
		CURLOPT_KEEP_SENDING_ON_ERROR = 245,

		/* The CApath or CAfile used to validate the proxy certificate
		this option is used only if PROXY_SSL_VERIFYPEER is true */
		CURLOPT_PROXY_CAINFO = 246,

		/* The CApath directory used to validate the proxy certificate
		this option is used only if PROXY_SSL_VERIFYPEER is true */
		CURLOPT_PROXY_CAPATH = 247,

		/* Set if we should verify the proxy in ssl handshake,
		set 1 to verify. */
		CURLOPT_PROXY_SSL_VERIFYPEER = 248,

		/* Set if we should verify the Common name from the proxy certificate in ssl
		* handshake = 2 to ensure that it matches
		* the provided hostname. */
		CURLOPT_PROXY_SSL_VERIFYHOST = 249,

		/* What version to specifically try to use for proxy.
		See CURL_SSLVERSION defines below. */
		CURLOPT_PROXY_SSLVERSION = 250,

		/* Set a username for authenticated TLS for proxy */
		CURLOPT_PROXY_TLSAUTH_USERNAME = 251,

		/* Set a password for authenticated TLS for proxy */
		CURLOPT_PROXY_TLSAUTH_PASSWORD = 252,

		/* Set authentication type for authenticated TLS for proxy */
		CURLOPT_PROXY_TLSAUTH_TYPE = 253,

		/* name of the file keeping your private SSL-certificate for proxy */
		CURLOPT_PROXY_SSLCERT = 254,

		/* type of the file keeping your SSL-certificate ("DER" = "ENG") for
		proxy */
		CURLOPT_PROXY_SSLCERTTYPE = 255,

		/* name of the file keeping your private SSL-key for proxy */
		CURLOPT_PROXY_SSLKEY = 256,

		/* type of the file keeping your private SSL-key ("DER" = "ENG") for
		proxy */
		CURLOPT_PROXY_SSLKEYTYPE = 257,

		/* password for the SSL private key for proxy */
		CURLOPT_PROXY_KEYPASSWD = 258,

		/* Specify which SSL ciphers to use for proxy */
		CURLOPT_PROXY_SSL_CIPHER_LIST = 259,

		/* CRL file for proxy */
		CURLOPT_PROXY_CRLFILE = 260,

		/* Enable/disable specific SSL features with a bitmask for proxy,  see
		CURLSSLOPT_* */
		CURLOPT_PROXY_SSL_OPTIONS = 261,

		/* Name of pre proxy to use. */
		CURLOPT_PRE_PROXY = 262,

		/* The public key in DER form used to validate the proxy public key
		this option is used only if PROXY_SSL_VERIFYPEER is true */
		CURLOPT_PROXY_PINNEDPUBLICKEY = 263,

		/* Path to an abstract Unix domain socket */
		CURLOPT_ABSTRACT_UNIX_SOCKET = 264,

		/* Suppress proxy CONNECT response headers from user callbacks */
		CURLOPT_SUPPRESS_CONNECT_HEADERS = 265,

		/* The request target,  instead of extracted from the URL */
		CURLOPT_REQUEST_TARGET = 266,

		/* bitmask of allowed auth methods for connections to SOCKS5 proxies */
		CURLOPT_SOCKS5_AUTH = 267,

		/* Enable/disable SSH compression */
		CURLOPT_SSH_COMPRESSION = 268,

		/* Post MIME data. */
		CURLOPT_MIMEPOST = 269,

		/* Time to use with the CURLOPT_TIMECONDITION. Specified in number of
		seconds since 1 Jan 1970. */
		CURLOPT_TIMEVALUE_LARGE = 270,

		/* Head start in milliseconds to give happy eyeballs. */
		CURLOPT_HAPPY_EYEBALLS_TIMEOUT_MS = 271,

		/* Function that will be called before a resolver request is made */
		CURLOPT_RESOLVER_START_FUNCTION = 272,

		/* User data to pass to the resolver start callback. */
		CURLOPT_RESOLVER_START_DATA = 273,

		/* send HAProxy PROXY protocol header? */
		CURLOPT_HAPROXYPROTOCOL = 274,

		/* shuffle addresses before use when DNS returns multiple */
		CURLOPT_DNS_SHUFFLE_ADDRESSES = 275,

		/* Specify which TLS 1.3 ciphers suites to use */
		CURLOPT_TLS13_CIPHERS = 276,
		CURLOPT_PROXY_TLS13_CIPHERS = 277,

		/* Disallow specifying username/login in URL. */
		CURLOPT_DISALLOW_USERNAME_IN_URL = 278,

		/* DNS-over-HTTPS URL */
		CURLOPT_DOH_URL = 279,

		/* Preferred buffer size to use for uploads */
		CURLOPT_UPLOAD_BUFFERSIZE = 280,

		/* Time in ms between connection upkeep calls for long-lived connections. */
		CURLOPT_UPKEEP_INTERVAL_MS = 281,

		/* Specify URL using CURL URL API. */
		CURLOPT_CURLU = 282,

		/* add trailing data just after no more data is available */
		CURLOPT_TRAILERFUNCTION = 283,

		/* pointer to be passed to HTTP_TRAILER_FUNCTION */
		CURLOPT_TRAILERDATA = 284,

		/* set this to 1L to allow HTTP/0.9 responses or 0L to disallow */
		CURLOPT_HTTP09_ALLOWED = 285,

		/* alt-svc control bitmask */
		CURLOPT_ALTSVC_CTRL = 286,

		/* alt-svc cache file name to possibly read from/write to */
		CURLOPT_ALTSVC = 287,

		/* maximum age of a connection to consider it for reuse (in seconds) */
		CURLOPT_MAXAGE_CONN = 288,

		/* SASL authorisation identity */
		CURLOPT_SASL_AUTHZID = 289,

		CURLOPT_LASTENTRY /* the last unused */
	};

public:
	cweeCURL();
	~cweeCURL();

	/* Set a cURL option to LONG type */
	int setopt(cweeCURL::CURLoption op, long val);
	/* Set a cURL request */
	int setopt_CUSTOMREQUEST(const char* val);
	/* Set a cURL option to STRING type */
	int setopt(cweeCURL::CURLoption op, const char* val);
	/* Set a cURL option to LIST OF STRINGS type */
	int setopt(cweeCURL::CURLoption op, cweeList<cweeStr> const& val);
	/* Set a cURL option to FUNCTION type */
	int setopt(cweeCURL::CURLoption op, size_t(*func)(void*, size_t, size_t, FILE*));
	/* Set a cURL option to FILE type */
	int setopt(cweeCURL::CURLoption op, FILE* fp);
	/* Execute the optioned request */
	int perform();

private:
	cweeSharedPtr<void> curl; // CURL*

};

/* Support for server-client queries across computers */
#ifdef includeZeroMQ
class cweeRouterRequest {
public:
	cweeRouterRequest() : mut(new cweeSysMutex()) {};

	bool TryLock() {
		return mut->Lock(false);
	};
	void Lock() {
		mut->Lock();
	};
	void Unlock() {
		mut->Unlock();
	};
	NODISCARD AUTO Guard() {
		return mut->Guard();
	};

	cweeStr 		routerPort;
	cweeStr 		request;
	cweeStr			reply;
	cweeSharedPtr<void>	requester_id; // zmq::message_t
	int				request_index;

	bool			started = false;
	bool			completed = false;
	cweeUnorderedList< cweeSharedPtr<cweeRouterRequest> >* routerRequests;
	cweeSharedPtr<cweeSysMutex> mut;
};

/* Manager for publisher/subscriber and client/router peer-to-peer communications. */
class ZeroMQ {
public:
	ZeroMQ() {};
	virtual ~ZeroMQ() {};
public:
	/* Publishing servers "push" data (like weather) to many readers simultaneously */
	virtual bool		StartPublishingServer(int Port) = 0;
	virtual bool		ClosePublishingServer(int Port) = 0;
	virtual cweeList<int>	GetPublishingServerPorts() = 0;
	virtual bool		UpdatePublishingServer(const cweeStr& message, int Port) = 0;
public:
	/* Subscribers "wait" for data from a publisher. */
	virtual bool		StartSubscriptionClient(const cweeStr& address, int Port) = 0;
	virtual bool		CloseSubscriptionClient(const cweeStr& address, int Port) = 0;
	virtual cweeList<cweeUnion<cweeStr, int>> GetSubscriptionClients() = 0;
	virtual bool		TryGetSubscriptionUpdate(cweeStr& out, const cweeStr& address, int Port) = 0;

public:
	/* Get the list of ports that are being used as servers */
	virtual cweeList<int>	GetRouterServerPorts() = 0;
	/* Routers are client-to-server request managers for complex question-answer format discussions between computers. */
	virtual bool		StartRouterServer(int Port) = 0;
	virtual bool		CloseRouterServer(int Port) = 0;
	/* Get a recent request from this port. Note: The request is erased from the list after successful query, and the return copy is the only copy remaining. If lost, the request will spin forever at the client. */
	virtual cweeSharedPtr<cweeRouterRequest>	TryGetRouterServerRequest(int Port) = 0;
	/* Send a reply to the client who made the request with the answer to their question/request. */
	virtual bool		ReplyToRequestClient(cweeSharedPtr<cweeRouterRequest> data) = 0;
public:
	/* Routers are client-to-server request managers for complex question-answer format discussions between computers. */
	virtual bool		StartRequestClient(const cweeStr& address, int Port) = 0;
	virtual bool		CloseRequestClient(const cweeStr& address, int Port) = 0;
	virtual cweeList<cweeUnion<cweeStr, int>> GetRequestClients() = 0;
	virtual bool		TrySendRequest(const cweeStr& request, const cweeStr& address, int Port) = 0;
	virtual bool		TryGetReply(cweeStr& out, const cweeStr& address, int Port) = 0;
	virtual cweeStr		GetReply(const cweeStr& request, const cweeStr& address, int Port) = 0;
};

/* Manager for publisher/subscriber and client/router peer-to-peer communications. */
extern cweeSharedPtr<ZeroMQ> Servers;
#endif