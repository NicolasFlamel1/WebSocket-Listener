// Check if Windows
#ifdef _WIN32

	// Set system version
	#define _WIN32_WINNT _WIN32_WINNT_VISTA
#endif

// Header files
#include <atomic>
#include <climits>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <random>
#include <signal.h>
#include <thread>
#include <unordered_set>
#include "event2/buffer.h"
#include "common.h"
#include "event2/bufferevent.h"
#include "event2/event.h"
#include "event2/event_struct.h"
#include "event2/http.h"
#include "event2/thread.h"
#include "json.h"

// Extern C
extern "C" {

	// Header files
	#include "feature/api/tor_api.h"
	#include "feature/api/tor_api_internal.h"
}

// Check if Windows
#ifdef _WIN32

	// Header files
	#include <ws2tcpip.h>

// Otherwise
#else

	// Header files
	#include <arpa/inet.h>
#endif

using namespace std;


// Definitions

// Check if Windows or macOS
#if defined _WIN32 || defined __APPLE__

	// Quick exit
	#define quick_exit _exit
#endif


// Constants

// WebSocket magic key value
static const char *WEBSOCKET_MAGIC_KEY_VALUE = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// HTTP port
static const uint16_t HTTP_PORT = 80;

// HTTP switching protocols
static const int HTTP_SWITCHING_PROTOCOL = 101;

// Check Tor connected interval microseconds
static const decltype(timeval::tv_usec) CHECK_TOR_CONNECTED_INTERVAL_MICROSECONDS = 100 * 1000;

// WebSocket opcode byte offset
static const size_t WEBSOCKET_OPCODE_BYTE_OFFSET = 0;

// WebSocket opcode byte mask
static const uint8_t WEBSOCKET_OPCODE_BYTE_MASK = 0x0F;

// WebSocket final frame byte offset
static const size_t WEBSOCKET_FINAL_FRAME_BYTE_OFFSET = WEBSOCKET_OPCODE_BYTE_OFFSET;

// WebSocket final frame byte mask
static const uint8_t WEBSOCKET_FINAL_FRAME_BYTE_MASK = 0x80;

// WebSocket extension byte offset
static const size_t WEBSOCKET_EXTENSION_BYTE_OFFSET = 0;

// WebSocket extension byte mask
static const uint8_t WEBSOCKET_EXTENSION_BYTE_MASK = 0x70;

// WebSocket mask byte offset
static const size_t WEBSOCKET_MASK_BYTE_OFFSET = 1;

// WebSocket mask byte mask
static const uint8_t WEBSOCKET_MASK_BYTE_MASK = 0x80;

// WebSocket length byte offset
static const size_t WEBSOCKET_LENGTH_BYTE_OFFSET = 1;

// WebSocket length byte mask
static const uint8_t WEBSOCKET_LENGTH_BYTE_MASK = 0x7F;

// WebSocket sixteen bits length
static const uint8_t WEBSOCKET_SIXTEEN_BITS_LENGTH = 0x7E;

// WebSocket sixty-three bits length
static const uint8_t WEBSOCKET_SIXTY_THREE_BITS_LENGTH = 0x7F;

// WebSocket mask length
static const size_t WEBSOCKET_MASK_LENGTH = 4;

// Maximum safe integer
static const uint64_t MAXIMUM_SAFE_INTEGER = pow(2, 53) - 1;

// Cookie separator
static const char COOKIE_SEPARATOR = ';';

// Cookie key value separator
static const char COOKIE_KEY_VALUE_SEPARATOR = '=';

// Session ID cookie name
static const char *SESSION_ID_COOKIE_NAME = "Listener_ID";

// Session ID minimum length
static const size_t SESSION_ID_MINIMUM_LENGTH = 40;

// Session ID maximum length
static const size_t SESSION_ID_MAXIMUM_LENGTH = 60;

// Session ID characters
static const char SESSION_ID_CHARACTERS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

// Session ID cookie maximum age seconds
static const int SESSION_ID_COOKIE_MAXIMUM_AGE_SECONDS = 4 * Common::WEEKS_IN_A_YEAR * Common::DAYS_IN_A_WEEK * Common::HOURS_IN_A_DAY * Common::MINUTES_IN_AN_HOUR * Common::SECONDS_IN_A_MINUTE;

// Ping interval seconds
static const decltype(timeval::tv_sec) PING_INTERVAL_SECONDS = 10;

// URL doesn't exist
static const vector<uint8_t> URL_DOESNT_EXIST = {};

// URL minimum length
static const size_t URL_MINIMUM_LENGTH = 4;

// URL maximum length
static const size_t URL_MAXIMUM_LENGTH = 24;

// URL characters
static const char URL_CHARACTERS[] = "abcdefghjkmnpqrstuvwxyz23456789";

// Temporary directory length
static const size_t TEMPORARY_DIRECTORY_LENGTH = 8;

// Maximum headers size
static const size_t MAXIMUM_HEADERS_SIZE = 1 * Common::KILOBYTE_IN_A_MEGABYTE * Common::BYTES_IN_A_KILOBYTE;

// Maximum body size
static const size_t MAXIMUM_BODY_SIZE = 10 * Common::KILOBYTE_IN_A_MEGABYTE * Common::BYTES_IN_A_KILOBYTE;

// WebSocket Opcode
enum class WebSocketOpcode {

	// Continuation
	CONTINUATION = 0x00,
	
	// Text
	TEXT = 0x01,
	
	// Ping
	PING = 0x09,
	
	// Pong
	PONG = 0x0A
};


// Classes

// Client class
class Client final {

	// Public
	public:
	
		// Constructor
		Client(const string &sessionId) :
		
			// Set session ID
			sessionId(sessionId),
		
			// Set interaction index
			interactionIndex(0)
		{
		}
		
		// Get session ID
		const string &getSessionId() const {
		
			// Return session ID
			return sessionId;
		}
		
		// Get next interaction index
		Json::Number getNextInteractionIndex() {
		
			// Save initial interaction index
			const Json::Number initialInteractionIndex = interactionIndex;
		
			// Loop until an unused interaction index is found
			do {
			
				// Increment interaction index
				++interactionIndex;
				
				// Check if the interaction index is past the maximum safe integer
				if(interactionIndex > MAXIMUM_SAFE_INTEGER) {
				
					// Reset interaction index
					interactionIndex = 0;
				}
				
				// Check if all interaction indices have been tried
				if(interactionIndex == initialInteractionIndex) {
				
					// Throw exception
					throw runtime_error("Finding unique interaction index failed");
				}
			
			} while(interactions.count(interactionIndex));
			
			// Return interaction index
			return interactionIndex;
		}
		
		// Add interaction
		bool addInteraction(Json::Number interactionIndex, evhttp_request *request) {
		
			// Check if interaction index is already being used
			if(interactions.count(interactionIndex)) {
			
				// Return false
				return false;
			}
			
			// Add interaction to list
			interactions.emplace(interactionIndex, request);
			
			// Return true
			return true;
		}
		
		// Remove interaction
		void removeInteraction(Json::Number interactionIndex) {
		
			// Check if interaction exists
			if(interactions.count(interactionIndex)) {
		
				// Remove interaction from list
				interactions.erase(interactionIndex);
			}
		}
		
		// Cancel all interactions
		void cancelAllInteractions() {
		
			// Go through all interactions
			for(unordered_map<Json::Number, evhttp_request *>::const_iterator i = interactions.cbegin(); i != interactions.cend(); ++i) {
			
				// Get interaction's request
				evhttp_request *request = i->second;
				
				// Remove request's buffer callbacks
				bufferevent_setcb(evhttp_connection_get_bufferevent(evhttp_request_get_connection(request)), nullptr, nullptr, nullptr, nullptr);
				
				// Reply with not found error to request
				evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
			}
			
			// Clear interactions
			interactions.clear();
		}
		
		// Get interaction
		evhttp_request *getInteraction(Json::Number interactionIndex) const {
		
			// Check if interaction exists
			if(interactions.count(interactionIndex)) {
			
				// Return interaction's request
				return interactions.at(interactionIndex);
			}
			
			// Return null
			return nullptr;
		}
	
	// Private
	private:
	
		// Session ID
		string sessionId;
		
		// Interaction index
		Json::Number interactionIndex;
		
		// Interactions
		unordered_map<Json::Number, evhttp_request *> interactions;
};

// Check if Windows
#ifdef _WIN32

	// Windows socket class
	class WindowsSocket final {

		// Public
		public:
		
			// Constructor
			WindowsSocket() {
			
				// Check if initiating Windows socket failed
				WSADATA wsaData;
				if(WSAStartup(MAKEWORD(WindowsSocket::MAJOR_VERSION, WindowsSocket::MINOR_VERSION), &wsaData)) {
				
					// Throw exception
					throw runtime_error("Initiating Windows socket failed");
				}
			}
			
			// Destructor
			~WindowsSocket() {
			
				// Clean up Windows socket
				WSACleanup();
			}
		
		// Private
		private:
		
			// Windows socket major version
			static const BYTE MAJOR_VERSION = 2;
			
			// Windows socket minor version
			static const BYTE MINOR_VERSION = 2;
	};
#endif


// Function prototypes

// Create WebSocket response
static const vector<uint8_t> createWebSocketResponse(const string &message, WebSocketOpcode opcode);

// Get cookies
static const unordered_map<string, string> getCookies(const string &cookieHttpHeader);

// Get random session ID
static const string getRandomSessionId();

// Get random URL
static const string getRandomUrl(const string &torHiddenServiceAddress);


// Main function
int main(int argc, char *argv[]) {

	// Display message
	cout << TOSTRING(PROGRAM_NAME) << " V" << TOSTRING(PROGRAM_VERSION) << endl;
	
	// Initialize listen address
	string listenAddress = "localhost";
	
	// Initialize listen port
	uint16_t listenPort = 9061;
	
	// Set options
	const option options[] = {
	
		// Version
		{"version", no_argument, nullptr, 'v'},
		
		// Address
		{"address", required_argument, nullptr, 'a'},
		
		// Port
		{"port", required_argument, nullptr, 'p'},
		
		// Help
		{"help", no_argument, nullptr, 'h'},
		
		// End
		{}
	};
	
	// Go through all options
	for(int option = getopt_long(argc, argv, "va:p:h", options, nullptr); option != -1; option = getopt_long(argc, argv, "va:p:h", options, nullptr)) {
	
		// Check option
		switch(option) {
		
			// Version
			case 'v':
			
				// Return success
				return EXIT_SUCCESS;
		
			// Address
			case 'a':
			
				// Set listen address
				listenAddress = optarg;
				
				// Break
				break;
			
			// Port
			case 'p':
			
				{
					// Get port
					string port = optarg;
					
					// Check if port is numeric
					if(Common::isNumeric(port)) {
					
						// Initialize error occured
						bool errorOccured = false;
					
						// Try
						int portNumber;
						try {
						
							// Get port number from port
							portNumber = stoi(port);
						}
						
						// Catch errors
						catch(...) {
						
							// Set error occured
							errorOccured = true;
						}
						
						// Check if an error didn't occurt
						if(!errorOccured) {
						
							// Check if port number is valid
							if(portNumber >= 1 && portNumber <= UINT16_MAX) {
							
								// Set listen port
								listenPort = portNumber;
						
								// Break
								break;
							}
						}
					}
					
					// Display message
					cout << "Invalid port: " << port << endl;
				}
			
			// Help or default
			case 'h':
			default:
			
				// Display message
				cout << endl << "Usage:" << endl << "\t\"" << argv[0] << "\" [options]" << endl << endl;
				cout << "Options:" << endl;
				cout << "\t-v, --version\t\tDisplays version information" << endl;
				cout << "\t-a, --address\t\tSets address to listen on" << endl;
				cout << "\t-p, --port\t\tSets port to listen on" << endl;
				cout << "\t-h, --help\t\tDisplays help information" << endl;
			
				// Return failure
				return EXIT_FAILURE;
		}
	}

	// Check if not Windows
	#ifndef _WIN32

		// Check if blocking all signals failed
		sigset_t signalMask;
		if(sigfillset(&signalMask) || pthread_sigmask(SIG_BLOCK, &signalMask, nullptr)) {
		
			// Display message
			cout << "Blocking all signals failed" << endl;
		
			// Return failure
			return EXIT_FAILURE;
		}
	#endif
	
	// Check if Windows
	#ifdef _WIN32
	
		// Check if enabling thread support failed
		if(evthread_use_windows_threads()) {
		
			// Display message
			cout << "Enabling thread support failed" << endl;
		
			// Return failure
			return EXIT_FAILURE;
		}
	
	// Otherwise
	#else

		// Check if enabling thread support failed
		if(evthread_use_pthreads()) {
		
			// Display message
			cout << "Enabling thread support failed" << endl;
		
			// Return failure
			return EXIT_FAILURE;
		}
	#endif
	
	// Check if Windows
	#ifdef _WIN32
	
		// Initialize Windows socket
		unique_ptr<WindowsSocket> windowsSocket;
		
		// Try
		try {
		
			// Set Windows socket
			windowsSocket = make_unique<WindowsSocket>();
		}
		
		// Catch errors
		catch(const runtime_error &error) {
		
			// Display message
			cout << error.what() << endl;
		
			// Return failure
			return EXIT_FAILURE;
		}
	#endif
	
	// Check if creating event base failed
	shared_ptr<event_base> eventBase(event_base_new(), event_base_free);
	if(!eventBase) {
	
		// Display message
		cout << "Creating event base failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Check if creating HTTP server failed
	unique_ptr<evhttp, decltype(&evhttp_free)> httpServer(evhttp_new(eventBase.get()), evhttp_free);
	if(!httpServer) {
	
		// Display message
		cout << "Creating HTTP server failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Set HTTP server to only allow GET and OPTIONS requests
	evhttp_set_allowed_methods(httpServer.get(), EVHTTP_REQ_GET | EVHTTP_REQ_OPTIONS);
	
	// Initialize Tor hidden service address
	string torHiddenServiceAddress;
	
	// Initialize clients
	unordered_map<evhttp_connection *, Client> clients;
	
	// Initialize ping event
	event pingEvent;
		
	// Check if setting ping callback failed
	if(event_assign(&pingEvent, eventBase.get(), -1, EV_PERSIST, ([](evutil_socket_t signal, short events, void *argument) {
	
		// Get clients from argument
		unordered_map<evhttp_connection *, Client> *clients = reinterpret_cast<unordered_map<evhttp_connection *, Client> *>(argument);
		
		// Go through all clients
		for(unordered_map<evhttp_connection *, Client>::const_iterator i = clients->cbegin(); i != clients->cend();) {
		
			// Get client's connection
			evhttp_connection *connection = i->first;
			
			// Increment client index
			++i;
			
			// Check if getting connection's buffer failed
			bufferevent *connectionsBuffer = evhttp_connection_get_bufferevent(connection);
			if(!connectionsBuffer) {
			
				// Close connection
				evhttp_connection_free(connection);
				
				// Cancel all client's interactions
				clients->at(connection).cancelAllInteractions();
				
				// Remove connection from list of clients
				clients->erase(connection);
			}
			
			// Otherwise
			else {
		
				// Get ping message
				const vector<uint8_t> pingMessage = createWebSocketResponse("", WebSocketOpcode::PING);
				
				// Check if sending ping message to client failed
				if(bufferevent_write(connectionsBuffer, pingMessage.data(), pingMessage.size())) {
				
					// Check if getting connection's buffer input was successful
					evbuffer *input = bufferevent_get_input(connectionsBuffer);
					if(input) {
					
						// Remove data from input
						evbuffer_drain(input, evbuffer_get_length(input));
					}
					
					// Remove connection's buffer callbacks
					bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
					
					// Close connection
					evhttp_connection_free(connection);
					
					// Cancel all client's interactions
					clients->at(connection).cancelAllInteractions();
					
					// Remove connection from list of clients
					clients->erase(connection);
				}
			}
		}
	
	}), &clients)) {
	
		// Display message
		cout << "Setting ping callback failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Set ping timer
	const timeval pingTimer = {
	
		// Seconds
		.tv_sec = PING_INTERVAL_SECONDS
	};
	
	// Check if adding ping event to the dispatched events failed
	if(evtimer_add(&pingEvent, &pingTimer)) {
	
		// Display message
		cout << "Adding ping event to the dispatched events failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Initialize URLs
	unordered_map<string, unordered_set<string>> urls;
	
	// Initialize HTTP server request callback argument
	tuple<const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *> httpServerRequestCallbackArgument(&torHiddenServiceAddress, &clients, &urls);
	
	// Set HTTP server WebSocket request callback
	evhttp_set_cb(httpServer.get(), "/", ([](evhttp_request *request, void *argument) {
	
		// Get HTTP server request callback argument from argument
		tuple<const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *> *httpServerRequestCallbackArgument = reinterpret_cast<tuple<const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *> *>(argument);
		
		// Get Tor hidden service address from HTTP server request callback argument
		const string *torHiddenServiceAddress = get<0>(*httpServerRequestCallbackArgument);
		
		// Get clients from HTTP server request callback argument
		unordered_map<evhttp_connection *, Client> *clients = get<1>(*httpServerRequestCallbackArgument);
		
		// Get URLs from HTTP server request callback argument
		unordered_map<string, unordered_set<string>> *urls = get<2>(*httpServerRequestCallbackArgument);
		
		// Check if setting request's CORS headers failed
		if(evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Origin", "*") || evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Headers", "*")) {
		
			// Reply with internal server error to request
			evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
		}
		
		// Otherwise check if request doesn't have a URI
		else if(!evhttp_request_get_uri(request) || !strlen(evhttp_request_get_uri(request))) {
		
			// Reply with bad request error to request
			evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
		}
		
		// Otherwise check if OPTIONS request
		else if(evhttp_request_get_command(request) == EVHTTP_REQ_OPTIONS) {
		
			// Check if setting CORS header failed
			if(evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Methods", "GET, OPTIONS")) {
			
				// Reply with internal server error to request
				evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
			}
			
			// Otherwise
			else {
			
				// Reply with ok to request
				evhttp_send_reply(request, HTTP_OK, nullptr, nullptr);
			}
		}
		
		// Otherwise
		else {
		
			// Initialize HTTP headers
			unordered_map<string, string> httpHeaders;
			
			// Check if request has headers
			evkeyvalq *headers = evhttp_request_get_input_headers(request);
			if(headers) {
			
				// Go through all of the request's headers
				for(evkeyval *header = headers->tqh_first; header; header = header->next.tqe_next) {
				
					// Append header to list
					httpHeaders[Common::toLowerCase(header->key)] = header->value;
				}
			}
			
			// Check if all required HTTP headers exists for a WebSocket connection
			if(httpHeaders.count("connection") && Common::toLowerCase(httpHeaders.at("connection")) == "upgrade" && httpHeaders.count("upgrade") && Common::toLowerCase(httpHeaders.at("upgrade")) == "websocket" && httpHeaders.count("sec-websocket-key")) {
			
				// Initialize cookies
				unordered_map<string, string> cookies;
				
				// Check if cookies are provided
				if(httpHeaders.count("cookie")) {
				
					// Try
					try {
					
						// Get cookies from HTTP headers
						cookies = getCookies(httpHeaders.at("cookie"));
					}
					
					// Catch errors
					catch(...) {
					
						// Reply with bad request error to request
						evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
						
						// Return
						return;
					}
				}
				
				// Initialize session ID to the cookie if valid otherwise a random session ID
				string sessionId = (cookies.count(SESSION_ID_COOKIE_NAME) && Common::isAlphanumeric(cookies.at(SESSION_ID_COOKIE_NAME)) && urls->count(cookies.at(SESSION_ID_COOKIE_NAME))) ? cookies.at(SESSION_ID_COOKIE_NAME) : getRandomSessionId();
				
				// Loop until a unique session ID is found
				while(true) {
				
					// Initialize session ID in use
					bool sessionIdInUse = false;
				
					// Go through all clients
					for(unordered_map<evhttp_connection *, Client>::const_iterator i = clients->cbegin(); i != clients->cend(); ++i) {
					
						// Get client
						const Client &client = i->second;
						
						// Check if client is using the session ID
						if(client.getSessionId() == sessionId) {
						
							// Set session Id in use
							sessionIdInUse = true;
						
							// Break
							break;
						}
					}
					
					// Check if session ID isn't in use
					if(!sessionIdInUse) {
					
						// Break
						break;
					}
				
					// Set session ID to a random session ID
					sessionId = getRandomSessionId();
				}
				
				// Set response key to request key appended the WebSocket magic key value to it
				string responseKey = httpHeaders.at("sec-websocket-key") + WEBSOCKET_MAGIC_KEY_VALUE;
				
				// Try
				try {
				
					// Finish creating response key
					responseKey = Json::base64Encode(Common::sha1Hash(vector<uint8_t>(responseKey.begin(), responseKey.end())));
				}
				
				// Catch errors
				catch(...) {
				
					// Reply with internal server error to request
					evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
					
					// Return
					return;
				}
				
				// Check if setting HTTP headers to finalize WebSocket handshake failed
				if(evhttp_add_header(evhttp_request_get_output_headers(request), "Upgrade", "websocket") || evhttp_add_header(evhttp_request_get_output_headers(request), "Connection", "Upgrade") || evhttp_add_header(evhttp_request_get_output_headers(request), "Sec-WebSocket-Accept", responseKey.c_str()) || evhttp_add_header(evhttp_request_get_output_headers(request), "Set-Cookie", (string(SESSION_ID_COOKIE_NAME) + '=' + sessionId + "; Max-Age=" + to_string(SESSION_ID_COOKIE_MAXIMUM_AGE_SECONDS) + "; HttpOnly; Secure; SameSite=None; Priority=High; Path=/").c_str())) {
				
					// Reply with internal server error to request
					evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
				}
				
				// Otherwise
				else {
				
					// Get request's connection
					evhttp_connection *connection = evhttp_request_get_connection(request);
					
					// Check if clients already includes the connection
					if(clients->count(connection)) {
					
						// Reply with internal server error to request
						evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
					}
					
					// Otherwise
					else {
					
						// Check if creating message failed
						unique_ptr<string> message = make_unique<string>();
						if(!message) {
						
							// Reply with internal server error to request
							evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
						}
						
						// Otherwise
						else {
						
							// Check if creating connection's buffer callbacks argument failed
							unique_ptr<tuple<evhttp_connection *, string *, const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *>> connectionsBufferCallbacksArgument = make_unique<tuple<evhttp_connection *, string *, const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *>>(connection, message.get(), torHiddenServiceAddress, clients, urls);
							if(!connectionsBufferCallbacksArgument) {
							
								// Reply with internal server error to request
								evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
							}
							
							// Otherwise
							else {
							
								// Reply with switching protocol response to request
								evhttp_send_reply(request, HTTP_SWITCHING_PROTOCOL, nullptr, nullptr);
								
								// Add connection to list of clients
								clients->emplace(connection, sessionId);
								
								// Check if URLs doesn't exist for the session ID
								if(!urls->count(sessionId)) {
								
									// Add session ID to URLs list
									urls->emplace(sessionId, unordered_set<string>());
								}
								
								// Set connection's buffer callbacks
								bufferevent_setcb(evhttp_connection_get_bufferevent(connection), ([](bufferevent *connectionsBuffer, void *argument) {
								
									// Get connection's buffer callbacks argument from argument
									unique_ptr<tuple<evhttp_connection *, string *, const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *>> connectionsBufferCallbacksArgument(reinterpret_cast<tuple<evhttp_connection *, string *, const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *> *>(argument));
									
									// Get connection from connection's buffer callbacks argument
									evhttp_connection *connection = get<0>(*connectionsBufferCallbacksArgument);
									
									// Get message from connection's buffer callbacks argument
									unique_ptr<string> message(get<1>(*connectionsBufferCallbacksArgument));
									
									// Get Tor hidden service address from connection's buffer callbacks argument
									const string *torHiddenServiceAddress = get<2>(*connectionsBufferCallbacksArgument);
									
									// Get clients from connection's buffer callbacks argument
									unordered_map<evhttp_connection *, Client> *clients = get<3>(*connectionsBufferCallbacksArgument);
									
									// Get URLs from connection's buffer callbacks argument
									unordered_map<string, unordered_set<string>> *urls = get<4>(*connectionsBufferCallbacksArgument);
								
									// Check if getting input from the connection's buffer failed
									evbuffer *input = bufferevent_get_input(connectionsBuffer);
									if(!input) {
									
										// Remove connection's buffer callbacks
										bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
									
										// Check if connection still exists
										if(clients->count(connection)) {
									
											// Close connection
											evhttp_connection_free(connection);
											
											// Cancel all client's interactions
											clients->at(connection).cancelAllInteractions();
											
											// Remove connection from list of clients
											clients->erase(connection);
										}
									}
									
									// Otherwise
									else {
									
										// Get input's length
										size_t length = evbuffer_get_length(input);
										
										// Check if connection doesn't exist
										if(!clients->count(connection)) {
										
											// Remove data from input
											evbuffer_drain(input, length);
										
											// Remove connection's buffer callbacks
											bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
										}
										
										// Otherwise
										else {
										
											// Check if getting data from input failed
											uint8_t data[length];
											if(evbuffer_copyout(input, data, length) == -1) {
											
												// Remove data from input
												evbuffer_drain(input, length);
											
												// Remove connection's buffer callbacks
												bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
												
												// Close connection
												evhttp_connection_free(connection);
												
												// Cancel all client's interactions
												clients->at(connection).cancelAllInteractions();
												
												// Remove connection from list of clients
												clients->erase(connection);
											}
											
											// Otherwise
											else {
											
												// Go through all WebSocket frames
												while(true) {
												
													// Check if frame contains an opcode
													if(length > WEBSOCKET_OPCODE_BYTE_OFFSET) {
													
														// Get opcode
														WebSocketOpcode opcode = static_cast<WebSocketOpcode>(data[WEBSOCKET_OPCODE_BYTE_OFFSET] & WEBSOCKET_OPCODE_BYTE_MASK);
														
														// Get is final frame
														const bool isFinalFrame = data[WEBSOCKET_FINAL_FRAME_BYTE_OFFSET] & WEBSOCKET_FINAL_FRAME_BYTE_MASK;
														
														// Check opcode
														switch(opcode) {
														
															// Continuation
															case WebSocketOpcode::CONTINUATION:
															
																// Check if no there is no frame to continue
																if(message->empty()) {
																
																	// Remove data from input
																	evbuffer_drain(input, length);
																
																	// Remove connection's buffer callbacks
																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																	
																	// Close connection
																	evhttp_connection_free(connection);
																	
																	// Cancel all client's interactions
																	clients->at(connection).cancelAllInteractions();
																	
																	// Remove connection from list of clients
																	clients->erase(connection);
																	
																	// Return
																	return;
																}
																
																// Break
																break;
															
															// Ping or pong
															case WebSocketOpcode::PING:
															case WebSocketOpcode::PONG:
															
																// Check if frame isn't the final frame
																if(!isFinalFrame) {
																
																	// Remove data from input
																	evbuffer_drain(input, length);
																
																	// Remove connection's buffer callbacks
																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																	
																	// Close connection
																	evhttp_connection_free(connection);
																	
																	// Cancel all client's interactions
																	clients->at(connection).cancelAllInteractions();
																	
																	// Remove connection from list of clients
																	clients->erase(connection);
																	
																	// Return
																	return;
																}
																
																// Break
																break;
															
															// Text
															case WebSocketOpcode::TEXT:
															
																// Break
																break;
															
															// Default
															default:
															
																// Remove data from input
																evbuffer_drain(input, length);
															
																// Remove connection's buffer callbacks
																bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																
																// Close connection
																evhttp_connection_free(connection);
																
																// Cancel all client's interactions
																clients->at(connection).cancelAllInteractions();
																
																// Remove connection from list of clients
																clients->erase(connection);
																
																// Return
																return;
														}
														
														// Get has extension
														const bool hasExtension = data[WEBSOCKET_EXTENSION_BYTE_OFFSET] & WEBSOCKET_EXTENSION_BYTE_MASK;
														
														// Check if has extension
														if(hasExtension) {
														
															// Remove data from input
															evbuffer_drain(input, length);
														
															// Remove connection's buffer callbacks
															bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
															
															// Close connection
															evhttp_connection_free(connection);
															
															// Cancel all client's interactions
															clients->at(connection).cancelAllInteractions();
															
															// Remove connection from list of clients
															clients->erase(connection);
															
															// Return
															return;
														}
														
														// Check if frame contains a length
														if(length > WEBSOCKET_LENGTH_BYTE_OFFSET) {
														
															// Get has mask
															const bool hasMask = data[WEBSOCKET_MASK_BYTE_OFFSET] & WEBSOCKET_MASK_BYTE_MASK;
															
															// Check if doesn't have a mask
															if(!hasMask) {
															
																// Remove data from input
																evbuffer_drain(input, length);
															
																// Remove connection's buffer callbacks
																bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																
																// Close connection
																evhttp_connection_free(connection);
																
																// Cancel all client's interactions
																clients->at(connection).cancelAllInteractions();
																
																// Remove connection from list of clients
																clients->erase(connection);
																
																// Return
																return;
															}
															
															// Get real length
															uint64_t realLength = data[WEBSOCKET_LENGTH_BYTE_OFFSET] & WEBSOCKET_LENGTH_BYTE_MASK;
															
															// Initialize mask offset
															size_t maskOffset;
															
															// Check if real length is expressed by next sixteen bits
															if(realLength == WEBSOCKET_SIXTEEN_BITS_LENGTH) {
															
																// Check of opcode is a ping or pong
																if(opcode == WebSocketOpcode::PING || opcode == WebSocketOpcode::PONG) {
																
																	// Remove data from input
																	evbuffer_drain(input, length);
																
																	// Remove connection's buffer callbacks
																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																	
																	// Close connection
																	evhttp_connection_free(connection);
																	
																	// Cancel all client's interactions
																	clients->at(connection).cancelAllInteractions();
																	
																	// Remove connection from list of clients
																	clients->erase(connection);
																	
																	// Return
																	return;
																}
															
																// Check if frame contains real length
																if(length > WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint16_t)) {
																
																	// Go through all real length bytes
																	realLength = 0;
																	for(size_t i = 0; i < sizeof(uint16_t); ++i) {
																	
																		// Include length byte in real length
																		realLength |= data[WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint16_t) - sizeof(uint8_t) * i] << (Common::BITS_IN_A_BYTE * i);
																	}
																	
																	// Set mask offsets
																	maskOffset = WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint16_t) + sizeof(uint8_t);
																}
																
																// Otherwise
																else {
																
																	// Break
																	break;
																}
															}
															
															// Otherwise check if real length is expressed by next sixty-three bits
															else if(realLength == WEBSOCKET_SIXTY_THREE_BITS_LENGTH) {
															
																// Check of opcode is a ping or pong
																if(opcode == WebSocketOpcode::PING || opcode == WebSocketOpcode::PONG) {
																
																	// Remove data from input
																	evbuffer_drain(input, length);
																
																	// Remove connection's buffer callbacks
																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																	
																	// Close connection
																	evhttp_connection_free(connection);
																	
																	// Cancel all client's interactions
																	clients->at(connection).cancelAllInteractions();
																	
																	// Remove connection from list of clients
																	clients->erase(connection);
																	
																	// Return
																	return;
																}
															
																// Check if frame contains real length
																if(length > WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint64_t)) {
																
																	// Go through all real length bytes
																	realLength = 0;
																	for(size_t i = 0; i < sizeof(uint64_t); ++i) {
																	
																		// Include length byte in real length
																		realLength |= static_cast<uint64_t>(data[WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint64_t) - sizeof(uint8_t) * i]) << (Common::BITS_IN_A_BYTE * i);
																	}
																	
																	// Check if real length is invalid
																	if(realLength > INT64_MAX) {
																	
																		// Remove data from input
																		evbuffer_drain(input, length);
																	
																		// Remove connection's buffer callbacks
																		bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																		
																		// Close connection
																		evhttp_connection_free(connection);
																		
																		// Cancel all client's interactions
																		clients->at(connection).cancelAllInteractions();
																		
																		// Remove connection from list of clients
																		clients->erase(connection);
																		
																		// Return
																		return;
																	}
																	
																	// Set mask offsets
																	maskOffset = WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint64_t) + sizeof(uint8_t);
																}
																
																// Otherwise
																else {
																
																	// Break
																	break;
																}
															}
															
															// Otherwise
															else {
															
																// Set mask offsets
																maskOffset = WEBSOCKET_LENGTH_BYTE_OFFSET + sizeof(uint8_t);
															}
															
															// Check if real length is invalid
															if(!isFinalFrame && !realLength) {
															
																// Remove data from input
																evbuffer_drain(input, length);
															
																// Remove connection's buffer callbacks
																bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																
																// Close connection
																evhttp_connection_free(connection);
																
																// Cancel all client's interactions
																clients->at(connection).cancelAllInteractions();
																
																// Remove connection from list of clients
																clients->erase(connection);
																
																// Return
																return;
															}
															
															// Check if frame contains the mask and data
															if(length >= maskOffset + WEBSOCKET_MASK_LENGTH + realLength) {
															
																// Go through all bytes of data
																for(uint64_t i = 0; i < realLength; ++i) {
																
																	// Append unmasked byte to the message
																	message->push_back(data[maskOffset + WEBSOCKET_MASK_LENGTH + i] ^ data[maskOffset + i % WEBSOCKET_MASK_LENGTH]);
																}
																
																// Check if removing frame from input failed
																if(evbuffer_drain(input, maskOffset + WEBSOCKET_MASK_LENGTH + realLength)) {
																
																	// Remove data from input
																	evbuffer_drain(input, length);
																	
																	// Remove connection's buffer callbacks
																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																	
																	// Close connection
																	evhttp_connection_free(connection);
																	
																	// Cancel all client's interactions
																	clients->at(connection).cancelAllInteractions();
																	
																	// Remove connection from list of clients
																	clients->erase(connection);
																	
																	// Return
																	return;
																}
																
																// Remove frame's length from length
																length -= maskOffset + WEBSOCKET_MASK_LENGTH + realLength;
																
																// Check is the final frame
																if(isFinalFrame) {
																
																	// Check opcode
																	switch(opcode) {
																	
																		// Text
																		case WebSocketOpcode::TEXT:
																			
																			{
																				// Initialize response
																				string response;
																		
																				// Check if current message is JSON
																				Json jsonMessage;
																				if(jsonMessage.decode(*message) && jsonMessage.getType() == Json::Type::OBJECT) {
																				
																					// Check if message contains an index
																					if(jsonMessage.getObjectValue().count("Index")) {
																					
																						// Check if index is valid
																						Json::Number integerComponent;
																						if(jsonMessage.getObjectValue().at("Index")->getType() == Json::Type::NUMBER && jsonMessage.getObjectValue().at("Index")->getNumberValue() >= 0 && jsonMessage.getObjectValue().at("Index")->getNumberValue() <= MAXIMUM_SAFE_INTEGER && modf(jsonMessage.getObjectValue().at("Index")->getNumberValue(), &integerComponent) == 0) {
																					
																							// Get index
																							const Json::Number &index = jsonMessage.getObjectValue().at("Index")->getNumberValue();
																						
																							// Check if message contains a valid request
																							if(jsonMessage.getObjectValue().count("Request") && jsonMessage.getObjectValue().at("Request")->getType() == Json::Type::STRING) {
																					
																								// Get JSON request
																								const Json::String &jsonRequest = jsonMessage.getObjectValue().at("Request")->getStringValue();
																								
																								// Check if the message request is to create a URL
																								if(jsonRequest == "Create URL") {
																								
																									// Initialize URL
																									string url;
																								
																									// Loop until an unused URL is found
																									while(true) {
																									
																										// Set URL to random URL
																										url = getRandomUrl(*torHiddenServiceAddress);
																										
																										// Initialize URL in use
																										bool urlInUse = false;
																										
																										// Go through all URLs
																										for(unordered_map<string, unordered_set<string>>::const_iterator i = urls->cbegin(); i != urls->cend(); ++i) {
																										
																											// Get session's URLs
																											const unordered_set<string> &sessionsUrls = i->second;
																											
																											// Check if URL already exists
																											if(sessionsUrls.count(url)) {
																											
																												// Set URL in use
																												urlInUse = true;
																											
																												// Break
																												break;
																											}
																										}
																										
																										// Check if URL isn't in use
																										if(!urlInUse) {
																										
																											// Break
																											break;
																										}
																									}
																									
																									// Get session's URLs
																									unordered_set<string> &sessionsUrls = urls->at(clients->at(connection).getSessionId());
																									
																									// Add URL to list of session's URLs
																									sessionsUrls.emplace(url);
																									
																									// Set response
																									response = Json(Json::Object{
																										{"Index", make_unique<Json>(index)},
																										{"Response", make_unique<Json>(url)}
																									}).encode();
																								}
																								
																								// Otherwise check if message request is to change a URL
																								else if(jsonRequest == "Change URL") {
																								
																									// Check if URL isn't provided or is invalid
																									if(!jsonMessage.getObjectValue().count("URL") || jsonMessage.getObjectValue().at("URL")->getType() != Json::Type::STRING) {
																									
																										// Set response
																										response = Json(Json::Object{
																											{"Index", make_unique<Json>(index)},
																											{"Error", make_unique<Json>(jsonMessage.getObjectValue().count("URL") ? "Invalid URL parameter" : "Missing URL parameter")}
																										}).encode();
																									}
																									
																									// Otherwise
																									else {
																									
																										// Get old URL
																										const Json::String oldUrl = Common::toLowerCase(jsonMessage.getObjectValue().at("URL")->getStringValue());
																										
																										// Get session's URLs
																										unordered_set<string> &sessionsUrls = urls->at(clients->at(connection).getSessionId());
																										
																										// Check if session owns the old URL
																										if(sessionsUrls.count(oldUrl)) {
																										
																											// Initialize URL
																											string url;
																										
																											// Loop until an unused URL is found
																											while(true) {
																											
																												// Set URL to random URL
																												url = getRandomUrl(*torHiddenServiceAddress);
																												
																												// Initialize URL in use
																												bool urlInUse = false;
																												
																												// Go through all URLs
																												for(unordered_map<string, unordered_set<string>>::const_iterator i = urls->cbegin(); i != urls->cend(); ++i) {
																												
																													// Get session's URLs
																													const unordered_set<string> &sessionsUrls = i->second;
																													
																													// Check if URL already exists
																													if(sessionsUrls.count(url)) {
																													
																														// Set URL in use
																														urlInUse = true;
																													
																														// Break
																														break;
																													}
																												}
																												
																												// Check if URL isn't in use
																												if(!urlInUse) {
																												
																													// Break
																													break;
																												}
																											}
																											
																											// Remove old URL from list of session's URLs
																											sessionsUrls.erase(oldUrl);
																											
																											// Add URL to list of session's URLs
																											sessionsUrls.emplace(url);
																											
																											// Set response
																											response = Json(Json::Object{
																												{"Index", make_unique<Json>(index)},
																												{"Response", make_unique<Json>(url)}
																											}).encode();
																										}
																										
																										// Otherwis
																										else {
																										
																											// Set response
																											response = Json(Json::Object{
																												{"Index", make_unique<Json>(index)},
																												{"Error", make_unique<Json>("URL doesn't exist or it isn't owned by your session ID")}
																											}).encode();
																										}
																									}
																								}
																								
																								// Otherwise check if the message request is to delete a URL
																								else if(jsonRequest == "Delete URL") {
																								
																									// Check if URL isn't provided or is invalid
																									if(!jsonMessage.getObjectValue().count("URL") || jsonMessage.getObjectValue().at("URL")->getType() != Json::Type::STRING) {
																									
																										// Set response
																										response = Json(Json::Object{
																											{"Index", make_unique<Json>(index)},
																											{"Error", make_unique<Json>(jsonMessage.getObjectValue().count("URL") ? "Invalid URL parameter" : "Missing URL parameter")}
																										}).encode();
																									}
																									
																									// Otherwise
																									else {
																									
																										// Get URL
																										const Json::String url = Common::toLowerCase(jsonMessage.getObjectValue().at("URL")->getStringValue());
																								
																										// Get session's URLs
																										unordered_set<string> &sessionsUrls = urls->at(clients->at(connection).getSessionId());
																										
																										// Check if session owns the URL
																										if(sessionsUrls.count(url)) {
																										
																											// Delete URL
																											sessionsUrls.erase(url);
																										
																											// Set response
																											response = Json(Json::Object{
																												{"Index", make_unique<Json>(index)},
																												{"Response", make_unique<Json>(true)}
																											}).encode();
																										}
																										
																										// Otherwise
																										else {
																										
																											// Set response
																											response = Json(Json::Object{
																												{"Index", make_unique<Json>(index)},
																												{"Error", make_unique<Json>("URL doesn't exist or it isn't owned by your session ID")}
																											}).encode();
																										}
																									}
																								}
																								
																								// Otherwise check if message request is to check if they own a URL
																								else if(jsonRequest == "Own URL") {
																								
																									// Check if URL isn't provided or is invalid
																									if(!jsonMessage.getObjectValue().count("URL") || jsonMessage.getObjectValue().at("URL")->getType() != Json::Type::STRING) {
																									
																										// Set response
																										response = Json(Json::Object{
																											{"Index", make_unique<Json>(index)},
																											{"Error", make_unique<Json>(jsonMessage.getObjectValue().count("URL") ? "Invalid URL parameter" : "Missing URL parameter")}
																										}).encode();
																									}
																									
																									// Otherwise
																									else {
																									
																										// Get URL
																										const Json::String url = Common::toLowerCase(jsonMessage.getObjectValue().at("URL")->getStringValue());
																										
																										// Get session's URLs
																										const unordered_set<string> &sessionsUrls = urls->at(clients->at(connection).getSessionId());
																										
																										// Check if session owns the URL
																										if(sessionsUrls.count(url)) {
																										
																											// Set response
																											response = Json(Json::Object{
																												{"Index", make_unique<Json>(index)},
																												{"Response", make_unique<Json>(true)}
																											}).encode();
																										}
																										
																										// Otherwise
																										else {
																										
																											// Set response
																											response = Json(Json::Object{
																												{"Index", make_unique<Json>(index)},
																												{"Response", make_unique<Json>(false)}
																											}).encode();
																										}
																									}
																								}
																								
																								// Otherwise
																								else {
																								
																									// Set response
																									response = Json(Json::Object{
																										{"Index", make_unique<Json>(index)},
																										{"Error", make_unique<Json>("Unknown request")}
																									}).encode();
																								}
																							}
																							
																							// Otherwise
																							else {
																							
																								// Set response
																								response = Json(Json::Object{
																									{"Index", make_unique<Json>(index)},
																									{"Error", make_unique<Json>(jsonMessage.getObjectValue().count("Request") ? "Invalid request parameter" : "Missing request parameter")}
																								}).encode();
																							}
																						}
																						
																						// Otherwise
																						else {
																						
																							// Set response
																							response = Json(Json::Object{
																								{"Error", make_unique<Json>("Invalid index parameter")}
																							}).encode();
																						}
																					}
																					
																					// Otherwise check if message contains a interaction
																					else if(jsonMessage.getObjectValue().count("Interaction")) {
																					
																						// Check if interaction is valid
																						Json::Number integerComponent;
																						if(jsonMessage.getObjectValue().at("Interaction")->getType() == Json::Type::NUMBER && jsonMessage.getObjectValue().at("Interaction")->getNumberValue() >= 0 && jsonMessage.getObjectValue().at("Interaction")->getNumberValue() <= MAXIMUM_SAFE_INTEGER && modf(jsonMessage.getObjectValue().at("Interaction")->getNumberValue(), &integerComponent) == 0) {
																						
																							// Get interaction index
																							const Json::Number &interactionIndex = jsonMessage.getObjectValue().at("Interaction")->getNumberValue();
																							
																							// Check if interaction currently exists
																							evhttp_request *request = clients->at(connection).getInteraction(interactionIndex);
																							if(request) {
																							
																								// Remove interaction from client
																								clients->at(connection).removeInteraction(interactionIndex);
																							
																								// Check if message contains valid data
																								if(jsonMessage.getObjectValue().count("Data") && jsonMessage.getObjectValue().at("Data")->getType() == Json::Type::STRING) {
																								
																									// Get data
																									const Json::String &data = jsonMessage.getObjectValue().at("Data")->getStringValue();
																									
																									// Try
																									bool invalidData = false;
																									vector<uint8_t> decodedData;
																									try {
																									
																										// Decode data
																										decodedData = data.empty() ? URL_DOESNT_EXIST : Json::base64Decode(data);
																									}
																									
																									// Catch errors
																									catch(...) {
																									
																										// Set invalid data
																										invalidData = true;
																									}
																									
																									// Check if data is invalid
																									if(invalidData) {
																									
																										// Set response
																										response = Json(Json::Object{
																											{"Interaction", make_unique<Json>(interactionIndex)},
																											{"Error", make_unique<Json>("Invalid data parameter")}
																										}).encode();
																									}
																									
																									// Otherwise
																									else {
																									
																										// Get request's buffer
																										bufferevent *requestsBuffer = evhttp_connection_get_bufferevent(evhttp_request_get_connection(request));
																									
																										// Set type to provided type otherwise HTML if not provided
																										const string type = (jsonMessage.getObjectValue().count("Type") && jsonMessage.getObjectValue().at("Type")->getType() == Json::Type::STRING) ? jsonMessage.getObjectValue().at("Type")->getStringValue() : "text/html";
																										
																										// Check if setting request's content type failed
																										if(!decodedData.empty() && evhttp_add_header(evhttp_request_get_output_headers(request), "Content-Type", type.c_str())) {
																										
																											// Reply with internal server error to request
																											evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
																											
																											// Remove request's buffer callbacks
																											bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
																											
																											// Set response
																											response = Json(Json::Object{
																												{"Interaction", make_unique<Json>(interactionIndex)},
																												{"Status", make_unique<Json>("Failed")}
																											}).encode();
																										}
																										
																										// Otherwise
																										else {
																										
																											// Check if creating request's buffer callbacks argument failed
																											unique_ptr<tuple<evhttp_connection *, unordered_map<evhttp_connection *, Client> *, const Json::Number>> requestsBufferCallbacksArgument = make_unique<tuple<evhttp_connection *, unordered_map<evhttp_connection *, Client> *, const Json::Number>>(connection, clients, interactionIndex);
																											if(!connectionsBufferCallbacksArgument) {
																											
																												// Reply with internal server error to request
																												evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
																												
																												// Remove request's buffer callbacks
																												bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
																												
																												// Set response
																												response = Json(Json::Object{
																													{"Interaction", make_unique<Json>(interactionIndex)},
																													{"Status", make_unique<Json>("Failed")}
																												}).encode();
																											}
																											
																											// Otherwise
																											else {
																											
																												// Check if creating buffer failed
																												unique_ptr<evbuffer, decltype(&evbuffer_free)> buffer(evbuffer_new(), evbuffer_free);
																												if(!buffer) {
																												
																													// Reply with internal server error to request
																													evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
																													
																													// Remove request's buffer callbacks
																													bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
																													
																													// Set response
																													response = Json(Json::Object{
																														{"Interaction", make_unique<Json>(interactionIndex)},
																														{"Status", make_unique<Json>("Failed")}
																													}).encode();
																												}
																											
																												// Otherwise check if adding decoded data to buffer failed
																												else if(evbuffer_add(buffer.get(), decodedData.data(), decodedData.size())) {
																												
																													// Reply with internal server error to request
																													evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
																													
																													// Remove request's buffer callbacks
																													bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
																													
																													// Set response
																													response = Json(Json::Object{
																														{"Interaction", make_unique<Json>(interactionIndex)},
																														{"Status", make_unique<Json>("Failed")}
																													}).encode();
																												}
																												
																												// Otherwise
																												else {
																												
																													// Set status to provided status otherwise ok if not provided
																													const int status = (jsonMessage.getObjectValue().count("Status") && jsonMessage.getObjectValue().at("Status")->getType() == Json::Type::NUMBER && jsonMessage.getObjectValue().at("Status")->getNumberValue() >= 0 && jsonMessage.getObjectValue().at("Status")->getNumberValue() <= INT_MAX && modf(jsonMessage.getObjectValue().at("Status")->getNumberValue(), &integerComponent) == 0) ? jsonMessage.getObjectValue().at("Status")->getNumberValue() : HTTP_OK;
																													
																													// Reply with status to request
																													evhttp_send_reply(request, status, nullptr, buffer.get());
																													
																													// Set request's buffer callbacks
																													bufferevent_setcb(requestsBuffer, nullptr, ([](bufferevent *requestsBuffer, void *argument) {
																													
																														// Get request's buffer callbacks argument from argument
																														unique_ptr<tuple<evhttp_connection *, unordered_map<evhttp_connection *, Client> *, const Json::Number>> requestsBufferCallbacksArgument(reinterpret_cast<tuple<evhttp_connection *, unordered_map<evhttp_connection *, Client> *, const Json::Number> *>(argument));
																														
																														// Get connection from request's buffer callbacks argument
																														evhttp_connection *connection = get<0>(*requestsBufferCallbacksArgument);
																														
																														// Get clients from request's buffer callbacks argument
																														unordered_map<evhttp_connection *, Client> *clients = get<1>(*requestsBufferCallbacksArgument);
																														
																														// Get interaction index from request's buffer callbacks argument
																														const Json::Number interactionIndex = get<2>(*requestsBufferCallbacksArgument);
																													
																														// Remove request's buffer callbacks
																														bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
																														
																														// Check if connection still exists
																														if(clients->count(connection)) {
																														
																															// Check if getting connection's buffer failed
																															bufferevent *connectionsBuffer = evhttp_connection_get_bufferevent(connection);
																															if(!connectionsBuffer) {
																															
																																// Close connection
																																evhttp_connection_free(connection);
																																
																																// Cancel all client's interactions
																																clients->at(connection).cancelAllInteractions();
																																
																																// Remove connection from list of clients
																																clients->erase(connection);
																															}
																															
																															// Otherwise
																															else {
																															
																																// Set response
																																const string response = Json(Json::Object{
																																	{"Interaction", make_unique<Json>(interactionIndex)},
																																	{"Status", make_unique<Json>("Succeeded")}
																																}).encode();
																															
																																// Get response message
																																const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
																																
																																// Check if sending response message to client failed
																																if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
																																
																																	// Check if getting connection's buffer input was successful
																																	evbuffer *input = bufferevent_get_input(connectionsBuffer);
																																	if(input) {
																																	
																																		// Remove data from input
																																		evbuffer_drain(input, evbuffer_get_length(input));
																																	}
																																	
																																	// Remove connection's buffer callbacks
																																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																																	
																																	// Close connection
																																	evhttp_connection_free(connection);
																																	
																																	// Cancel all client's interactions
																																	clients->at(connection).cancelAllInteractions();
																																	
																																	// Remove connection from list of clients
																																	clients->erase(connection);
																																}
																															}
																														}
																														
																													}), ([](bufferevent *requestsBuffer, short event, void *argument) {
																													
																														// Get request's buffer callbacks argument from argument
																														unique_ptr<tuple<evhttp_connection *, unordered_map<evhttp_connection *, Client> *, const Json::Number>> requestsBufferCallbacksArgument(reinterpret_cast<tuple<evhttp_connection *, unordered_map<evhttp_connection *, Client> *, const Json::Number> *>(argument));
																														
																														// Get connection from request's buffer callbacks argument
																														evhttp_connection *connection = get<0>(*requestsBufferCallbacksArgument);
																														
																														// Get clients from request's buffer callbacks argument
																														unordered_map<evhttp_connection *, Client> *clients = get<1>(*requestsBufferCallbacksArgument);
																														
																														// Get interaction index from request's buffer callbacks argument
																														const Json::Number interactionIndex = get<2>(*requestsBufferCallbacksArgument);
																													
																														// Remove request's buffer callbacks
																														bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
																														
																														// Check if connection still exists
																														if(clients->count(connection)) {
																														
																															// Check if getting connection's buffer failed
																															bufferevent *connectionsBuffer = evhttp_connection_get_bufferevent(connection);
																															if(!connectionsBuffer) {
																															
																																// Close connection
																																evhttp_connection_free(connection);
																																
																																// Cancel all client's interactions
																																clients->at(connection).cancelAllInteractions();
																																
																																// Remove connection from list of clients
																																clients->erase(connection);
																															}
																															
																															// Otherwise
																															else {
																															
																																// Set response
																																const string response = Json(Json::Object{
																																	{"Interaction", make_unique<Json>(interactionIndex)},
																																	{"Status", make_unique<Json>("Failed")}
																																}).encode();
																															
																																// Get response message
																																const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
																																
																																// Check if sending response message to client failed
																																if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
																																
																																	// Check if getting connection's buffer input was successful
																																	evbuffer *input = bufferevent_get_input(connectionsBuffer);
																																	if(input) {
																																	
																																		// Remove data from input
																																		evbuffer_drain(input, evbuffer_get_length(input));
																																	}
																																	
																																	// Remove connection's buffer callbacks
																																	bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																																	
																																	// Close connection
																																	evhttp_connection_free(connection);
																																	
																																	// Cancel all client's interactions
																																	clients->at(connection).cancelAllInteractions();
																																	
																																	// Remove connection from list of clients
																																	clients->erase(connection);
																																}
																															}
																														}
																														
																													}), requestsBufferCallbacksArgument.get());
																													
																													// Release request's callback argument
																													requestsBufferCallbacksArgument.release();
																												}
																											}
																										}
																									}
																								}
																								
																								// Otherwise
																								else {
																								
																									// Set response
																									response = Json(Json::Object{
																										{"Interaction", make_unique<Json>(interactionIndex)},
																										{"Error", make_unique<Json>((jsonMessage.getObjectValue().count("Data") && jsonMessage.getObjectValue().at("Data")->getType() != Json::Type::STRING) ? "Invalid data parameter" : "Missing data parameter")}
																									}).encode();
																								}
																							}
																							
																							// Otherwise
																							else {
																							
																								// Set response
																								response = Json(Json::Object{
																									{"Interaction", make_unique<Json>(interactionIndex)},
																									{"Error", make_unique<Json>("Interaction doesn't exist or it was already processed")}
																								}).encode();
																							}
																						}
																						
																						// Otherwise
																						else {
																						
																							// Set response
																							response = Json(Json::Object{
																								{"Error", make_unique<Json>("Invalid interaction parameter")}
																							}).encode();
																						}
																					}
																					
																					// Otherwise
																					else {
																					
																						// Set response
																						response = Json(Json::Object{
																							{"Error", make_unique<Json>("Unknown message type")}
																						}).encode();
																					}
																				}
																				
																				// Otherwise
																				else {
																				
																					// Set response
																					response = Json(Json::Object{
																						{"Error", make_unique<Json>("Message is not JSON")}
																					}).encode();
																				}
																				
																				// Check if response exists
																				if(!response.empty()) {
																				
																					// Get response message
																					const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
																					
																					// Check if sending response message to client failed
																					if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
																					
																						// Remove data from input
																						evbuffer_drain(input, length);
																						
																						// Remove connection's buffer callbacks
																						bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																						
																						// Close connection
																						evhttp_connection_free(connection);
																						
																						// Cancel all client's interactions
																						clients->at(connection).cancelAllInteractions();
																						
																						// Remove connection from list of clients
																						clients->erase(connection);
																						
																						// Return
																						return;
																					}
																				}
																			}
																		
																			// Break
																			break;
																		
																		// Ping
																		case WebSocketOpcode::PING:
																		
																			{
																				// Get pong message
																				const vector<uint8_t> pongMessage = createWebSocketResponse(*message, WebSocketOpcode::PONG);
																				
																				// Check if sending pong message to client failed
																				if(bufferevent_write(connectionsBuffer, pongMessage.data(), pongMessage.size())) {
																				
																					// Remove data from input
																					evbuffer_drain(input, length);
																					
																					// Remove connection's buffer callbacks
																					bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
																					
																					// Close connection
																					evhttp_connection_free(connection);
																					
																					// Cancel all client's interactions
																					clients->at(connection).cancelAllInteractions();
																					
																					// Remove connection from list of clients
																					clients->erase(connection);
																					
																					// Return
																					return;
																				}
																			}
																		
																			// Break
																			break;
																		
																		// Pong or default
																		case WebSocketOpcode::PONG:
																		default:
																		
																			// Break
																			break;
																	}
																	
																	// Clear message
																	message->clear();
																}
															}
															
															// Otherwise
															else {
															
																// break
																break;
															}
														}
														
														// Otherwise
														else {
														
															// break
															break;
														}
													}
													
													// Otherwise
													else {
													
														// Break
														break;
													}
												}
												
												// Release message
												message.release();
												
												// Release connection's buffer callbacsk argument
												connectionsBufferCallbacksArgument.release();
											}
										}
									}
								
								}), nullptr, ([](bufferevent *connectionsBuffer, short event, void *argument) {
								
									// Get connection's buffer callbacks argument from argument
									unique_ptr<tuple<evhttp_connection *, string *, const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *>> connectionsBufferCallbacksArgument(reinterpret_cast<tuple<evhttp_connection *, string *, const string *, unordered_map<evhttp_connection *, Client> *, unordered_map<string, unordered_set<string>> *> *>(argument));
									
									// Get connection from connection's buffer callbacks argument
									evhttp_connection *connection = get<0>(*connectionsBufferCallbacksArgument);
									
									// Get message from connection's buffer callbacks argument
									unique_ptr<string> message(get<1>(*connectionsBufferCallbacksArgument));
									
									// Get clients from connection's buffer callbacks argument
									unordered_map<evhttp_connection *, Client> *clients = get<3>(*connectionsBufferCallbacksArgument);
									
									// Check if getting connection's buffer input was successful
									evbuffer *input = bufferevent_get_input(connectionsBuffer);
									if(input) {
									
										// Remove data from input
										evbuffer_drain(input, evbuffer_get_length(input));
									}
									
									// Remove connection's buffer callbacks
									bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
									
									// Check if connection still exists
									if(clients->count(connection)) {
									
										// Close connection
										evhttp_connection_free(connection);
										
										// Cancel all client's interactions
										clients->at(connection).cancelAllInteractions();
										
										// Remove connection from list of clients
										clients->erase(connection);
									}
									
								}), connectionsBufferCallbacksArgument.get());
								
								// Release message
								message.release();
								
								// Release connection's buffer callbacsk argument
								connectionsBufferCallbacksArgument.release();
							}
						}
					}
				}
			}
			
			// Otherwise
			else {
			
				// Reply with bad request error to request
				evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
			}
		}
		
	}), &httpServerRequestCallbackArgument);
	
	// Set HTTP server request callback
	evhttp_set_gencb(httpServer.get(), ([](evhttp_request *request, void *argument) {
	
		// Check if setting request's CORS headers failed
		if(evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Origin", "*") || evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Headers", "*")) {
		
			// Reply with internal server error to request
			evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
		}
		
		// Otherwise check if request doesn't have a URI
		else if(!evhttp_request_get_uri(request) || !strlen(evhttp_request_get_uri(request))) {
		
			// Reply with bad request error to request
			evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
		}
		
		// Otherwise check if OPTIONS request
		else if(evhttp_request_get_command(request) == EVHTTP_REQ_OPTIONS) {
		
			// Check if setting CORS header failed
			if(evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Methods", "GET, OPTIONS")) {
			
				// Reply with internal server error to request
				evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
			}
			
			// Otherwise
			else {
			
				// Reply with ok to request
				evhttp_send_reply(request, HTTP_OK, nullptr, nullptr);
			}
		}
		
		// Otherwise
		else {
	
			// Reply with not found error to request
			evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
		}
		
	}), nullptr);
	
	// Check if creating Tor server failed
	unique_ptr<evhttp, decltype(&evhttp_free)> torServer(evhttp_new(eventBase.get()), evhttp_free);
	if(!torServer) {
	
		// Display message
		cout << "Creating Tor server failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Set Tor server's maximum header size
	evhttp_set_max_headers_size(torServer.get(), MAXIMUM_HEADERS_SIZE);
	
	// Set Tor server's maximum body size
	evhttp_set_max_body_size(torServer.get(), MAXIMUM_BODY_SIZE);
	
	// Set Tor server to only allow POST and OPTIONS requests
	evhttp_set_allowed_methods(torServer.get(), EVHTTP_REQ_POST | EVHTTP_REQ_OPTIONS);
	
	// Initialize Tor server request callback argument
	tuple<const string *, unordered_map<evhttp_connection *, Client> *, const unordered_map<string, unordered_set<string>> *> torServerRequestCallbackArgument(&torHiddenServiceAddress, &clients, &urls);
	
	// Set Tor server request callback
	evhttp_set_gencb(torServer.get(), ([](evhttp_request *request, void *argument) {
	
		// Get Tor server request callback argument from argument
		tuple<const string *, unordered_map<evhttp_connection *, Client> *, const unordered_map<string, unordered_set<string>> *> *torServerRequestCallbackArgument = reinterpret_cast<tuple<const string *, unordered_map<evhttp_connection *, Client> *, const unordered_map<string, unordered_set<string>> *> *>(argument);
		
		// Get Tor hidden service address from Tor server request callback argument
		const string *torHiddenServiceAddress = get<0>(*torServerRequestCallbackArgument);
		
		// Get clients from Tor server request callback argument
		unordered_map<evhttp_connection *, Client> *clients = get<1>(*torServerRequestCallbackArgument);
		
		// Get URLs from Tor server request callback argument
		const unordered_map<string, unordered_set<string>> *urls = get<2>(*torServerRequestCallbackArgument);
		
		// Check if setting request's CORS headers failed
		if(evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Origin", "*") || evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Headers", "Content-Type, Authorization")) {
		
			// Reply with internal server error to request
			evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
		}
		
		// Otherwise check if request doesn't have a URI
		else if(!evhttp_request_get_uri(request) || !strlen(evhttp_request_get_uri(request))) {
		
			// Reply with bad request error to request
			evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
		}
		
		// Otherwise check if OPTIONS request
		else if(evhttp_request_get_command(request) == EVHTTP_REQ_OPTIONS) {
		
			// Check if setting CORS header failed
			if(evhttp_add_header(evhttp_request_get_output_headers(request), "Access-Control-Allow-Methods", "POST, OPTIONS")) {
			
				// Reply with internal server error to request
				evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
			}
			
			// Otherwise
			else {
			
				// Reply with ok to request
				evhttp_send_reply(request, HTTP_OK, nullptr, nullptr);
			}
		}
		
		// Otherwise
		else {
		
			// Check if parsing request's URI failed
			unique_ptr<evhttp_uri, decltype(&evhttp_uri_free)> uri(evhttp_uri_parse(evhttp_request_get_uri(request)), evhttp_uri_free);
			if(!uri) {
			
				// Reply with bad request error to request
				evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
			}
			
			// Otherwise
			else {
			
				// Set path to the URI's path
				const string path = (evhttp_uri_get_path(uri.get()) && strlen(evhttp_uri_get_path(uri.get()))) ? evhttp_uri_get_path(uri.get()) : "/";
				
				// Check if path doesn't contain a URL delimeter
				const size_t urlDelimiter = path.find('/', sizeof('/'));
				if(path[0] != '/' || urlDelimiter == string::npos) {
				
					// Reply with not found error to request
					evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
				}
				
				// Otherwise
				else {
				
					// Set URL
					const string url = "http://" + *torHiddenServiceAddress + ".onion" + Common::toLowerCase(path.substr(0, urlDelimiter));
					
					// Set API
					const string api = path.substr(urlDelimiter);
				
					// Initialize connection
					evhttp_connection *connection = nullptr;
					
					// Go through all URLs
					for(unordered_map<string, unordered_set<string>>::const_iterator i = urls->cbegin(); i != urls->cend(); ++i) {
					
						// Get session's URLs
						const unordered_set<string> &sessionsUrls = i->second;
						
						// Check if session owns the URL
						if(sessionsUrls.count(url)) {
						
							// Get session ID
							const string &sessionId = i->first;
							
							// Go through all clients
							for(unordered_map<evhttp_connection *, Client>::const_iterator j = clients->cbegin(); j != clients->cend(); ++j) {
							
								// Get client
								const Client &client = j->second;
								
								// Check if session ID is for the client
								if(client.getSessionId() == sessionId) {
								
									// Set connection to the client
									connection = j->first;
								
									// Break
									break;
								}
							}
						
							// Break
							break;
						}
					}
					
					// Check if URL doesn't exist or client isn't connected
					if(!connection) {
					
						// Reply with not found error to request
						evhttp_send_reply(request, HTTP_NOTFOUND, nullptr, nullptr);
					}
					
					// Otherwise
					else {
					
						// Check if getting connection's buffer failed
						bufferevent *connectionsBuffer = evhttp_connection_get_bufferevent(connection);
						if(!connectionsBuffer) {
						
							// Reply with internal server error to request
							evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
						
							// Close connection
							evhttp_connection_free(connection);
							
							// Cancel all client's interactions
							clients->at(connection).cancelAllInteractions();
							
							// Remove connection from list of clients
							clients->erase(connection);
						}
						
						// Otherwise
						else {
						
							// Try
							Json::Number interactionIndex;
							try {
							
								// Get client's next interaction index
								interactionIndex = clients->at(connection).getNextInteractionIndex();
							}
							
							// Catch errors
							catch(...) {
							
								// Reply with internal server error to request
								evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
								
								// Return
								return;
							}
							
							// Initialize content type
							string contentType;
							
							// Check if getting request's headers was successful
							const evkeyvalq *headers = evhttp_request_get_input_headers(request);
							if(headers) {
							
								// Get header's content type
								const char *type = evhttp_find_header(headers, "Content-Type");
								
								// Set content type to the content type if provided otherwise to HTML
								contentType = type ? type : "text/html";
							}
							
							// Initialize data
							string data;
							
							// Check if getting request's input was succesful
							evbuffer *input = evhttp_request_get_input_buffer(request);
							if(input) {
							
								// Get input's length
								const size_t length = evbuffer_get_length(input);
								
								// Check if input's length is invaid
								if(!length) {
								
									// Reply with bad request error to request
									evhttp_send_reply(request, HTTP_BADREQUEST, nullptr, nullptr);
									
									// Return
									return;
								}
								
								// Check if getting data from input failed
								vector<uint8_t> buffer(length);
								if(evbuffer_copyout(input, buffer.data(), length) == -1) {
								
									// Reply with internal server error to request
									evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
									
									// Return
									return;
								}
								
								// Set data to buffer
								data = Json::base64Encode(buffer);
							}
						
							// Set response
							const string response = Json(Json::Object{
								{"Interaction", make_unique<Json>(interactionIndex)},
								{"URL", make_unique<Json>(url)},
								{"API", make_unique<Json>(api)},
								{"Type", make_unique<Json>(contentType)},
								{"Data", make_unique<Json>(data)}
							}).encode();
						
							// Get response message
							const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
							
							// Check if sending response message to client failed
							if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
							
								// Reply with internal server error to request
								evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
								
								// Check if getting connection's buffer input was successful
								evbuffer *input = bufferevent_get_input(connectionsBuffer);
								if(input) {
								
									// Remove data from input
									evbuffer_drain(input, evbuffer_get_length(input));
								}
								
								// Remove connection's buffer callbacks
								bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
							
								// Close connection
								evhttp_connection_free(connection);
								
								// Cancel all client's interactions
								clients->at(connection).cancelAllInteractions();
								
								// Remove connection from list of clients
								clients->erase(connection);
							}
							
							// Otherwise
							else {
							
								// Check if adding interaction to client failed
								if(!clients->at(connection).addInteraction(interactionIndex, request)) {
								
									// Reply with internal server error to request
									evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
									
									// Set response
									const string response = Json(Json::Object{
										{"Interaction", make_unique<Json>(interactionIndex)},
										{"Status", make_unique<Json>("Failed")}
									}).encode();
								
									// Get response message
									const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
									
									// Check if sending response message to client failed
									if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
									
										// Check if getting connection's buffer input was successful
										evbuffer *input = bufferevent_get_input(connectionsBuffer);
										if(input) {
										
											// Remove data from input
											evbuffer_drain(input, evbuffer_get_length(input));
										}
										
										// Remove connection's buffer callbacks
										bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
										
										// Close connection
										evhttp_connection_free(connection);
										
										// Cancel all client's interactions
										clients->at(connection).cancelAllInteractions();
										
										// Remove connection from list of clients
										clients->erase(connection);
									}
								}
								
								// Otherwise
								else {
							
									// Check if creating request's buffer callbacks argument failed
									unique_ptr<tuple<unordered_map<evhttp_connection *, Client> *, evhttp_connection *, const Json::Number>> requestsBufferCallbacksArgument = make_unique<tuple<unordered_map<evhttp_connection *, Client> *, evhttp_connection *, const Json::Number>>(clients, connection, interactionIndex);
									if(!requestsBufferCallbacksArgument) {
									
										// Reply with internal server error to request
										evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
										
										// Remove interaction from client
										clients->at(connection).removeInteraction(interactionIndex);
										
										// Set response
										const string response = Json(Json::Object{
											{"Interaction", make_unique<Json>(interactionIndex)},
											{"Status", make_unique<Json>("Failed")}
										}).encode();
									
										// Get response message
										const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
										
										// Check if sending response message to client failed
										if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
										
											// Check if getting connection's buffer input was successful
											evbuffer *input = bufferevent_get_input(connectionsBuffer);
											if(input) {
											
												// Remove data from input
												evbuffer_drain(input, evbuffer_get_length(input));
											}
											
											// Remove connection's buffer callbacks
											bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
											
											// Close connection
											evhttp_connection_free(connection);
											
											// Cancel all client's interactions
											clients->at(connection).cancelAllInteractions();
											
											// Remove connection from list of clients
											clients->erase(connection);
										}
									}
									
									// Otherwise
									else {
								
										// Get request's buffer
										bufferevent *requestsBuffer = evhttp_connection_get_bufferevent(evhttp_request_get_connection(request));
										
										// Set request's buffer callbacks
										bufferevent_setcb(requestsBuffer, nullptr, nullptr, ([](bufferevent *requestsBuffer, short event, void *argument) {
										
											// Get request's buffer callbacks argument from argument
											unique_ptr<tuple<unordered_map<evhttp_connection *, Client> *, evhttp_connection *, const Json::Number>> requestsBufferCallbacksArgument(reinterpret_cast<tuple<unordered_map<evhttp_connection *, Client> *, evhttp_connection *, const Json::Number> *>(argument));
											
											// Get clients from request's buffer callbacks argument
											unordered_map<evhttp_connection *, Client> *clients = get<0>(*requestsBufferCallbacksArgument);
											
											// Get connection from request's buffer callbacks argument
											evhttp_connection *connection = get<1>(*requestsBufferCallbacksArgument);
											
											// Get interaction index from request's buffer callbacks argument
											const Json::Number interactionIndex = get<2>(*requestsBufferCallbacksArgument);
										
											// Remove request's buffer callbacks
											bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
											
											// Check if connection still exists
											if(clients->count(connection)) {
											
												// Check if interaction still exists
												if(clients->at(connection).getInteraction(interactionIndex)) {
											
													// Remove interaction from client
													clients->at(connection).removeInteraction(interactionIndex);
												
													// Check if getting connection's buffer failed
													bufferevent *connectionsBuffer = evhttp_connection_get_bufferevent(connection);
													if(!connectionsBuffer) {
													
														// Close connection
														evhttp_connection_free(connection);
														
														// Cancel all client's interactions
														clients->at(connection).cancelAllInteractions();
														
														// Remove connection from list of clients
														clients->erase(connection);
													}
													
													// Otherwise
													else {
													
														// Set response
														const string response = Json(Json::Object{
															{"Interaction", make_unique<Json>(interactionIndex)},
															{"Status", make_unique<Json>("Failed")}
														}).encode();
													
														// Get response message
														const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
														
														// Check if sending response message to client failed
														if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
														
															// Check if getting connection's buffer input was successful
															evbuffer *input = bufferevent_get_input(connectionsBuffer);
															if(input) {
															
																// Remove data from input
																evbuffer_drain(input, evbuffer_get_length(input));
															}
															
															// Remove connection's buffer callbacks
															bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
															
															// Close connection
															evhttp_connection_free(connection);
															
															// Cancel all client's interactions
															clients->at(connection).cancelAllInteractions();
															
															// Remove connection from list of clients
															clients->erase(connection);
														}
													}
												}
											}
											
										}), requestsBufferCallbacksArgument.get());
										
										// Check if enabling reading on the request's buffer failed
										if(bufferevent_enable(requestsBuffer, EV_READ)) {
										
											// Remove request's buffer callbacks
											bufferevent_setcb(requestsBuffer, nullptr, nullptr, nullptr, nullptr);
										
											// Reply with internal server error to request
											evhttp_send_reply(request, HTTP_INTERNAL, nullptr, nullptr);
											
											// Remove interaction from client
											clients->at(connection).removeInteraction(interactionIndex);
											
											// Set response
											const string response = Json(Json::Object{
												{"Interaction", make_unique<Json>(interactionIndex)},
												{"Status", make_unique<Json>("Failed")}
											}).encode();
										
											// Get response message
											const vector<uint8_t> responseMessage = createWebSocketResponse(response, WebSocketOpcode::TEXT);
											
											// Check if sending response message to client failed
											if(bufferevent_write(connectionsBuffer, responseMessage.data(), responseMessage.size())) {
											
												// Check if getting connection's buffer input was successful
												evbuffer *input = bufferevent_get_input(connectionsBuffer);
												if(input) {
												
													// Remove data from input
													evbuffer_drain(input, evbuffer_get_length(input));
												}
												
												// Remove connection's buffer callbacks
												bufferevent_setcb(connectionsBuffer, nullptr, nullptr, nullptr, nullptr);
												
												// Close connection
												evhttp_connection_free(connection);
												
												// Cancel all client's interactions
												clients->at(connection).cancelAllInteractions();
												
												// Remove connection from list of clients
												clients->erase(connection);
											}
										}
										
										// Otherwise
										else {
										
											// Release request's buffer callbacks argument
											requestsBufferCallbacksArgument.release();
										}
									}
								}
							}
						}
					}
				}
			}
		}
	
	}), &torServerRequestCallbackArgument);
	
	// Check if creating Tor configuration failed
	shared_ptr<tor_main_configuration_t> torConfiguration(tor_main_configuration_new(), tor_main_configuration_free);
	if(!torConfiguration) {
	
		// Display message
		cout << "Creating Tor configuration failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Check if getting Tor control socket failed
	tor_control_socket_t torControlSocket = tor_main_configuration_setup_control_socket(torConfiguration.get());
	if(torControlSocket == INVALID_TOR_CONTROL_SOCKET) {
	
		// Display message
		cout << "Getting Tor control socket failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Check if creating Tor connection from Tor control socket failed
	unique_ptr<bufferevent, decltype(&bufferevent_free)> torConnection(bufferevent_socket_new(eventBase.get(), torControlSocket, BEV_OPT_DEFER_CALLBACKS | BEV_OPT_THREADSAFE), bufferevent_free);
	if(!torConnection) {
	
		// Display message
		cout << "Creating Tor connection from Tor control socket failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Initialize Tor connected
	bool torConnected = false;
	
	// Initialize Tor connection callbacks argument
	tuple<const string *, const uint16_t *, evhttp *, string *, evhttp *, bool *> torConnectionCallbacksArgument(&listenAddress, &listenPort, httpServer.get(), &torHiddenServiceAddress, torServer.get(), &torConnected);
	
	// Set Tor connection callbacks
	bufferevent_setcb(torConnection.get(), ([](bufferevent *torConnection, void *argument) {
	
		// Get Tor connection callbacks argument from argument
		tuple<const string *, const uint16_t *, evhttp *, string *, evhttp *, bool *> *torConnectionCallbacksArgument = reinterpret_cast<tuple<const string *, const uint16_t *, evhttp *, string *, evhttp *, bool *> *>(argument);
		
		// Get listen address from Tor connection callbacks arguments
		const string *listenAddress = get<0>(*torConnectionCallbacksArgument);
		
		// Get listen port from Tor connection callbacks arguments
		const uint16_t *listenPort = get<1>(*torConnectionCallbacksArgument);
		
		// Get HTTP server from Tor connection callbacks arguments
		evhttp *httpServer = get<2>(*torConnectionCallbacksArgument);
		
		// Get Tor hidden service address from Tor connection callbacks argument
		string *torHiddenServiceAddress = get<3>(*torConnectionCallbacksArgument);
		
		// Get Tor server from Tor connection callbacks arguments
		evhttp *torServer = get<4>(*torConnectionCallbacksArgument);
	
		// Get Tor connected from Tor connection callbacks argument
		bool *torConnected = get<5>(*torConnectionCallbacksArgument);
		
		// Check if getting input from the Tor connection failed
		evbuffer *input = bufferevent_get_input(torConnection);
		if(!input) {
		
			// Display message
			cout << "Getting input from the Tor connection failed" << endl;
			
			// Remove Tor connection callbacks
			bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
		
			// Exit failure
			quick_exit(EXIT_FAILURE);
		}
		
		// Otherwise
		else {
		
			// Get input's length
			const size_t length = evbuffer_get_length(input);
			
			// Check if getting data from input failed
			uint8_t data[length];
			if(evbuffer_copyout(input, data, length) == -1) {
			
				// Display message
				cout << "Getting data from input failed" << endl;
				
				// Remove data from input
				evbuffer_drain(input, length);
				
				// Remove Tor connection callbacks
				bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
			
				// Exit failure
				quick_exit(EXIT_FAILURE);
			}
			
			// Otherwise check if removing data from input failed
			else if(evbuffer_drain(input, length)) {
			
				// Display message
				cout << "Removing data from input failed" << endl;
				
				// Remove Tor connection callbacks
				bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
			
				// Exit failure
				quick_exit(EXIT_FAILURE);
			}
			
			// Otherwise
			else {
			
				// Check if Tor isn't connected
				if(!*torConnected) {
					
					// Check if Tor is connected
					const char connectedMessage[] = "250-status/circuit-established=1";
					
					if(length >= sizeof(connectedMessage) - sizeof('\0') && !memcmp(data, connectedMessage, sizeof(connectedMessage) - sizeof('\0'))) {
					
						// Set Tor connected
						*torConnected = true;
						
						// Display message
						cout << "Connected to the Tor network" << endl;
						
						// Check if binding Tor server to random port failed
						evhttp_bound_socket *torServerSocket = evhttp_bind_socket_with_handle(torServer, "localhost", 0);
						if(!torServerSocket) {
						
							// Display message
							cout << "Binding Tor server to random port failed" << endl;
							
							// Remove Tor connection callbacks
							bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
						
							// Exit failure
							quick_exit(EXIT_FAILURE);
						}
						
						// Otherwise
						else {
						
							// Check if getting Tor server socket details failed
							sockaddr_in socketDetails;
							socklen_t length = sizeof(socketDetails);
							if(getsockname(evhttp_bound_socket_get_fd(torServerSocket), reinterpret_cast<sockaddr *>(&socketDetails), &length)) {
							
								// Display message
								cout << "Getting Tor server socket details failed" << endl;
								
								// Remove Tor connection callbacks
								bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
							
								// Exit failure
								quick_exit(EXIT_FAILURE);
							}
							
							// Otherwise
							else {
							
								// Initialize create Tor hidden service command
								const string createTorHiddenServiceCommand = "ADD_ONION NEW:BEST Flags=DiscardPK Port=80," + to_string(ntohs(socketDetails.sin_port)) + "\n";
								
								// Check if creating Tor hidden service with the Tor connection failed
								if(bufferevent_write(torConnection, createTorHiddenServiceCommand.c_str(), createTorHiddenServiceCommand.length())) {
								
									// Display message
									cout << "Creating Tor hidden service with the Tor connection failed" << endl;
									
									// Remove Tor connection callbacks
									bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
								
									// Exit failure
									quick_exit(EXIT_FAILURE);
								}
							}
						}
					}
					
					// Otherwise
					else {
					
						// Check if creating timer event failed
						unique_ptr<event> timerEvent = make_unique<event>();
						if(!timerEvent) {
						
							// Display message
							cout << "Creating timer event failed" << endl;
							
							// Remove Tor connection callbacks
							bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
						
							// Exit failure
							quick_exit(EXIT_FAILURE);
						}
						
						// Otherwise
						else {
						
							// Check if creating timer callback argument failed
							unique_ptr<tuple<bufferevent *, event *>> timerCallbackArgument = make_unique<tuple<bufferevent *, event *>>(torConnection, timerEvent.get());
							if(!timerCallbackArgument) {
							
								// Display message
								cout << "Creating timer callback argument failed" << endl;
								
								// Remove Tor connection callbacks
								bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
							
								// Exit failure
								quick_exit(EXIT_FAILURE);
							}
							
							// Otherwise
							else {
							
								// Check if setting timer callback failed
								if(evtimer_assign(timerEvent.get(), bufferevent_get_base(torConnection), ([](evutil_socket_t signal, short events, void *argument) {
								
									// Get timer callback argument from argument
									unique_ptr<tuple<bufferevent *, event *>> timerCallbackArgument(reinterpret_cast<tuple<bufferevent *, event *> *>(argument));
									
									// Get Tor connection from timer callback argument
									bufferevent *torConnection = get<0>(*timerCallbackArgument);
									
									// Get timer event from timer callback argument
									unique_ptr<event> timerEvent(get<1>(*timerCallbackArgument));
									
									// Check if getting status from the Tor connection failed
									if(bufferevent_write(torConnection, "getinfo status/circuit-established\n", sizeof("getinfo status/circuit-established\n") - sizeof('\0'))) {
									
										// Display message
										cout << "Getting status from the Tor connection failed" << endl;
									
										// Remove Tor connection callbacks
										bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
									
										// Exit failure
										quick_exit(EXIT_FAILURE);
									}
								
								}), timerCallbackArgument.get())) {
								
									// Display message
									cout << "Setting timer callback failed" << endl;
									
									// Remove Tor connection callbacks
									bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
								
									// Exit failure
									quick_exit(EXIT_FAILURE);
								}
								
								// Otherwise
								else {
								
									// Set timer
									const timeval timer = {
									
										// Microseconds
										.tv_usec = CHECK_TOR_CONNECTED_INTERVAL_MICROSECONDS
									};
									
									// Check if adding timer event to the dispatched events failed
									if(evtimer_add(timerEvent.get(), &timer)) {
									
										// Display message
										cout << "Adding timer event to the dispatched events failed" << endl;
										
										// Remove Tor connection callbacks
										bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
									
										// Exit failure
										quick_exit(EXIT_FAILURE);
									}
									
									// Otherwise
									else {
									
										// Release timer event
										timerEvent.release();
										
										// Release timer callback argument
										timerCallbackArgument.release();
									}
								}
							}
						}
					}
				}
				
				// Otherwise
				else {
				
					// Check if got Tor hidden service information
					const char torHiddenServiceInformationMessage[] = "250-ServiceID=";
					
					if(length >= sizeof(torHiddenServiceInformationMessage) - sizeof('\0') && !memcmp(data, torHiddenServiceInformationMessage, sizeof(torHiddenServiceInformationMessage) - sizeof('\0'))) {
					
						// Check if getting Tor hidden service address delimiter failed
						const uint8_t *addressDelimiter = reinterpret_cast<uint8_t *>(memchr(&data[sizeof(torHiddenServiceInformationMessage) - sizeof('\0')], '\r', length - (sizeof(torHiddenServiceInformationMessage) - sizeof('\0'))));
						if(!addressDelimiter) {
						
							// Display message
							cout << "Getting Tor hidden service address delimiter failed" << endl;
							
							// Remove Tor connection callbacks
							bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
						
							// Exit failure
							quick_exit(EXIT_FAILURE);
						}
						
						// Otherwise
						else {
						
							// Get Tor hidden service address
							string address(reinterpret_cast<char *>(&data[sizeof(torHiddenServiceInformationMessage) - sizeof('\0')]), addressDelimiter - &data[sizeof(torHiddenServiceInformationMessage) - sizeof('\0')]);
							
							// Check if Tor hidden service address is invalid
							if(address.empty()) {
							
								// Display message
								cout << "Tor hidden service address is invalid" << endl;
								
								// Remove Tor connection callbacks
								bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
							
								// Exit failure
								quick_exit(EXIT_FAILURE);
							}
							
							// Otherwise
							else {
											
								// Check if binding HTTP server to listen address and listen port failed
								if(evhttp_bind_socket(httpServer, listenAddress->c_str(), *listenPort)) {
								
									// Display message
									cout << "Binding HTTP server to " << *listenAddress << ':' << to_string(*listenPort) << " failed" << endl;
									
									// Remove Tor connection callbacks
									bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
								
									// Exit failure
									quick_exit(EXIT_FAILURE);
								}
								
								// Otherwise
								else {
								
									// Set Tor hidden service address to address
									*torHiddenServiceAddress = address;
									
									// Check if listen address is an IPv6 address
									char temp[sizeof(in6_addr)];
									if(inet_pton(AF_INET6, listenAddress->c_str(), temp) == 1) {
									
										// Display message
										cout << "Listening at ws://[" << *listenAddress << ']' << ((*listenPort != HTTP_PORT) ? ':' + to_string(*listenPort) : "") << endl;
									}
									
									// Otherwise
									else {
									
										// Display message
										cout << "Listening at ws://" << *listenAddress << ((*listenPort != HTTP_PORT) ? ':' + to_string(*listenPort) : "") << endl;
									}
								}
							}
						}
					}
					
					// Otherwise
					else {
					
						// Display message
						cout << "Getting Tor hidden service information failed" << endl;
						
						// Remove Tor connection callbacks
						bufferevent_setcb(torConnection, nullptr, nullptr, nullptr, nullptr);
					
						// Exit failure
						quick_exit(EXIT_FAILURE);
					}
				}
			}
		}
		
	}), nullptr, nullptr, &torConnectionCallbacksArgument);
	
	// Check if enabling reading with the Tor buffer failed
	if(bufferevent_enable(torConnection.get(), EV_READ)) {
	
		// Display message
		cout << "Enabling reading with the Tor buffer failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Display message
	cout << "Connecting to the Tor network" << endl;
	
	// Check if sending authentication message to Tor connection failed
	if(bufferevent_write(torConnection.get(), "authenticate \"\"\n", sizeof("authenticate \"\"\n") - sizeof('\0'))) {
	
		// Display message
		cout << "sending authentication message to Tor connection failed" << endl;
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Initialize generator
	random_device device;
	mt19937 generator(device());
	
	// Initialize distribution
	uniform_int_distribution<uint8_t> distribution(0, UINT8_MAX);
	
	// Loop until a unique temporary directory is created
	filesystem::path temporaryDirectory;
	while(true) {
	
		// Initialize random string
		stringstream randomString;
		
		// Go through all bytes in random string length
		for(size_t i = 0; i < TEMPORARY_DIRECTORY_LENGTH; ++i) {
		
			// Fill byte in random string
			randomString << hex << static_cast<uint16_t>(distribution(generator));
		}	
		
		// Set temporary directory
		temporaryDirectory = filesystem::temp_directory_path() / randomString.str();
		
		// Check if creating temporary directory was successful
		if(filesystem::create_directory(temporaryDirectory)) {
		
			// Break
			break;
		}
	}
	
	// Get temporary directory as a string
	const string &temporaryDirectoryString = temporaryDirectory.string();
	
	// Set Tor arguments
	const char *torArguments[] = {
	
		// Program name
		argv[0],
		
		// Quiet
		"--quiet",
		
		// Disable SOCKS port
		"--SocksPort", "0",
		
		// Disable Geo IPv4
		"--GeoIPFile", "",
		
		// Disable Geo IPv6
		"--GeoIPv6File", "",
		
		// Disable configuration file
		"--torrc-file", "",
		
		// Ignore missing configuration file
		"--ignore-missing-torrc",
		
		// Data directory
		"--DataDirectory", temporaryDirectoryString.c_str(),
		
		// End
		nullptr
	};
	
	// Check if configuring Tor configuration with the Tor arguments failed
	if(tor_main_configuration_set_command_line(torConfiguration.get(), sizeof(torArguments) / sizeof(torArguments[0]) - 1, const_cast<char **>(torArguments))) {
	
		// Display message
		cout << "Configuring Tor configuration with the Tor arguments failed" << endl;
		
		// Remove temporary directory
		filesystem::remove_all(temporaryDirectory);
	
		// Return failure
		return EXIT_FAILURE;
	}
	
	// Initialize thread error
	atomic_bool threadError(false);
	
	// Create Tor thread
	thread torThread(([&eventBase, &torConfiguration, &temporaryDirectory, &threadError]() {
	
		// Check if Windows
		#ifdef _WIN32
		
			// Check if running Tor failed
			if(tor_run_main(torConfiguration.get()) != EXIT_SUCCESS) {
			
				// Display message
				cout << "Running Tor failed" << endl;
			
				// Set thread error
				threadError.store(true);
			}
		
		// Otherwise
		#else
	
			// Check if allowing all signals was successful
			sigset_t signalMask;
			if(!sigemptyset(&signalMask) && !pthread_sigmask(SIG_SETMASK, &signalMask, nullptr)) {
				
				// Check if running Tor failed
				if(tor_run_main(torConfiguration.get()) != EXIT_SUCCESS) {
				
					// Display message
					cout << "Running Tor failed" << endl;
					
					// Set thread error
					threadError.store(true);
				}
			}
			
			// Otherwise
			else {
			
				// Display message
				cout << "Allowing all signals failed" << endl;
			
				// Set thread error
				threadError.store(true);
			}
		#endif
		
		// Check if breaking out of event dispatch loop failed
		if(event_base_loopbreak(eventBase.get())) {
		
			// Display message
			cout << "Breaking out of event dispatch loop failed" << endl;
			
			// Remove temporary directory
			filesystem::remove_all(temporaryDirectory);
		
			// Exit failure
			quick_exit(EXIT_FAILURE);
		}
	}));
	
	// Check if running event dispatch loop failed
	if(event_base_dispatch(eventBase.get()) == -1) {
	
		// Display message
		cout << "Running event dispatch loop failed" << endl;
		
		// Remove temporary directory
		filesystem::remove_all(temporaryDirectory);
	
		// Exit failure
		quick_exit(EXIT_FAILURE);
	}
	
	// Otherwise
	else {
	
		// Initialize error occured
		bool errorOccured = false;
	
		// Check if Tor thread is joinable
		if(torThread.joinable()) {
		
			// Try
			try {
		
				// Join Tor thread
				torThread.join();
			}
			
			// Catch errors
			catch(...) {
			
				// Display message
				cout << "Joining Tor thread failed" << endl;
			
				// Set error occured
				errorOccured = true;
				
				// Remove temporary directory
				filesystem::remove_all(temporaryDirectory);
			
				// Exit failure
				quick_exit(EXIT_FAILURE);
			}
		}
		
		// Check if an error didn't occur
		if(!errorOccured) {
		
			// Check if a thread error occured
			if(threadError.load()) {
			
				// Remove temporary directory
				filesystem::remove_all(temporaryDirectory);
			
				// Return failure
				return EXIT_FAILURE;
			}
			
			// Check if removing temporary directory failed
			if(!filesystem::remove_all(temporaryDirectory)) {
			
				// Return failure
				return EXIT_FAILURE;
			}
			
			// Return success
			return EXIT_SUCCESS;
		}
	}
	
	// Return failure
	return EXIT_FAILURE;
}


// Supporting function implementation

// Create WebSocket response
const vector<uint8_t> createWebSocketResponse(const string &message, WebSocketOpcode opcode) {

	// Initialize response
	vector<uint8_t> response;
	
	// Go through all WebSocket response frames
	for(string::size_type i = 0;;) {
	
		// Get if response is final frame
		const bool isFinalFrame = message.length() - i <= INT64_MAX;
		
		// Get frame length
		const string::size_type frameLength = isFinalFrame ? message.length() - i : INT64_MAX;
		
		// Append opcode and is final frame to the response
		response.push_back(static_cast<uint8_t>(opcode) | (isFinalFrame ? WEBSOCKET_FINAL_FRAME_BYTE_MASK : 0));
		
		// Check if frame's length requires sixty-three to express
		if(frameLength > UINT16_MAX) {
		
			// Append length to the response
			response.push_back(WEBSOCKET_SIXTY_THREE_BITS_LENGTH);
			
			// Go through all length bytes
			for(size_t j = 0; j < sizeof(uint64_t); ++j) {
			
				// Append length byte to the response
				response.push_back(frameLength >> (Common::BITS_IN_A_BYTE * (sizeof(uint64_t) - sizeof(uint8_t) - j)));
			}
		}
		
		// Otherwise check if frame's length requires sixteen bits to express
		else if(frameLength >= WEBSOCKET_SIXTEEN_BITS_LENGTH) {
		
			// Append length to the response
			response.push_back(WEBSOCKET_SIXTEEN_BITS_LENGTH);
			
			// Go through all length bytes
			for(size_t j = 0; j < sizeof(uint16_t); ++j) {
			
				// Append length byte to the response
				response.push_back(frameLength >> (Common::BITS_IN_A_BYTE * (sizeof(uint16_t) - sizeof(uint8_t) - j)));
			}
		}
		
		// Otherwise
		else {
		
			// Append length to the response
			response.push_back(frameLength);
		}
		
		// Append message to the response
		response.insert(response.end(), message.begin() + i, message.begin() + i + frameLength);
		
		// Check if is final frame
		if(isFinalFrame) {
		
			// Break
			break;
		}
		
		// Update offset to next frame
		i += frameLength;
		
		// Set opcode to continuation
		opcode = WebSocketOpcode::CONTINUATION;
	}
	
	// Return response
	return response;
}

// Get cookies
const unordered_map<string, string> getCookies(const string &cookieHttpHeader) {

	// Initialize cookies
	unordered_map<string, string> cookies;

	// Go through all cookies
	for(string::size_type startOfCookie = 0, endOfCookie = cookieHttpHeader.find(COOKIE_SEPARATOR, startOfCookie);; startOfCookie = endOfCookie + sizeof(COOKIE_SEPARATOR), endOfCookie = cookieHttpHeader.find(COOKIE_SEPARATOR, startOfCookie)) {
	
		// Get cookie
		const string cookie = cookieHttpHeader.substr(startOfCookie, (endOfCookie != string::npos) ? endOfCookie - startOfCookie : string::npos);
	
		// Check if cookie isn't separated into a key value pair
		const string::size_type separator = cookie.find(COOKIE_KEY_VALUE_SEPARATOR);
		if(separator == string::npos) {
		
			// Throw exception
			throw runtime_error("Parsing cookies failed");
		}
		
		// Check if key value pair is invalid
		string key = Common::trim(cookie.substr(0, separator));
		string value = Common::trim(cookie.substr(separator + sizeof(COOKIE_KEY_VALUE_SEPARATOR)));
		
		if(key.empty() || value.empty()) {
		
			// Throw exception
			throw runtime_error("Parsing cookies failed");
		}
		
		// Append key value pair to cookies
		cookies[key] = value;
		
		// Check if at the last cookie
		if(endOfCookie == string::npos) {
		
			// Break
			break;
		}
	}
	
	// Return cookies
	return cookies;
}

// Get random session ID
const string getRandomSessionId() {

	// Initialize generator
	random_device device;
	mt19937 generator(device());
	
	// Initialize distribution
	uniform_int_distribution<uint8_t> distribution(0, UINT8_MAX);

	// Go through all bytes in the session ID length
	size_t sessionIdLength;
	for(size_t i = 0; i < sizeof(sessionIdLength); ++i) {
	
		// Set byte to random value
		reinterpret_cast<uint8_t *>(&sessionIdLength)[i] = distribution(generator);
	}
	
	// Limit session ID length
	sessionIdLength = sessionIdLength % (SESSION_ID_MAXIMUM_LENGTH - SESSION_ID_MINIMUM_LENGTH + 1) + SESSION_ID_MINIMUM_LENGTH;
	
	// Initialize session ID
	string sessionId;
	
	// Go through all characters in the session ID
	for(size_t i = 0; i < sessionIdLength; ++i) {
	
		// Append random character to session ID
		sessionId.push_back(SESSION_ID_CHARACTERS[distribution(generator) % (sizeof(SESSION_ID_CHARACTERS) - sizeof('\0'))]);
	}
	
	// Return session ID
	return sessionId;
}

// Get random URL
const string getRandomUrl(const string &torHiddenServiceAddress) {

	// Initialize generator
	random_device device;
	mt19937 generator(device());
	
	// Initialize distribution
	uniform_int_distribution<uint8_t> distribution(0, UINT8_MAX);

	// Go through all bytes in the URL length
	size_t urlLength;
	for(size_t i = 0; i < sizeof(urlLength); ++i) {
	
		// Set byte to random value
		reinterpret_cast<uint8_t *>(&urlLength)[i] = distribution(generator);
	}
	
	// Limit URL length
	urlLength = urlLength % (URL_MAXIMUM_LENGTH - URL_MINIMUM_LENGTH + 1) + URL_MINIMUM_LENGTH;
	
	// Initialize URL
	string url = "http://" + torHiddenServiceAddress + ".onion/";
	
	// Go through all characters in the URL
	for(size_t i = 0; i < urlLength; ++i) {
	
		// Append random character to URL
		url.push_back(URL_CHARACTERS[distribution(generator) % (sizeof(URL_CHARACTERS) - sizeof('\0'))]);
	}
	
	// Return URL
	return url;
}