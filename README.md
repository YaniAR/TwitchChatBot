# TwitchChatBot
Creates a TCP server using blocking sockets [winsock] to send and receive data over Twitch IRC to make a chat bot

## Program Commands
You must use ":" at the start of every command in the program. Note that the chat message might go over the command. Just ignore and continue typing in the characters you want.

|Commands       |Description            |Example        |
|    :----:     |       :----:          |    :----:     |
|:join [channel]|Joins the channel      |:join pokimane |
|:leave         |Leaves the channel     |:leave         |
|:/[command]    |Sends a Twitch command |:/ban pokimane |
|:[message]     |Send a message to chat |:Nice stream!  |
|:quit          |Quits the program      |:quit          |

- You must be signed into Twitch and then get a OAuth token

- All inputs must be in lowercase, including the channel name. 

- Don't use the web address, use the chanel name!

- Prints "Listening..." if no chat messages are received over TCP. Timeout is 10 seconds.

- Change int val = [value] in Socket::SetSocketOption(param, param) in Socket.cpp to change the timeout timer.

## Using data
```C++
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

      pop_front(lines);
      lines.push_back(sUserName + ": " + chat);
    }

    text.clear();
  }
}
```
* bytesReceived = number of characters in the chat message.

* text = std::string of the whole chat message including "!" start

* sUsername = string of the user.

* chat = string of the message

* lines = vector consisting of the chat messages.

## Twitch API Documentation
https://dev.twitch.tv/docs/irc/guide
