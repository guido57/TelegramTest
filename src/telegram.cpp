#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ctime>
#include "telegram.h"

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "1436244177:AAGX4-8faRQAAetxPE5d0dC5FqAyOHA31ZQ"

#define CHAT_ID "-414817855" // ID for group Cozz

const unsigned long BOT_MTBS = 1000; // mean time between scan messages
// ===========================================================
// tosk run by Taskscheduler to handle the telegram messages
// ===========================================================
TaskTelegram::~TaskTelegram(){};
TaskTelegram::TaskTelegram(unsigned long interval, Scheduler *aS, void (*myCallback)()) : Task(interval, TASK_FOREVER, aS, true)
{
  _myCallback = myCallback;
};
bool TaskTelegram::Callback()
{
  _myCallback();
  return true;
};
// ===================================================
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime;          // last time messages' scan has been done
bool Start = false;
bool Restart = false;
TaskTelegram * myTaskTelegram; 

// =======================================================
// handy function to split words separated by a character
// =======================================================
const std::vector<std::string> split(const std::string& string_to_be_splitted, const char& separator_char)
{
	std::string buff{""};
	std::vector<std::string> v;
	
	for(auto n:string_to_be_splitted)
	{
		if(n != separator_char) buff+=n; else
		if(n == separator_char && buff != "") { v.push_back(buff); buff = ""; }
	}
	if(buff != "") v.push_back(buff);
	
	return v;
};

// =======================================================
// callback for new received messages
// =======================================================
void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/start" or text == "/help" or text == "/h" or text == "/?")
    {
      String welcome = "Welcome to CozzBot Telegram Bot, " + from_name + ".\n";
      welcome += "your chat id is " + chat_id + "\n\n";
      welcome += "\n\n";
      welcome += "/start or /help or /h or /? to show this message\n\n";
      welcome += "/gpio,x,y digital write y (0 or 1) to GPIOx e.g. /gpio,12,1 set 1 (high) to GPIO12\n\n";
      welcome += "/gpio,x digital read the level (0 or 1) of GPIOx e.g. /gpio,12 read 0 (low) or 1 (high) fron GPIO12\n\n";
      bot.sendMessage(chat_id, welcome);
    }

    // =======================================================
    // handle digital read and write
    // =======================================================
    if (text.startsWith("/gpio"))
    {
      // e.g. /gpio12,1     set GPIO12 to HIGH
      std::vector<std::string> words = split(text.c_str(), ',');
      if(words.size() == 3){
        // digital out
        int gpio_num = atoi(words[1].c_str());
        int level = atoi(words[2].c_str());
        pinMode(gpio_num,OUTPUT);
        digitalWrite(gpio_num,level);
        String answer = "GPIO" + String(gpio_num) + " was set to " + String(level);
        Serial.println(answer);
        bot.sendMessage(chat_id, answer);

      }else if(words.size() == 2){
        // digital input
        int gpio_num = atoi(words[1].c_str());
        pinMode(gpio_num,INPUT);
        int level = digitalRead(gpio_num);
        String answer = "GPIO" + String(gpio_num) + " is " + String(level);
        Serial.println(answer);
        bot.sendMessage(chat_id, answer);
      } 
    }
    // =======================================================
    // handle restart
    // =======================================================
    if (text.startsWith("/restart"))
    {
      String answer = "restart received";
      Serial.println(answer);
      bot.sendMessage(chat_id, answer);
      Restart = true;
    }
  }
}
// =====================================
// call this after WiFi connection
// =====================================
void telegram_setup(Scheduler * _scheduler)
{
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  Serial.print("Retrieving time: ");
  configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    //Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  tm *ltm = localtime(&now);
  String now_str = String(ltm->tm_hour) + ":" + String(ltm->tm_min) + ":" + String(ltm->tm_sec);
  Serial.println(now_str);

  myTaskTelegram = new TaskTelegram(BOT_MTBS,_scheduler,telegram_loop); 

  String answer="CozzBot just started!\r\n";
  answer += "IP address is " + String(WiFi.localIP()) + "\r\n";
  bot.sendMessage(CHAT_ID, answer, "");
  
}

void telegram_loop()
{
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    if(Restart)
      ESP.restart();  

}
