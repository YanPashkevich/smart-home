#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#define Relay1 5  //Задаем пин реле (D1)
#define Relay2 4  //Задаем пин реле (D2)
#define ReedSwitch 16 //Задаем пин геркона (D0)
#define WIFI_SSID "***" //Название wi-fi
#define WIFI_PASSWORD "***" //Пароль от wi-fi

// Сигнализация
bool PrevState = 0; //Предыдущее состояние геркона 0-закрыто, 1-открыто]
int ModeAlarm = 0;  //Режим системы[0-выключено, 1-оповещение, 3-сигнализация(оповещение и включение реле]
String StateAlarm = "OFF";  //Состояние системы в настоящее время (для отображения системы)
String StateReedSwitch = "null";  //Состояние геркона в настоящее время (для отображения системы)

// Реле
String RelayState1 = "OFF";
String RelayState2 = "OFF";

// Состояние всей системы
String StateSystem;

// Создаем объект необходимый для wifi подключения
WiFiClientSecure secured_client;

// Конфигурация Telegram
#define BOT_TOKEN "***"  //Токен бота полученный от BotFather
#define CHAT_ID "-***"  //Чат id[для получения id https://api.telegram.org/bot(вставить бот токен)/getUpdates Для получения id группы (id всегда с минусом)
const unsigned long BOT_MTBS = 3000;  //Константа для задержки между запросом и значением обработки (должен быть какой-то промежуток)
X509List cert(TELEGRAM_CERTIFICATE_ROOT); //Строка запрашивающая сертификат шифрования
UniversalTelegramBot bot(BOT_TOKEN, secured_client);  //Инициализируем конструктор
unsigned long bot_lasttime; //Переменная для хранения времени с последнего запроса серверов Telegram

void setup()
{
  Serial.begin(9600); //Инициализируем последовательный порт (ДЛЯ ОТЛАДКИ)

  pinMode(Relay1, OUTPUT);  //Режим работы заданного пина (D1)
  pinMode(Relay2, OUTPUT);  //Режим работы заданного пина (D2)
  pinMode(ReedSwitch, INPUT); //Режим работы заданного пина (D3)
  digitalWrite(Relay1, HIGH); //Обратная логика[HIGH-выключить реле, LOW-включить]
  digitalWrite(Relay2, HIGH); //Обратная логика[HIGH-выключить реле, LOW-включить]

  configTime(0, 0, "pool.ntp.org"); //Синхронизируем время
  secured_client.setTrustAnchors(&cert);  //Работа с сертификатом
  Serial.print("Connecting to WiFi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.print("\nWiFi connected. IP adress: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "HI! I'm online", ""); //3-й параметр для указания модифиации парсинга
}

void loop()
{
  // Сигнализация
  int SensorState = digitalRead(ReedSwitch);  //Переменная для считывания состояния геркона в режиме реального времени

  if (PrevState == 1)
  {
    if (SensorState == HIGH)
    {
      PrevState = 0;
      StateReedSwitch = "The door is CLOSED";
      // Система в режиме ОПОВЕЩЕНИЯ -> прислать сообщение о ЗАКРЫТИИ двери
      if (ModeAlarm == 1)
      {
        bot.sendMessage(CHAT_ID, StateReedSwitch, "");
      }

      // Система в режиме СИГНАЛИЗАЦИИ -> прислать сообщение о ЗАКРЫТИИ двери и ВЫКЛЮЧИТЬ реле
      if (ModeAlarm == 2)
      {
        String message = "Alarm!\nDoor is CLOSED";
        bot.sendMessage(CHAT_ID, message, "");
        RelayState1 = "OFF";
        RelayState2 = "OFF";
        digitalWrite(Relay1, HIGH);
        digitalWrite(Relay2, HIGH);
      }
    }
  }

  if (PrevState == 0)
  {
    if (SensorState == LOW)
    {
      PrevState = 1;
      StateReedSwitch = "The door is OPEN";
      // Система в режиме ОПОВЕЩЕНИЯ -> прислать сообщение об ОТКРЫТИИ двери
      if (ModeAlarm == 1)
      {
        bot.sendMessage(CHAT_ID, StateReedSwitch, "");
      }

      // Система в режиме СИГНАЛИЗАЦИИ -> прислать сообщение об ОТКРЫТИИ двери и ВКЛЮЧИТЬ реле
      if (ModeAlarm == 2)
      {
        String message = "Alarm!\nDoor is OPEN";
        bot.sendMessage(CHAT_ID, message, "");
        RelayState1 = "ON";
        RelayState2 = "ON";
        digitalWrite(Relay1, LOW);
        digitalWrite(Relay2, LOW);
      }
    }
  }

  delay(300);
  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1); //Записываем все принятые ботом сообщения и записываем в переменную
    while (numNewMessages)
    {
      //Цикл который будет выполнятся пока NumNewMessage != 0 и с каждой иттерацией мы будем вызывать функцию, которая будет обрабатывать сообщения
      handleNewMessages(numNewMessages);  //Функция обрабатывающая сообщения
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }
}

void handleNewMessages(int numNewMessages)
{
  //Принимает количество текущих сообщений на текущий момент
  for (int i = 0; i < numNewMessages; i++)
  {
    //Цикл который делает ровно столько итераций, сколько сообщений
    if (bot.messages[i].chat_id == CHAT_ID)
    {
      //Проверяем наш id чата, чтобы если мы создадим кучу ботов и распихаем их по группам
      String text = bot.messages[i].text; //Получаем сообщение
      Serial.print(text);
      // ВЫКЛЮЧИТЬ СИГНАЛИЗАЦИЮ
      if (text == "/system_off")
      {
        ModeAlarm = 0;
        StateAlarm = "OFF";
        bot.sendMessage(CHAT_ID, "System state: OFF", "");
      }

      // СИГНАЛИЗАЦИЯ: РЕЖИМ ОПОВЕЩЕНИЕ 
      if (text == "/system_alert")
      {
        ModeAlarm = 1;
        StateAlarm = "ALERT";
        bot.sendMessage(CHAT_ID, "System state: ALERT", "");
      }

      // СИГНАЛИЗАЦИЯ: РЕЖИМ СИГНАЛИЗАЦИИ 
      if (text == "/system_on")
      {
        ModeAlarm = 2;
        StateAlarm = "ON";
        bot.sendMessage(CHAT_ID, "System state: ON", "");
      }

      // ВКЛЮЧИТЬ реле 1
      if (text == "/1relay_on")
      {
        RelayState1 = "ON";
        digitalWrite(Relay1, LOW);
        bot.sendMessage(CHAT_ID, "1 relay ON", "");
      }

      // ВКЛЮЧИТЬ реле 2
      if (text == "/2relay_on")
      {
        RelayState2 = "ON";
        digitalWrite(Relay2, LOW);
        bot.sendMessage(CHAT_ID, "2 relay ON", "");
      }

      // ВЫКЛЮЧИТЬ реле 1
      if (text == "/1relay_off")
      {
        RelayState1 = "OFF";
        digitalWrite(Relay1, HIGH);
        bot.sendMessage(CHAT_ID, "1 relay OFF", "");
      }

      // ВЫКЛЮЧИТЬ реле 2
      if (text == "/2relay_off")
      {
        RelayState2 = "OFF";
        digitalWrite(Relay2, HIGH);
        bot.sendMessage(CHAT_ID, "2 relay OFF", "");
      }

      // ВКЛЮЧИТЬ все реле
      if (text == "/relays_on")
      {
        RelayState1 = "ON";
        RelayState2 = "ON";
        digitalWrite(Relay1, LOW);
        digitalWrite(Relay2, LOW);
        bot.sendMessage(CHAT_ID, "Relays are ON", "");
      }

      // ВЫКЛЮЧИТЬ все реле
      if (text == "/relays_off")
      {
        RelayState1 = "OFF";
        RelayState2 = "OFF";
        digitalWrite(Relay1, HIGH);
        digitalWrite(Relay2, HIGH);
        bot.sendMessage(CHAT_ID, "Relays are OFF", "");
      }

      // ВЫКЛЮЧИТЬ ВСЕ
      if (text == "/everything_off")
      {
        RelayState1 = "OFF";
        RelayState2 = "OFF";
        ModeAlarm = 0;
        StateAlarm = "OFF";
        StateReedSwitch = "null";
        digitalWrite(Relay1, HIGH);
        digitalWrite(Relay2, HIGH);
        bot.sendMessage(CHAT_ID, "Everything is OFF", "");
      }

      if (text == "/system_state")
      {
        String StateAlarmP = "System state: " + StateAlarm;
        String StateReedSwitchP = "Door state: " + StateReedSwitch;
        String RelayStateP1 = "1 relay state: " + RelayState1;
        String RelayStateP2 = "2 relay state: " + RelayState2;
        StateSystem = "System WORK - OK\n" + StateAlarmP + String("\n") + StateReedSwitchP + String("\n") + RelayStateP1 + String("\n") + RelayStateP2;
        bot.sendMessage(CHAT_ID, StateSystem, "");
      }
    }
  }
}
