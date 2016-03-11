/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_HTTPCLIENT_H_
#define _SMING_CORE_NETWORK_HTTPCLIENT_H_

#include "TcpClient.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Services/DateTime/DateTime.h"
#include "../Delegate.h"
#include "../../Services/WebHelpers/http_parser.h"

#define MAX_HTTP_HEADERS_SIZE  16*1024

class HttpClient;
class URL;

//typedef void (*HttpClientCompletedCallback)(HttpClient& client, bool successful);
typedef Delegate<void(HttpClient& client, bool successful)> HttpClientCompletedDelegate;

/* The below 2 delegates have to return 0 on success or negative value on failure */
typedef Delegate<int(HttpClient& client, const char *at, size_t length)> ResponseBodyDelegate;
typedef Delegate<int(HttpClient& client)> HeadersCompleteDelegate;

enum HttpClientMode
{
	eHCM_String = 0,
	eHCM_File,
	eHCM_UserDefined
};

class HttpClient: protected TcpClient
{
public:
	HttpClient(bool autoDestruct = false);
	HttpClient(ResponseBodyDelegate responseBodyDelegate, bool autoDestruct = false);
	HttpClient(HeadersCompleteDelegate headersCompleteDelegate,
			   ResponseBodyDelegate responseBodyDelegate = NULL, bool autoDestruct = false);
	virtual ~HttpClient();

	// Text mode
	bool downloadString(String url, HttpClientCompletedDelegate onCompleted);
	String getResponseString(); // Can be used only after calling downloadString!

	// File mode
	bool downloadFile(String url, HttpClientCompletedDelegate onCompleted = NULL);
	bool downloadFile(String url, String saveFileName, HttpClientCompletedDelegate onCompleted = NULL);

	void setPostBody(const String& _method);
	String getPostBody();

	void setRequestHeader(const String name, const String value);
	bool hasRequestHeader(const String name);
	void setRequestContentType(String _content_type);

	// Resulting HTTP status code
	__forceinline int getResponseCode() { return code; }
	__forceinline bool isSuccessful() { return (!writeError) && (code >= 200 && code <= 399); }

	__forceinline bool isProcessing()  { return TcpClient::isProcessing(); }
	__forceinline TcpClientState getConnectionState() { return TcpClient::getConnectionState(); }

	String getResponseHeader(String headerName, String defaultValue = "");
	HashMap<String, String> &getResponseHeaders();
	DateTime getLastModifiedDate(); // Last-Modified header
	DateTime getServerDate(); // Date header

	void reset(); // Reset current status, data and etc.

protected:
	bool startDownload(URL uri, HttpClientMode mode, HttpClientCompletedDelegate onCompleted);
	void onFinished(TcpClientState finishState);
	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf *buf);

	virtual err_t onResponseBody(const char *at, size_t length);

	/**
	 * Method that handles protocol upgrade. Implement this metheod in child classes.
	 * For example in WebSockets client class.
	 *
	 * @param http_parser* parser
	 * - the parser->data contains pointer to the current object that called the method
	 *
	 * @return err_t
	 */
	virtual err_t onProtocolUpgrade(http_parser* parser);

protected:
	bool waitParse = false;
	bool writeError = false;


	static int staticOnMessageComplete(http_parser* parser);
	static int staticOnHeadersComplete(http_parser* parser);

	static int staticOnHeaderField(http_parser *parser, const char *at, size_t length);
	static int staticOnHeaderValue(http_parser *parser, const char *at, size_t length);
	static int staticOnBody(http_parser *parser, const char *at, size_t length);

private:
	int code;
	HttpClientCompletedDelegate onCompleted;
	HttpClientMode mode;
	HashMap<String, String> requestHeaders;
	HashMap<String, String> responseHeaders;

	String responseStringData;
	String body = "";
	file_t saveFile;

	http_parser_settings parserSettings;
	http_parser *parser = NULL;
	bool lastWasValue = true;
	String lastData = "";
	String currentField  = "";

	ResponseBodyDelegate responseBodyDelegate;
	HeadersCompleteDelegate headersCompleteDelegate;
	int totalHeadersSize = 0;
};

#endif /* _SMING_CORE_NETWORK_HTTPCLIENT_H_ */
