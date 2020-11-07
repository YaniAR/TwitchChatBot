#include "Socket.h"
#include "Endpoint.h"
#include <vector>
#include <thread>
#include <memory>
#include <mutex>
#include <chrono>
#include <atomic>
#include <random>
#include <ctime>
#include <map>
#include <fstream>

template<typename T>
void pop_front(std::vector<T>& v)
{
	assert(!v.empty());
	v.erase(v.begin());
}

namespace Randomiser
{
	std::mt19937 generator{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
}
int getRandomNumber(int min, int max)
{
	std::uniform_int_distribution<int> rand{ min, max };
	return rand(Randomiser::generator);
}

class Client
{
private:
	Socket s{};
	IPEndPoint twitchSock{ "irc.chat.twitch.tv", 6667 };
	
	std::atomic<bool> running{};
	std::atomic<bool> connected{};
	std::mutex _inMutex{}; // Into client
	std::mutex _outMutex{}; // Out from client
	std::thread inThread{}; // Data Into Client 
	std::thread outThread{}; // Data From Client

	int flag{ 0 };
	std::string input{};
	bool leftStatus{ false };
	
	void RenderText()
	{
		if (twitchSock.GetIpVersion() == Socket::IPVersion::IPV4)
		{
			std::cout << "Hostname: " << twitchSock.GetHostname() << '\n';
			std::cout << "IP: " << twitchSock.getIPString() << '\n';
			std::cout << "Port: " << twitchSock.getPort() << '\n';
			std::cout << "IP Bytes..." << '\n';

			// Prints each bytes
			for (auto& digits : twitchSock.getIPBytes())
				std::cout << (int)digits << "...";
			std::cout << '\n';
		}
	}

	void sendData(const std::string& tag)
	{
		std::string submit{};
		submit = tag + input;
		submit += '\n';
		s.Send(submit.c_str(), submit.length(), flag);
	}

	void SendMessage(const std::string& messageToSend)
	{
		if (messageToSend == ":quit")
		{
			if (!leftStatus)
			{
				sendData("PATH #");
				std::cout << "LEFT " << input << "..." << '\n';
			}
			quit();
			return;
		}
		
		if (messageToSend == ":leave")
		{
			sendData("PART #");
			std::cout << "LEAVING " << input << "..." << '\n';

			leftStatus = true;
			return;
		}
		
		auto found{ messageToSend.find(":join ") };
		if (found != std::string::npos)
		{
			std::string channelName{ messageToSend.substr(found + 6) };
			
			if (!leftStatus)
			{
				sendData("PATH #");
				std::cout << "LEFT " << input << "..." << '\n';
			}
			
			input = channelName;
			sendData("JOIN #");
			
			std::cout << "JOINING " << input << "..." << '\n';
			leftStatus = false;
			return;
		}

		found = messageToSend.find(":");
		if (found == 0)
		{
			std::string submitString{};
			submitString = "PRIVMSG #" + input + " " + messageToSend;
			submitString += '\n';
			s.Send(submitString.c_str(), submitString.length(), flag);
		}
	}

public:
	Client()
		: running { true }, connected{ false },
		outThread(&Client::clientToServer, this), inThread(&Client::serverToClient, this)
	{
	}
	
	~Client()
	{
		outThread.join();
		inThread.join();
	}

	static void errorMessage()
	{
		// Socket timeout is 5 seconds. Change val to change timeout time at Socket::Recv()
		std::cout << "Console: Closing...\n";
		std::this_thread::sleep_for(std::chrono::seconds(3));
		std::cout << "Console: I'm trying, please be patient.\n";
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	inline void quit()
	{
		running = false;
		connected = false;

		std::map<std::string, std::vector<std::string>> gram{};

		std::ifstream wordStream{ "sample.txt" };
		std::string fWord;
		std::string sWord;

		int i{ 0 };
		while (wordStream)
		{
			wordStream >> fWord;
			if (i != 0)
				gram[sWord].push_back(fWord);
			wordStream >> sWord;
			gram[fWord].push_back(sWord);
			if (i == 0)
				i++;
		}

		markovIt(gram);
		std::cout << '\n';


		errorMessage();
	}

	std::string setData(const std::string& message, std::string submitString)
	{
		std::cout << message;
		std::cin >> input;
		
		submitString += input;
		submitString += '\n';

		return submitString;
	}

	void handleInput()
	{
		std::string messageToSend{};
		std::getline(std::cin, messageToSend);
		SendMessage(messageToSend);
	}

	void clientToServer()
	{
		if (running)
		{
			// Create the socket
			if (s.Create() == Socket::PResult::P_SUCCESS)
			{
				std::cout << "Socket successfully created.\n";

				// Connect to Twitch IRC
				if (s.Connect(twitchSock) == Socket::PResult::P_SUCCESS)
				{
					std::cout << "Successfully connected to Twitch.\n";

					RenderText();

					// Sends data
					std::string str{};

					str = setData("Enter your OAuth Token from https://twitchapps.com/tmi/ \nPASS oauth:<Twitch OAuth token>: ",
						"PASS ");
					s.Send(str.c_str(), str.length(), flag);

					str = setData("Enter your Twitch username :", "NICK ");
					s.Send(str.c_str(), str.length(), flag);

					str = setData("Enter the streamer's username in lowercase:", "JOIN #");
					s.Send(str.c_str(), str.length(), flag);

					// CORRECT
					connected = true;
				}
				
				else
				{
					std::cerr << "Could not connect to Twitch.\n";
					quit();
				}

				while (connected)
				{
					std::lock_guard<std::mutex> locker{ _outMutex };
					handleInput();
				}
			}
			else
			{
				std::cerr << "Socket was not created.\n";
				quit();
			}

		}
	}

	void markovIt(const std::map<std::string, std::vector<std::string>>& m)
	{
		std::string temp{};
		std::vector<std::string> possibilities{};
		std::string next{};

		int i{ 0 };
		int randomNumber{ getRandomNumber(0, m.size()) };
		for (const auto& n : m)
		{
			if (i == randomNumber)
			{
				temp = n.first;
			}
			++i;
		}
		
		std::cout << "===GENERATED===\n";
		auto found{ m.find(temp) };
		i = 0;
		while (found != m.end())
		{
			// 50 words
			if (i == 50)
				break;

			possibilities = found->second;
			next = possibilities.at(getRandomNumber(0, possibilities.size() - 1));

			std::cout << temp << " ";
			temp = next;

			found = m.find(temp);
			++i;
		}
	}

	void serverToClient()
	{
		char buffer[(int)MAXBYTE]{};
		std::string text{};
		std::vector<std::string> lines{ "", "", "" };
		
		int bytesReceived{ 0 };
		while (running)
		{
			if (connected)
			{
				std::lock_guard<std::mutex> locker{ _inMutex };

				s.Recv(buffer, (int)MAXBYTE, flag, bytesReceived);
				std::fstream openData("sample.txt", std::fstream::out | std::fstream::app);
				for (int j = 0; j < bytesReceived; j++)
				{
					text.append(1, buffer[j]);
					if (buffer[j] == '\n')
					{
						// User name is between first ':' and '!'. 
						// User chat is after second ':'
						std::size_t m{ text.find('!') };
						std::string sUserName{ text.substr(1, m - 1) };
						size_t n{ text.find(':', m) };
						if (n != std::string::npos)
						{
							std::string chat{ text.substr(n + 1) };
							std::cout << sUserName.c_str() << ": " << chat.c_str() << '\n';

							openData << chat << '\n';

							pop_front(lines);
							lines.push_back(sUserName + ": " + chat);
						}

						text.clear();
					}
				}
			}
		}
	}

};

int main()
{
	WSADATA wsadata{};
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
		std::cerr << "Could not connect to window api.\n";
	
	// Check version
	if(LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wVersion) != 2)
		std::cerr << "Could not find the version.\n";
	
	// Success
	std::cout << "Winsock api successfully initialised.\n";

	std::unique_ptr<Client> run{ std::make_unique<Client>() };

	return 0;
}

