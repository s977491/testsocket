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

#include "../shared/TestMessage.h"
using namespace std;
using boost::asio::ip::tcp;

class TestSession : public enable_shared_from_this<TestSession>{
public:
	TestSession(tcp::socket socket) :
		socket_(move(socket))
	{
		ostringstream oss;
		boost::system::error_code ec;
		auto endpt = socket_.remote_endpoint(ec);
		oss << endpt;
		remoteAddress = oss.str();
	}
	void start() {
		cout << "connected to " << remoteAddress << endl;
		boost::system::error_code ec;
		socket_.set_option(tcp::no_delay(true), ec);
		if (!ec) {
			cerr << ec.message() << endl;
		}
		
		do_read_header();
	}
private:
	void do_read_header() {
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(msg_.data(), TestMessage::header_length),
			[this, self](boost::system::error_code ec, size_t length) {
			if (!ec && msg_.decode_header()) {
				do_read_body();
			}
			else {
				cerr << "connection failed :" << remoteAddress << endl;
			}
		}
		);
	}
	void do_read_body() {
		auto self(shared_from_this());
		boost::asio::async_read(socket_,
			boost::asio::buffer(msg_.body(), msg_.body_length()),
			[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (!ec) {
				//write back instantly 
				do_write();
			}
			else {
				cerr << "connection failed at reading:" << remoteAddress << endl;
			}
		}
		);
	}

	void do_write() {
		auto self(shared_from_this());
		boost::asio::async_write(socket_,
			boost::asio::buffer(msg_.data(), msg_.length()),
			[this, self](boost::system::error_code ec, std::size_t length)
			{
				if (!ec) {
					do_read_header();
				}
				else {
					cerr << "connection failed at writing :" << remoteAddress << endl;
				}
			}
		);
	}

	string remoteAddress;
	tcp::socket socket_;
	TestMessage msg_;
};

class TestServer {
public:
	TestServer(boost::asio::io_service& io_service,
		const tcp::endpoint& endpoint)
		: acceptor_(io_service, endpoint),
		socket_(io_service) {
		
		do_accept();
	}
private:
	void do_accept() {
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec) {
			if (!ec) {
				make_shared<TestSession>(move(socket_))->start();
			} 
			//this_thread::sleep_for(100ms);
			do_accept();
		});
	}

	tcp::acceptor acceptor_;
	tcp::socket socket_;
};

int main(int argc, char * argv[])
{
	try {
		if (argc < 2) {
			cerr << "please provide binding port" << endl;
			return 1;
		}
		boost::asio::io_service io_service;
		tcp::endpoint endpoint(tcp::v4(), atoi(argv[1]));
		TestServer myServer(io_service, endpoint);
		io_service.run();
	}
	catch (exception& e) {
		cerr << e.what() << endl;
	}
    return 0;
}

