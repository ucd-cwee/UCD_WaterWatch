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

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define CURL_STATICLIB

//#ifndef WIN32
#include "CURL_STATIC_LIB.h"

#include "curl/curl.h"
#pragma comment(lib,"Iphlpapi.lib")

#ifdef includeZeroMQ
#include "zeroMQ/zmq.h"
#include "zeroMQ/zmq_addon.hpp"
#endif

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Crypt32.lib")
#pragma comment(lib,"Wldap32.lib")
#pragma comment(lib,"Normaliz.lib")

#include <stdio.h>
#include <Windows.h>
#include <Iphlpapi.h>
#include <Assert.h>
#pragma comment(lib, "iphlpapi.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

CURL* GetCurl(cweeSharedPtr<void> srce) {
	return static_cast<CURL*>(srce.Get());
};
CURLoption GetCurlOp(cweeCURL::CURLoption op) {
	return static_cast<CURLoption>(static_cast<int>(op)+10000);
};

cweeCURL::cweeCURL() {
	curl = cweeSharedPtr<void>(cweeSharedPtr<CURL>(curl_easy_init(), [](CURL* p) { curl_easy_cleanup(p); }), [](void* p) { return p; });
};
cweeCURL::~cweeCURL() {
	curl = nullptr;
};

int cweeCURL::setopt(cweeCURL::CURLoption op, long val) {
	CURLcode result = curl_easy_setopt(GetCurl(curl), GetCurlOp(op), val);
	return static_cast<int>(result);
};
int cweeCURL::setopt_CUSTOMREQUEST(const char* val) {
	CURLcode result = curl_easy_setopt(GetCurl(curl), ::CURLOPT_CUSTOMREQUEST, val);
	return static_cast<int>(result);
};
int cweeCURL::setopt(cweeCURL::CURLoption op, const char* val) {
	CURLcode result = curl_easy_setopt(GetCurl(curl), GetCurlOp(op), val);
	return static_cast<int>(result);
};
int cweeCURL::setopt(cweeCURL::CURLoption op, cweeList<cweeStr> const& val) {
	struct curl_slist* headers = NULL;
	for (auto& x : val) headers = curl_slist_append(headers, x.c_str());
	CURLcode result = curl_easy_setopt(GetCurl(curl), GetCurlOp(op), headers);
	return static_cast<int>(result);
};
int cweeCURL::setopt(cweeCURL::CURLoption op, size_t(*func)(void*, size_t, size_t, FILE*)) {
	CURLcode result = curl_easy_setopt(GetCurl(curl), GetCurlOp(op), func);
	return static_cast<int>(result);
};
int cweeCURL::setopt(cweeCURL::CURLoption op, FILE* fp) {
	CURLcode result = curl_easy_setopt(GetCurl(curl), GetCurlOp(op), fp);
	return static_cast<int>(result);
};
int cweeCURL::perform() {
	CURLcode result = curl_easy_perform(GetCurl(curl));
	return static_cast<int>(result);
};

#define useCurveSecurity

#ifdef includeZeroMQ
class cweeSocket {
public:
	cweeStr			Name;
	cweeStr			address;
	int				port = 0;
	zmq::socket_t	socket;
	bool			awaitingReply = false;
};

class cweeRouter {
public:
	~cweeRouter() {
		proxyThread.detach();
	};

	cweeStr			Name;
	int				frontend_port = 0;
	cweeStr			backend_name = "";
	zmq::socket_t	frontend_socket;
	zmq::socket_t	backend_socket;
	zmq::socket_t	backend_worker_socket;

	cweeStr			frontend_socket_secretKey;
	cweeStr			backend_socket_secretKey;
	cweeStr			backend_worker_socket_secretKey;

	void		StartRouterProxy() {
		zmq::proxy(static_cast<void*>(frontend_socket),
			static_cast<void*>(backend_socket),
			nullptr);
	};

	DispatchTimer	periodicUpdater;

	std::thread		proxyThread;
};

void	DoRouterRequest(cweeRouterRequest* io);

class ZeroMQImpl final : public ZeroMQ {
public:
	ZeroMQImpl() : ZeroMQ() {};
	virtual ~ZeroMQImpl() {};
private:
	zmq::context_t zeroMQ_context = zmq::context_t(1);
	cweeUnorderedList< cweeSocket > publishingSockets;
	cweeUnorderedList< cweeSocket > subscribingSockets;

	cweeUnorderedList< cweeRouter > routerSockets;
	cweeUnorderedList< cweeSocket > clientSockets;

	cweeStr privateServerKey = "yevUcBl^KyeCO*WUqF6}n:Ci/RzcmqLiT^#>IZ!f"; // public should be == jow0MY5igjre>]VY*P=5COS0$1Ztk]3<R<1MgrPT

	cweeStr		getUniqueFingerprint() {
		cweeStr MAC_ADDRESS;
		cweeStr IP_ADDRESS;
		{
			PIP_ADAPTER_INFO AdapterInfo;
			DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

			AdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
			if (AdapterInfo != NULL) {
				// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
				if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
					free(AdapterInfo);
					AdapterInfo = (IP_ADAPTER_INFO*)malloc(dwBufLen);
				}
				if (AdapterInfo != NULL) {
					if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR) {
						// Contains pointer to current adapter info
						PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
						do {
							MAC_ADDRESS = "";
							for (int i = 0; i < pAdapterInfo->AddressLength; i++) {
								MAC_ADDRESS.AddToDelimiter(cweeStr::printf("%02X", pAdapterInfo->Address[i]), ":");
							}
							IP_ADDRESS = pAdapterInfo->IpAddressList.IpAddress.String;

							break;

							pAdapterInfo = pAdapterInfo->Next;
						} while (pAdapterInfo);
					}
					free(AdapterInfo);
				}
			}
		}

		// cweeStr randomID = cweeStr::printf("%04X-%04X", cweeRandomInt(0x10000), cweeRandomInt(0x10000)); // ideally we don't want this... 

		return MAC_ADDRESS + "-" + IP_ADDRESS;// +"-" + randomID;
	};

	cweeUnorderedList< cweeSharedPtr<cweeRouterRequest> > routerRequests;

	void		Pack_Message(cweeStr& toPackage) {
		toPackage.Append(GetServerMessageSuffix());
	};
	void		Unpack_Message(cweeStr& toUnpackage) {
		if (toUnpackage.Length() >= 6) {
			cweeParser a(toUnpackage, GetServerMessageSuffix(), true);
			if (a.getNumVars() >= 2) {
				toUnpackage = a[0];
			}
		}
		else {
			// could not possibly have a "correct" message, therefore we just report it as-is
		}
	};
	cweeStr     GetServerMessageSuffix() { return "</WW0>"; };
	zmq::message_t cweeStr_to_zMQ(const cweeStr& message) {
		cweeStr msg = message.c_str();
		Pack_Message(msg);
		zmq::message_t Message(msg.c_str(), msg.Size());
		return Message;
	};
	cweeStr		zMQ_to_cweeStr(const zmq::message_t& message) {
		cweeStr out = static_cast<const char*>(message.data());
		Unpack_Message(out);
		return out;
	};
	cweeStr		receiveAllMessageParts(zmq::socket_t& socket) {
		cweeStr out;

		while (1) {
			//  Process all parts of the message
			zmq::message_t message;
			auto result = socket.recv(message, zmq::recv_flags::dontwait);
			if (result.has_value()) {
				out.AddToDelimiter(zMQ_to_cweeStr(message), "\n");
			}
			else {
				break;
			}

			int more = 0;           //  Multipart detection
			size_t more_size = sizeof(more);
			socket.getsockopt(ZMQ_RCVMORE, &more, &more_size);
			if (!more) break;              //  Last message part
		}

		return out;
	};

public:
	bool		StartPublishingServer(int Port) override {
		try {
			int tryFind = publishingSockets.FindIndexWithName(cweeStr(Port));
			if (tryFind >= 0) {
				// already exists;
				return true;
			}
			else {
				publishingSockets.Lock();
				tryFind = publishingSockets.UnsafeAppend();
				auto ptr = publishingSockets.UnsafeRead(tryFind);
				if (ptr) {
					ptr->address = "";
					ptr->port = Port;
					ptr->Name = cweeStr(Port);
					ptr->socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::pub);
					ptr->socket.bind("tcp://*:" + cweeStr(Port)); // may fail if there is already another server. 
				}
				publishingSockets.Unlock();

				return true;
			}
		}
		catch (const std::exception& exc) {			
			return false;
		}
		return false;
	};
	bool		ClosePublishingServer(int Port) override {
		try {
			int tryFind = publishingSockets.FindIndexWithName(cweeStr(Port));
			if (tryFind >= 0) {
				// already exists;
				publishingSockets.Erase(tryFind);
				return true;
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	cweeList<int>	GetPublishingServerPorts() override {
		cweeThreadedList<int> out;
		try {
			for (auto& x : publishingSockets.GetList()) {
				publishingSockets.Lock();
				auto ptr = publishingSockets.UnsafeRead(x);
				if (ptr) {
					out.Append(ptr->port);
				}
				publishingSockets.Unlock();
			}
		}
		catch (const std::exception& exc) {
			return out;
		}
		return out;
	};
	bool		UpdatePublishingServer(const cweeStr& message, int Port) override {
		try {
			int tryFind = publishingSockets.FindIndexWithName(cweeStr(Port));
			if (tryFind >= 0) {
				// already exists;
				zmq::message_t Message = cweeStr_to_zMQ(message);

				publishingSockets.Lock();
				auto ptr = publishingSockets.UnsafeRead(tryFind);
				if (ptr) {
					ptr->socket.send(Message, zmq::send_flags::dontwait);
				}
				publishingSockets.Unlock();

				return true;
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	bool		StartSubscriptionClient(const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = subscribingSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				// already exists;
				return true;
			}
			else {
				subscribingSockets.Lock();
				tryFind = subscribingSockets.UnsafeAppend();
				auto ptr = subscribingSockets.UnsafeRead(tryFind);
				if (ptr) {
					ptr->address = address;
					ptr->port = Port;
					ptr->Name = name;
					ptr->socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::sub);
					ptr->socket.connect("tcp://" + name);
					ptr->socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
				}
				subscribingSockets.Unlock();

				return true;
			}

		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	bool		CloseSubscriptionClient(const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = subscribingSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				// already exists;
				subscribingSockets.Erase(tryFind);
				return true;
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	cweeList<cweeUnion<cweeStr, int>> GetSubscriptionClients() override {
		cweeThreadedList<cweeUnion<cweeStr, int>> out;
		try {
			for (auto& x : subscribingSockets.GetList()) {
				subscribingSockets.Lock();
				auto ptr = subscribingSockets.UnsafeRead(x);
				if (ptr) {
					out.Append(cweeUnion<cweeStr, int>(ptr->address.c_str(), ptr->port));
				}
				subscribingSockets.Unlock();
			}
		}
		catch (const std::exception& exc) {
			return out;
		}
		return out;
	};
	bool		TryGetSubscriptionUpdate(cweeStr& out, const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = subscribingSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				// already exists;
				zmq::recv_result_t result;
				zmq::message_t update;
				{
					subscribingSockets.Lock();
					auto ptr = subscribingSockets.UnsafeRead(tryFind);
					if (ptr) {
						result = ptr->socket.recv(update, zmq::recv_flags::dontwait);
					}
					subscribingSockets.Unlock();
				}

				if (result.has_value()) {
					out = zMQ_to_cweeStr(update);
					// data is available
					return true;
				}
				else {
					// no data available
					return false;
				}
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};

	bool		StartRouterServer(int Port) override {
		try {
			int tryFind = routerSockets.FindIndexWithName(cweeStr(Port));
			if (tryFind >= 0) {
				// already exists;
				return true;
			}
			else {
				routerSockets.Lock();
				tryFind = routerSockets.UnsafeAppend();
				auto ptr = routerSockets.UnsafeRead(tryFind);
				if (ptr) {
					ptr->frontend_port = Port;
					ptr->Name = cweeStr(Port);
					ptr->backend_name = cweeStr(Port) + "backend";

					ptr->frontend_socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::router);
					ptr->backend_socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::dealer);
					ptr->backend_worker_socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::dealer);

#ifdef useCurveSecurity

					char serverPublicKey[41], serverSecretKey[41];
					for (int i = 0; i < 41; i++) serverSecretKey[i] = privateServerKey[i];

					{
						if (zmq_curve_public(serverPublicKey, serverSecretKey) == 0) {
							ptr->frontend_socket.setsockopt(ZMQ_CURVE_SERVER, 1);
							ptr->frontend_socket.setsockopt(ZMQ_CURVE_PUBLICKEY, serverPublicKey, strlen(serverPublicKey));
							ptr->frontend_socket.setsockopt(ZMQ_CURVE_SECRETKEY, serverSecretKey, strlen(serverSecretKey));
							ptr->frontend_socket_secretKey = serverSecretKey;
						}
					}
#endif

					ptr->frontend_socket.bind("tcp://*:" + cweeStr(Port));
					ptr->backend_socket.bind("inproc://:" + ptr->backend_name);
					ptr->backend_worker_socket.connect("inproc://:" + ptr->backend_name);

					ptr->proxyThread = std::thread(std::bind(&cweeRouter::StartRouterProxy, ptr));

					ptr->periodicUpdater = DispatchTimer(1000, cweeJob([this](int port) {
						this->UpdateRouterServer(port);
					}, Port));
				}
				routerSockets.Unlock();

				return true;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	bool		CloseRouterServer(int Port) override {
		try {
			int tryFind = routerSockets.FindIndexWithName(cweeStr(Port));
			if (tryFind >= 0) {
				// already exists;				
				routerSockets.Erase(tryFind);
				return true;
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	cweeList<int>	GetRouterServerPorts() override {
		cweeThreadedList<int> out;
		try {
			for (auto& x : routerSockets.GetList()) {
				routerSockets.Lock();
				auto ptr = routerSockets.UnsafeRead(x);
				if (ptr) {
					out.Append(ptr->frontend_port);
				}
				routerSockets.Unlock();
			}
		}
		catch (const std::exception& exc) {
			return out;
		}
		return out;
	};
	bool		UpdateRouterServer(int Port)  {
		try {
			int tryFind = routerSockets.FindIndexWithName(cweeStr(Port));
			if (tryFind >= 0) {
				zmq::recv_result_t result;
				zmq::message_t id;
				cweeStr identity;
				zmq::message_t msg;
				cweeStr request;
				zmq::message_t copied_id;
				zmq::message_t outbound_msg;

				bool success = false;

				{
					routerSockets.Lock();
					auto ptr = routerSockets.UnsafeRead(tryFind);
					if (ptr) {
						result = ptr->backend_worker_socket.recv(id, zmq::recv_flags::dontwait);
						if (result.has_value()) {
							ptr->backend_worker_socket.recv(&msg);
							success = true;
						}
					}
					routerSockets.Unlock();
				}

				if (success) {
					request = zMQ_to_cweeStr(msg);

					routerRequests.Lock();
					int index = routerRequests.UnsafeAppend();
					auto reqPtr = routerRequests.UnsafeRead(index);
					if (reqPtr) {
						reqPtr->Lock(); {
							reqPtr->operator->()->request = request;
							{
								AUTO reqIDcopy = new zmq::message_t();
								reqIDcopy->copy(id);
								reqPtr->operator->()->requester_id = cweeSharedPtr<void>(cweeSharedPtr<zmq::message_t>(reqIDcopy), [](void* p) { return p; });
							}
							reqPtr->operator->()->request_index = index;
							reqPtr->operator->()->routerPort = cweeStr(Port);
							reqPtr->operator->()->routerRequests = &routerRequests;
						} reqPtr->Unlock();
					}
					routerRequests.Unlock();
					return true;
				}
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	cweeSharedPtr<cweeRouterRequest>		TryGetRouterServerRequest(int Port) override {
		cweeSharedPtr<cweeRouterRequest> data;

		routerRequests.Lock();
		AUTO list = routerRequests.UnsafeList();
		if (list.Num() > 0) {
			AUTO ptr = routerRequests.UnsafeRead(list[0]);
			if (ptr) {
				data = *ptr;
				routerRequests.Erase(list[0]);
			}
		}
		routerRequests.Unlock();
		
		return data;
	};
	bool		ReplyToRequestClient(cweeSharedPtr<cweeRouterRequest> data) override {
		bool success = false;
		if (data) {
			data->Lock(); {
				int tryFind = routerSockets.FindIndexWithName(data->routerPort);
				if (tryFind >= 0) {
					auto outbound_msg = cweeStr_to_zMQ(data->reply);
					routerSockets.Lock();
					auto ptr = routerSockets.UnsafeRead(tryFind);
					if (ptr) {
						ptr->backend_worker_socket.send(*(static_cast<zmq::message_t*>(data->requester_id.Get())), ZMQ_SNDMORE);
						ptr->backend_worker_socket.send(outbound_msg);
						success = true;
					}
					routerSockets.Unlock();
				}
			} data->Unlock();
		}
		return success;
	};

	bool		StartRequestClient(const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = clientSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				// already exists;
				return true;
			}
			else {
				clientSockets.Lock();
				tryFind = clientSockets.UnsafeAppend();
				auto ptr = clientSockets.UnsafeRead(tryFind);
				if (ptr) {
					ptr->address = address;
					ptr->port = Port;
					ptr->Name = name;
					ptr->socket = zmq::socket_t(zeroMQ_context, zmq::socket_type::dealer);

					cweeStr identity =
						// cweeStr::printf("%04X-%04X", cweeRandomInt(0x10000), cweeRandomInt(0x10000));
						getUniqueFingerprint();

					Pack_Message(identity);

					ptr->socket.setsockopt(ZMQ_IDENTITY, identity.c_str(), strlen(identity));


#ifdef useCurveSecurity

					char serverPublicKey[41], serverSecretKey[41];
					for (int i = 0; i < 41; i++) serverSecretKey[i] = privateServerKey[i];
					zmq_curve_public(serverPublicKey, serverSecretKey);

					char clientPublicKey[41], clientSecretKey[41];
					{
						if (zmq_curve_keypair(clientPublicKey, clientSecretKey) == 0) { // no error on 0 return
							ptr->socket.setsockopt(ZMQ_CURVE_SERVERKEY, serverPublicKey, strlen(serverPublicKey));
							ptr->socket.setsockopt(ZMQ_CURVE_PUBLICKEY, clientPublicKey, strlen(clientPublicKey));
							ptr->socket.setsockopt(ZMQ_CURVE_SECRETKEY, clientSecretKey, strlen(clientSecretKey));
						}
					}

#endif


					ptr->socket.connect("tcp://" + name);
				}
				clientSockets.Unlock();

				return true;
			}

		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	bool		CloseRequestClient(const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = clientSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				// already exists;
				clientSockets.Erase(tryFind);
				return true;
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	cweeList<cweeUnion<cweeStr, int>> GetRequestClients() override {
		cweeThreadedList<cweeUnion<cweeStr, int>> out;
		try {
			for (auto& x : clientSockets.GetList()) {
				clientSockets.Lock();
				auto ptr = clientSockets.UnsafeRead(x);
				if (ptr) {
					out.Append(cweeUnion<cweeStr, int>(ptr->address.c_str(), ptr->port));
				}
				clientSockets.Unlock();
			}
		}
		catch (const std::exception& exc) {
			return out;
		}
		return out;
	};
	bool		TrySendRequest(const cweeStr& request, const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = clientSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				auto req = cweeStr_to_zMQ(request);
				bool success = false;
				zmq::recv_result_t result;
				{
					clientSockets.Lock();
					auto ptr = clientSockets.UnsafeRead(tryFind);
					if (ptr && !(ptr->awaitingReply)) {
						result = ptr->socket.send(req, zmq::send_flags::dontwait);
						ptr->awaitingReply = true;
						success = true;
					}
					clientSockets.Unlock();
				}

				return success;

				//if (result.has_value()) {
				//	// request successfully sent
				//	return true;
				//}
				//else {
				//	// request failed to send - (i.e. previous request was already made)
				//	return false;
				//}
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	bool		TryGetReply(cweeStr& out, const cweeStr& address, int Port) override {
		try {
			cweeStr name = address + ":" + cweeStr(Port);
			int tryFind = clientSockets.FindIndexWithName(name);
			if (tryFind >= 0) {
				bool success = false;
				{
					clientSockets.Lock();
					auto ptr = clientSockets.UnsafeRead(tryFind);
					if (ptr && ptr->awaitingReply) {
						zmq::pollitem_t items[] = { { ptr->socket, 0, ZMQ_POLLIN, 0 } };

						zmq::poll(items, 1, 10); // 10 milliseconds?

						if (items[0].revents & ZMQ_POLLIN) {
							out = receiveAllMessageParts(ptr->socket);
							ptr->awaitingReply = false;
							success = true;
						}
					}
					clientSockets.Unlock();
				}

				if (success) {
					// data is available
					return true;
				}
				else {
					// no data available
					return false;
				}
			}
			else {
				// does not exist;
				return false;
			}
		}
		catch (const std::exception& exc) {
			return false;
		}
		return false;
	};
	cweeStr		GetReply(const cweeStr& request, const cweeStr& address, int Port) override {
		cweeStr out;

		if (TrySendRequest(request, address, Port)) {
			while (true) {
				if (TryGetReply(out, address, Port)) break;
			}
			// got 'out'
		}
		else {
			if (TryGetReply(out, address, Port)) return out;
			else out = "Request Could Not Be Made";
		}

		return out;
	};
};
cweeSharedPtr<ZeroMQ> Servers = cweeSharedPtr<ZeroMQImpl>(new ZeroMQImpl()).CastReference<ZeroMQ>();

#if 0
INLINE void	DoRouterRequest(cweeRouterRequest* io) {
	int											request_index = -1;
	cweeUnorderedList< cweeRouterRequest >* routerRequests = nullptr;

	if (io && io->routerRequests) {
		routerRequests = io->routerRequests;

		cweeStr request;
		io->Lock(); {
			io->started = true;
			request = io->request;
		} io->Unlock();

		auto job = scripting->Do(request);
		job.ContinueWith([=]() { // job, io, request_index, routerRequests
			cweeStr& reply = job.GetResult().cast();
			io->Lock(); {
				io->completed = true;
				io->reply = reply;
			} io->Unlock();
			Servers->ReplyToRequestClient(io); // in-thread job

			if (routerRequests) {
				routerRequests->Erase(request_index);
			}
		});
	}
	else {
		if (routerRequests) {
			routerRequests->Erase(request_index);
		}
	}
};
#endif
#endif


static void CurlExampleFunctions() {
	AUTO fileSys_write_data = [](void* ptr, size_t size, size_t nmemb, FILE * stream)->size_t {
		size_t written = std::fwrite(ptr, size, nmemb, stream);
		return written;
	};

	cweeCURL curl;
	{
		int result;
		FILE* fp = fopen("", "wb");
		result = curl.setopt(cweeCURL::CURLoption::CURLOPT_CUSTOMREQUEST, "GET");
		result = curl.setopt(cweeCURL::CURLoption::CURLOPT_URL, "https://find-any-ip-address-or-domain-location-world-wide.p.rapidapi.com/iplocation?apikey=873dbe322aea47f89dcf729dcc8f60e8");
		curl.setopt(cweeCURL::CURLoption::CURLOPT_WRITEFUNCTION, fileSys_write_data);
		curl.setopt(cweeCURL::CURLoption::CURLOPT_WRITEDATA, fp);
		cweeThreadedList<cweeStr> headers;
		headers.Append("X-RapidAPI-Key: 5ad46c78bbmsh509c23a7b43a6d5p1e1963jsn4998d1523143");
		headers.Append("X-RapidAPI-Host: find-any-ip-address-or-domain-location-world-wide.p.rapidapi.com");
		result = curl.setopt(cweeCURL::CURLoption::CURLOPT_HTTPHEADER, headers);
		result = curl.perform();
		fclose(fp);
	}
};


//#endif