// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <boost/asio/steady_timer.hpp>
#include <chrono>
#include "../shared/TestMessage.h"

using namespace std;
using boost::asio::ip::tcp;

#define REPETITIONS 10000

class TestClient {
public:
	TestClient(boost::asio::io_service& io_service,
		tcp::resolver::iterator endpoint_iterator)
		: socket_(io_service), io_service_(io_service)
	{
		//showNoDelayStatus();
		boost::system::error_code ec;

		socket_.set_option(tcp::no_delay(true), ec);
		if (!ec) {
			cerr << ec.message() << endl;
		}
		//showNoDelayStatus();

		do_connect(endpoint_iterator);
	}

	void write(shared_ptr<TestMessage> pmsg) {
		io_service_.dispatch(
			[this, pmsg]()
		{
			bool writing = !write_msgs_.empty();
			write_msgs_.push_back(pmsg);
			if (!writing) {
				do_write();
			}
		}
		);
	}
	
private:
	void showNoDelayStatus() {
		boost::asio::ip::tcp::no_delay option;
		socket_.get_option(option);
		bool is_set = option.value();
		cout << "no delay : " << is_set << endl;
	}
	void do_write() {
		boost::asio::async_write(socket_,
			boost::asio::buffer(write_msgs_.front()->data(), write_msgs_.front()->length()),
			[this](boost::system::error_code ec, size_t length)
		{
			if (!ec) {
				write_msgs_.pop_front();
				if (!write_msgs_.empty()) {
					do_write();
				}
			}
			else {
				cerr << "fail to write" << endl;
				socket_.close();
			}
		});
	}
	void do_connect(tcp::resolver::iterator endpoint_iterator) {
		boost::system::error_code ec;
		socket_.set_option(tcp::no_delay(true), ec);
		if (!ec) {
			cerr << ec.message() << endl;
		}
		boost::asio::async_connect(socket_, endpoint_iterator,
			[this](boost::system::error_code ec, tcp::resolver::iterator) {
			if (!ec) {

				boost::system::error_code ec;

				socket_.set_option(tcp::no_delay(true), ec);
				if (!ec) {
					cerr << ec.message() << endl;
				}

				do_read_header();
			}
			else {
				cerr << "fail to connect" << endl;
				socket_.close();
			}
		});
	}

	void do_read_header() {
		boost::asio::async_read(socket_,
			boost::asio::buffer(msg_.data(), TestMessage::header_length),
			[this](boost::system::error_code ec, size_t length) {
				if (!ec) {
					msg_.decode_header();
					do_read_body();
				}
				else {
					cerr << "fail to read header" << endl;
					socket_.close();
				}
			}
		);
	}

	void do_read_body() {
		boost::asio::async_read(socket_,
			boost::asio::buffer(msg_.body(), msg_.body_length()),
			[this](boost::system::error_code ec, size_t length)
		{
			if (!ec) {
				//cal stuff
				TestData* pData = reinterpret_cast<TestData *> (msg_.body());
				auto elapsed = std::chrono::high_resolution_clock::now().time_since_epoch().count() - pData->tick;

				sprintf(buffer, "%10lld", elapsed);
				
				cout << "elapsed ns:" << elapsed <<  endl;
				do_read_header();
			}
			else {
				cerr << "fail to read body" << endl;
				socket_.close();
			}
		});
	}

	tcp::socket socket_;
	boost::asio::io_service& io_service_;
	TestMessage msg_;
	list<shared_ptr<TestMessage> > write_msgs_;
	char buffer[100];
};
int main(int argc, char* argv[])
{
	auto start = std::chrono::high_resolution_clock::now();  // start timer

	if (argc < 4) {
		cout << "please enter host and port and order rate" << endl;
		return 1;
	}

	boost::asio::io_service io_service;

	tcp::resolver resolver(io_service);
	auto endpoint_iterator = resolver.resolve({ argv[1], argv[2] });
	TestClient client(io_service, endpoint_iterator);

	thread t([&io_service]() {io_service.run(); });
	
	chrono::microseconds sleepTime((int)(1000000 / atof(argv[3])));
	for (int i = 0; i < REPETITIONS; ++i) {
		
		this_thread::sleep_for(sleepTime);

		shared_ptr<TestMessage> pMsg = make_shared<TestMessage>();

		TestData* pData = reinterpret_cast<TestData*>(pMsg->body());
		pData->id = i;
		pData->tick = std::chrono::high_resolution_clock::now().time_since_epoch().count();
		pMsg->body_length(sizeof(TestData));
		pMsg->encode_header();
		client.write(move(pMsg));
	}
	

    return 0;
}

