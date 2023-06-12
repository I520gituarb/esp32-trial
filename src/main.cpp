#include <TFT_eSPI.h> // TFT LED小屏幕
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h> //连接到WIFI
#include <Adafruit_AHTX0.h>
#include <ArduinoJson.h>
#include "BMP.h" //gif图

#define NTP1 "ntp1.aliyun.com"
#define NTP2 "ntp2.aliyun.com"
#define NTP3 "ntp3.aliyun.com"
#define WIDTH 160
#define HEIGHT 80
#define I2C_SDA 0 // 传感器I2C使用的管脚
#define I2C_SCL 1
#define Key0 8    // up
#define Key1 13   // down
#define Key2 5    // right
#define Key3 9    // left
#define Key4 4    // mid 暂时没有使用
#define REDLED 12 // WIFI开始连接，亮灯，连接成功后，灯灭
//-----1------引入库与定义引脚

TFT_eSPI tft = TFT_eSPI(); // Invoke library, pins defined in User_Setup.h
TFT_eSprite spr = TFT_eSprite(&tft);
Adafruit_AHTX0 aht;
WiFiClient client; // 用户端wifi类

// =========================================================================
// Setup
// =========================================================================
const char *ssid = "vivo S15";         // WIFI账户
const char *password = "temporary123"; // WIFI密码
const char *host = "192.168.18.111";   // 记得电脑也要连手机热点，改成电脑正在连的wifi的分配的ip
int16_t gpu_usage;                     // 查看这三个的使用情况（usage）
int16_t cpu_usage;
int16_t ram_usage;
int i = 1;

char stateFlag = 0;  // 主状态机状态
char stateFlag2 = 0; // 附状态机状态
char keyFlag = 0;    // 按键状态

void KeyScan() // 按键状态，应该是检查你按了哪个
{
  if (keyFlag == 1)
  {
    // 检查按键是否释放
    // Key0 =8    // up
    // Key1 =13   // down
    // Key2= 5    // right
    // Key3 =9    // left
    // Key4 =4    // mid 暂时没有使用
    if (digitalRead(Key0) == 0)
      return;
    else if (digitalRead(Key1) == 0)
      return;
    else if (digitalRead(Key2) == 0)
      return;
    else if (digitalRead(Key3) == 0)
      return;
    else if (digitalRead(Key4) == 0)
      return;
    else
      keyFlag = 0; // 按键已经释放
  }

  else
  {
    if (digitalRead(Key0) == 0)
    {
      keyFlag = 1;
      stateFlag = stateFlag + 1; // 主状态机状态
      if (stateFlag > 2)
        stateFlag = 0;
      return;
    }
    else if (digitalRead(Key1) == 0)
    {
      keyFlag = 1;
      if (stateFlag > 0)
        stateFlag = stateFlag - 1;
      else
        stateFlag = 2;
      return;
    }
    else if (digitalRead(Key2) == 0)
    {
      keyFlag = 1;
      if (stateFlag2 > 0)
        stateFlag2 = stateFlag2 - 1;
      else
        stateFlag2 = 2;
      return;
    }
    else if (digitalRead(Key3) == 0)
    {
      keyFlag = 1;

      stateFlag2 = stateFlag2 + 1;
      if (stateFlag2 > 2)
        stateFlag2 = 0;
      return;
    }
    else if (digitalRead(Key4) == 0)
    {
      keyFlag = 1;
      return;
    }
  }
}

void PC_infor() // 通过AIDA64，获取PC硬件使用信息，即cpu情况、gpu情况
{               // 参考https://blog.csdn.net/weixin_42487906/article/details/119990801
  // http://www.taichi-maker.com/homepage/iot-development/iot-dev-reference/esp8266-c-plus-plus-reference/wificlient/connect/

  if (!client.connect(host, 80)) // 先连接pc电脑，看看有没有连接成功
  {
    Serial.println("Connect host failed!");
    return;
  }
  Serial.println("host Conected!");

  String getUrl = "/sse";
  client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Content-Type=application/json;charset=utf-8\r\n" + "Host: " + host + "\r\n" + "User-Agent=ESP32\r\n" + "Connection: close\r\n\r\n");
  Serial.println("Get send");

  delay(10);

  char endOfHeaders[] = "\n\n";
  bool ok = client.find(endOfHeaders);
  if (!ok)
  {
    Serial.println("No response or invalid response!");
  }
  Serial.println("Skip headers");

  String line = "";

  line += client.readStringUntil('\n');

  Serial.println("Content:");
  Serial.println(line);

  int16_t dataStart = 0;
  int16_t dataEnd = 0;
  String dataStr;

  // 接下来要获取data，那个gpu占用情况的数字
  char gpu_use[] = "GPU Utilization";
  dataStart = line.indexOf(gpu_use) + strlen(gpu_use); // 数字的开头
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd); // 把数字（现在是字符串）截取下来
  gpu_usage = dataStr.toInt();                  // 变数字

  char cpuUsage[] = "CPU Utilization";
  dataStart = line.indexOf(cpuUsage) + strlen(cpuUsage);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  cpu_usage = dataStr.toInt();

  /*char ramUsage[] = "Memory Utilization";
  dataStart = line.indexOf(ramUsage) + strlen(ramUsage);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  ram_usage = dataStr.toInt();

  Serial.print("GPU usage :");
  Serial.println(gpu_usage);
  Serial.print("CPU usage :");
  Serial.println(cpu_usage);
  Serial.print("RAM usage :");
  Serial.println(ram_usage);*/

  //
  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);

  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextFont(2);
  spr.setTextSize(2);
  spr.setCursor(20, 8);
  spr.print("CPU: ");
  spr.print(cpu_usage);
  spr.println("%");
  spr.setCursor(20, 40);
  spr.print("GPU: ");
  spr.print(gpu_usage);
  spr.println("%");
  // spr.print("RAM: "); spr.print(ram_usage);spr.println("%");//不显示内存占用率
  spr.pushSprite(0, 0);
}

void PC_infor1() // 通过AIDA64，获取PC硬件使用信息
{                // 参考https://blog.csdn.net/weixin_42487906/article/details/119990801
  // http://www.taichi-maker.com/homepage/iot-development/iot-dev-reference/esp8266-c-plus-plus-reference/wificlient/connect/
  if (!client.connect(host, 80))
  {
    Serial.println("Connect host failed!");
    return;
  }
  Serial.println("host Conected!");

  String getUrl = "/sse";
  client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Content-Type=application/json;charset=utf-8\r\n" + "Host: " + host + "\r\n" + "User-Agent=ESP32\r\n" + "Connection: close\r\n\r\n");
  Serial.println("Get send");

  delay(10);

  char endOfHeaders[] = "\n\n";
  bool ok = client.find(endOfHeaders);
  if (!ok)
  {
    Serial.println("No response or invalid response!");
  }
  Serial.println("Skip headers");

  String line = "";

  line += client.readStringUntil('\n');

  Serial.println("Content:");
  Serial.println(line);

  int16_t dataStart = 0;
  int16_t dataEnd = 0;
  String dataStr;

  char gpu_use[] = "GPU Utilization";
  dataStart = line.indexOf(gpu_use) + strlen(gpu_use);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  gpu_usage = dataStr.toInt();

  /*char cpuUsage[] = "CPU Utilization";
  dataStart = line.indexOf(cpuUsage) + strlen(cpuUsage);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  cpu_usage = dataStr.toInt();

  char ramUsage[] = "Memory Utilization";
  dataStart = line.indexOf(ramUsage) + strlen(ramUsage);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  ram_usage = dataStr.toInt();

  Serial.print("GPU usage :");
  Serial.println(gpu_usage);
  Serial.print("CPU usage :");
  Serial.println(cpu_usage);
  Serial.print("RAM usage :");
  Serial.println(ram_usage);*/

  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);

  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN, TFT_BLACK);
  spr.setTextFont(2);
  spr.setTextSize(2);
  // spr.setCursor(20, 8);
  // spr.print("CPU: "); spr.print(cpu_usage); spr.println("%");
  // spr.setCursor(20, 40);
  // spr.pushImage(0,0,64,64,bmp2_lbili);
  spr.setCursor(8, 24);
  spr.print("GPU: ");
  spr.print(gpu_usage);
  spr.println("%");
  // spr.print("RAM: "); spr.print(ram_usage);spr.println("%");//不显示内存占用率
  spr.pushSprite(0, 0);
}

void PC_infor2() // 通过AIDA64，获取PC硬件使用信息
{                // 参考https://blog.csdn.net/weixin_42487906/article/details/119990801
  // http://www.taichi-maker.com/homepage/iot-development/iot-dev-reference/esp8266-c-plus-plus-reference/wificlient/connect/
  if (!client.connect(host, 80))
  {
    Serial.println("Connect host failed!");
    return;
  }
  Serial.println("host Conected!");

  String getUrl = "/sse";
  client.print(String("GET ") + getUrl + " HTTP/1.1\r\n" + "Content-Type=application/json;charset=utf-8\r\n" + "Host: " + host + "\r\n" + "User-Agent=ESP32\r\n" + "Connection: close\r\n\r\n");
  Serial.println("Get send");

  delay(10);

  char endOfHeaders[] = "\n\n";
  bool ok = client.find(endOfHeaders);
  if (!ok)
  {
    Serial.println("No response or invalid response!");
  }
  Serial.println("Skip headers");

  String line = "";

  line += client.readStringUntil('\n');

  Serial.println("Content:");
  Serial.println(line);

  int16_t dataStart = 0;
  int16_t dataEnd = 0;
  String dataStr;

  /*char gpu_use[] = "GPU Utilization";
  dataStart = line.indexOf(gpu_use) + strlen(gpu_use);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  gpu_usage = dataStr.toInt();*/

  char cpuUsage[] = "CPU Utilization";
  dataStart = line.indexOf(cpuUsage) + strlen(cpuUsage);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  cpu_usage = dataStr.toInt();

  /*char ramUsage[] = "Memory Utilization";
  dataStart = line.indexOf(ramUsage) + strlen(ramUsage);
  dataEnd = line.indexOf("%", dataStart);
  dataStr = line.substring(dataStart, dataEnd);
  ram_usage = dataStr.toInt();

  Serial.print("GPU usage :");
  Serial.println(gpu_usage);
  Serial.print("CPU usage :");
  Serial.println(cpu_usage);
  Serial.print("RAM usage :");
  Serial.println(ram_usage);*/

  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);

  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_RED, TFT_BLACK);
  spr.setTextFont(2);
  spr.setTextSize(2);

  spr.setCursor(8, 24);
  spr.print("CPU: ");
  spr.print(cpu_usage);
  spr.println("%");
  // spr.print("RAM: "); spr.print(ram_usage);spr.println("%");//不显示内存占用率
  spr.pushSprite(0, 0);
}

void temp_hum() // 温度和湿度初始化？
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degrees C");
  Serial.print("Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println("% rH");

  // Draw a black rectangle in sprite so when we move it 1 pixel it does not leave a trail
  // on the black screen background
  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);
  spr.setCursor(0, 0);

  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextFont(2);
  spr.setTextSize(2);
  spr.setCursor(0, 8);
  spr.print("Temp: ");
  spr.println(temp.temperature, 1); // 默认是保留两位小数，这里我们保留一位小数
  spr.setCursor(0, 40);
  spr.print("Humi: ");
  spr.print(humidity.relative_humidity, 1);
  spr.println("%");
  spr.pushSprite(0, 0);
}

void temp() // 温度
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_WHITE);
  spr.setCursor(0, 0);

  spr.fillSprite(TFT_WHITE);
  spr.setTextColor(TFT_BLACK, TFT_WHITE);
  spr.setTextFont(2);
  spr.setTextSize(2);
  spr.pushImage(8, 20, 40, 40, bmp3_temp);
  spr.setCursor(60, 24);
  spr.println(temp.temperature, 1);
  spr.pushSprite(0, 0);
}

void Humidity() // 湿度
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_WHITE);
  spr.setCursor(0, 0);

  spr.fillSprite(TFT_WHITE);
  spr.setTextColor(TFT_BLACK, TFT_WHITE);
  spr.setTextFont(2);
  spr.setTextSize(2);
  spr.pushImage(8, 20, 40, 40, bmp6_humnew);
  spr.setCursor(60, 24);
  spr.print(humidity.relative_humidity, 1);
  spr.println("%");
  spr.pushSprite(0, 0);
}

void setClock0() // 获取时间的（时分秒）
{

  struct tm timeinfo;
  /*struct tm {
  int tm_sec; // 秒，取值0~59；
  int tm_min; // 分，取值0~59；
  int tm_hour; // 时，取值0~23；
  int tm_mday; // 月中的日期，取值1~31；
  int tm_mon; // 月，取值0~11；
  int tm_year; // 年，其值等于实际年份减去1900；
  int tm_wday; // 星期，取值0~6，0为周日，1为周一，依此类推；
  int tm_yday; // 年中的日期，取值0~365，0代表1月1日，1代表1月2日，依此类推；
  int tm_isdst; // 夏令时标识符，实行夏令时的时候，tm_isdst为正；不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负
  };*/
  if (!getLocalTime(&timeinfo))
  {
    // 如果获取失败，就开启网络对时
    Serial.println("Failed to obtain time");

    configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
    return;
  }

  Serial.println(&timeinfo, "%F %T %A"); // 格式化输出:2021-10-24 23:00:44 Sunday
  Serial.println(&timeinfo, "%F");
  Serial.println(&timeinfo, "%T");
  Serial.println(&timeinfo, "%A");
  Serial.print(asctime(&timeinfo)); // 默认打印格式：Mon Oct 25 11:13:29 2021

  // Draw a black rectangle in sprite so when we move it 1 pixel it does not leave a trail
  // on the black screen background
  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextFont(2);
  spr.setTextSize(1);
  spr.setCursor(0, 0); // 从左上角原点显示
  spr.setCursor(40, 0);
  spr.println(&timeinfo, "%F");
  spr.setTextSize(2);
  // spr.setTextFont(2);
  spr.setCursor(14, 16);
  spr.println(&timeinfo, "%A");
  // spr.setTextSize(2);
  spr.setCursor(24, 48);
  spr.println(&timeinfo, "%T"); // 显示时分秒：hh:mm:ss
  // spr.setTextSize(1);
  spr.pushSprite(0, 0); // 从左上角原点显示
}

void setClock1()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    // 如果获取失败，就开启网络对时
    Serial.println("Failed to obtain time");
    configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
    return;
  }

  Serial.println(&timeinfo, "%F %T %A"); // 格式化输出:2021-10-24 23:00:44 Sunday
  Serial.println(&timeinfo, "%F");
  Serial.println(&timeinfo, "%T");
  Serial.println(&timeinfo, "%A");
  Serial.print(asctime(&timeinfo)); // 默认打印格式：Mon Oct 25 11:13:29 2021

  // Draw a black rectangle in sprite so when we move it 1 pixel it does not leave a trail
  // on the black screen background
  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_BLACK);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_YELLOW, TFT_BLACK);
  spr.setTextFont(2);
  // spr.setTextSize(1);
  // spr.setCursor(0, 0);           //从左上角原点显示
  // spr.setCursor(40, 0);spr.println(&timeinfo, "%F");
  spr.setTextSize(2);
  // spr.setTextFont(2);
  // spr.setCursor(14, 16);spr.println(&timeinfo, "%A");
  // spr.setTextSize(2);
  spr.setCursor(24, 24);
  spr.println(&timeinfo, "%T"); // 显示时分秒：hh:mm:ss
  // spr.setTextSize(1);
  spr.pushSprite(0, 0); // 从左上角原点显示
}

void setClock2()
{
  // Draw a black rectangle in sprite so when we move it 1 pixel it does not leave a trail
  // on the black screen background
  spr.drawRect(0, 0, WIDTH, HEIGHT, TFT_WHITE);
  spr.fillSprite(TFT_WHITE);
  spr.setTextColor(TFT_BLACK, TFT_WHITE);
  spr.setTextFont(2);
  spr.setTextSize(1);
  // spr.setCursor(0, 0);           //从左上角原点显示
  // spr.setCursor(40, 0);spr.println(&timeinfo, "%F");
  spr.pushImage(0, 8, 64, 64, bmp8_gif[i]); // 调用图片数据
  i += 1;
  if (i > 4)
  {
    i = 0;
  }
  delay(100);
  // spr.pushImage(0,0,64,64,bmp2_lbili);
  spr.setCursor(80, 22);
  spr.println("Design by");
  spr.setCursor(90, 39);
  spr.println("Adrian");
  spr.pushSprite(0, 0); // 从左上角原点显示
}

void setup() // 定义引脚、连接wifi（其中有查看状态）
{
  Serial.begin(115200);

  pinMode(Key0, INPUT_PULLUP);
  pinMode(Key1, INPUT_PULLUP);
  pinMode(Key2, INPUT_PULLUP);
  pinMode(Key3, INPUT_PULLUP);
  pinMode(Key4, INPUT_PULLUP);
  pinMode(REDLED, OUTPUT);

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("Adafruit AHT10 demo!");

  if (!aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }
  Serial.println("AHT10 found");

  tft.init();
  tft.setRotation(3);
  tft.setTextFont(2);

  // Create a sprite of defined size
  spr.createSprite(WIDTH, HEIGHT);

  // Clear the TFT screen to black
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  // tft.setTextSize(1);
  tft.setCursor(25, 12);
  tft.println("Power by");
  tft.setCursor(25, 28);
  tft.setTextSize(2);
  tft.println("Arduino");
  delay(2000);

  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("WIFI Connecting");
  tft.fillScreen(TFT_BLACK); // LCD清空
  tft.setCursor(0, 0);
  tft.setTextSize(1);
  tft.print("WIFI Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(REDLED, 1);
    delay(500);
    Serial.print(".");
    tft.print(".");
    digitalWrite(REDLED, 0);
  }
  Serial.println();
  Serial.println("WiFi connected!");

  // 从网络时间服务器上获取并设置时间
  configTime(8 * 3600, 0, NTP1, NTP2, NTP3);
  delay(300);

  tft.println();
  tft.println("WiFi connected!");
  delay(1000);
  tft.fillScreen(TFT_BLACK); // LCD清空
  tft.setCursor(4, 20);
  tft.setTextSize(2);
  tft.println("Ready!!! GO");
  delay(1000);
  tft.fillScreen(TFT_BLACK);
}

void loop() // 要做的事
{
  KeyScan();         // 按键扫描
  switch (stateFlag) // 主状态机
  {
  case 0: // 显示时间

    switch (stateFlag2) // 副状态机
    {
    case 0: // 显示详细时间

      setClock0();

      break;

    case 1: // 显示时分秒

      setClock1();

      break;

    case 2: // 显示小张吃土了

      setClock2();

      break;

    default:
      stateFlag = 0;
      break;
    }

    break;

  case 1: // 显示温湿度

    switch (stateFlag2) // 副状态机
    {
    case 0: // 显示详细时间

      temp_hum();

      break;

    case 1: // 显示时分秒

      temp();

      break;

    case 2: // 显示小张吃土了

      Humidity();

      break;

    default:
      stateFlag = 0;
      break;
    }

    break;

  case 2: // 显示CPU&GPU负载率

    switch (stateFlag2) // 副状态机
    {
    case 0: // 显示详细

      PC_infor();

      break;

    case 1: // 显示GPU

      PC_infor1();

      break;

    case 2: // 显示CPU

      PC_infor2();

      break;

    default:
      stateFlag = 0;
      break;
    }

    break;

  default:
    stateFlag = 0;
    break;
  }
}
