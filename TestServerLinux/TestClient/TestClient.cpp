#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <boost/asio.hpp>

#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include "../shared/TestMessage.h"

#include <boost/lockfree/spsc_queue.hpp>
#include "ClientFn.hpp"



using namespace std;
using boost::asio::ip::tcp;

#define REPETITIONS 10000

class TestClient {
public:
	TestClient(const char * ip, int port)
	{
		if (initsocket(ip, port) < 0) {
			cout << " fail to connect! exit" << endl;
			throw " fail to connect! exit";
		}
		
		//start thread in active polling
		socketThread = thread(&TestClient::spin, this);
		
	}
	
	void spin() {
		int sent = 0;
		int received = 0;
		TestMessage* pSendMsg = nullptr;
		TestMessage* pRecvMsg  = new TestMessage();
			
		for (;;) {
			
			if (pSendMsg != nullptr || sendq_.pop(pSendMsg)) {
				while (sent != pSendMsg->length()) {
					int deta = sendsocket(pSendMsg->data() + sent, pSendMsg->length() - sent);
					if (deta <= -2) {
						cerr << "failure serious, exit thread" << endl;
						return;
					}						
					if (deta <= -1) //would block, do sth else
						break;
					sent += deta;
				}
				if (pSendMsg != nullptr && pSendMsg->length() == sent) {
					delete pSendMsg;
					pSendMsg = nullptr;
					sent = 0;
				}			
			}
			
			while (received < TestMessage::header_length) {
				
				int deta = recvsocket(pRecvMsg->data() + received, TestMessage::header_length - received);
				if (deta <= -2) {
					cerr << "failure serious, exit thread" << endl;
					return;
				}						
				if (deta <= -1) {//would block, do sth else 
					break;
				}
				received += deta;
				if (received >= TestMessage::header_length && !pRecvMsg->decode_header()) {
					
					cerr << "unexpected lenght of recieved stuff" << endl;
					return;
				}
			}
			
			while (received >= TestMessage::header_length && received < pRecvMsg->length()) {
				int deta = recvsocket(pRecvMsg->data() + received, pRecvMsg->length() - received);
				if (deta <= -2) {
						cerr << "failure serious, exit thread" << endl;
						return;
					}						
				if (deta <= -1) //would block, do sth else
					break;
				received += deta;
				
				if (received >= pRecvMsg->length()) {
					
					TestData* pData = reinterpret_cast<TestData *> (pRecvMsg->body());
					auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch().count() - pData->tick;
					//sprintf(buffer, "%d %010lld", pData->id,  elapsed);				
					cout << "elapsed ns:" << elapsed <<  " " <<  pData->id << endl;	
					
					received = 0;
					/*
					while (!recvq_.push(pRecvMsg));
					{
						
						cout << "finished proc recv body req" << endl;
						received = 0;
						pRecvMsg  = new TestMessage();
						
					}
					*/
				}
			}
		}
	}
	void write(TestMessage* pmsg) {
		while (!sendq_.push(pmsg)) ;
		cout << " pushed" << endl;
	}
	TestMessage* receive() {
		TestMessage* value;
		while (!recvq_.pop(value));
		return value;
	}
	
	
private:
	thread socketThread;
	boost::lockfree::spsc_queue< TestMessage* , boost::lockfree::capacity<512>> sendq_;
	boost::lockfree::spsc_queue< TestMessage* , boost::lockfree::capacity<512>> recvq_;

};
int main(int argc, char* argv[])
{
	if (argc < 4) {
		cout << "please enter host and port and order rate" << endl;
		return 1;
	}
	TestClient client(argv[1], atoi(argv[2]));
/*
	thread t([&client]() {
		TestMessage* p ;
		
		while (( p = client.receive()) != nullptr) {
			TestData* pData = reinterpret_cast<TestData *> (p->body());
			auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch().count() - pData->tick;
			//sprintf(buffer, "%d %010lld", pData->id,  elapsed);				
			cout << "elapsed ns:" << elapsed <<  " " <<  pData->id << endl;	
			delete p;
		}
	});
*/	
	chrono::microseconds sleepTime((int)(1000000 / atof(argv[3])));
	for (int i = 0; i < REPETITIONS; ++i) {
		cout << "sleeping" << endl;
		this_thread::sleep_for(sleepTime);

		TestMessage* pMsg = new TestMessage();

		TestData* pData = reinterpret_cast<TestData*>(pMsg->body());
		pData->id = i;
		pData->tick = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		pMsg->body_length(sizeof(TestData));
		pMsg->encode_header();
		client.write((pMsg));
	}	
    return 0;
}

